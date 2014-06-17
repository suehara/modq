// AcqSiLda.cc
#include "modq/AcqSiLda.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <iostream>
#include <errno.h>
#include <arpa/inet.h>

#include <sstream>

using namespace std;

namespace modq{

  string AcqSiLdaPacket::processToArray()const
  {
    ostringstream s;
    
    s.write(_dstMac.getBinary(), 6);
    s.write(_srcMac.getBinary(), 6);
    util::putUShort(s, _ethernetType);

    if(_ethernetType == 0x0809){
      util::putUShort(s, 0xfa57);
      util::putUShort(s, _difId);
      s.put(_fcComma);
      s.put(_fcData);
      unsigned short parity = fcCalcParity();
      util::putUShort(s, parity);

    }else if(_ethernetType == 0x0810){
      s.put(_ldaTypeSubsystem);
      s.put(_ldaTypeOperation);
      util::putUShort(s, _difId);
      util::putUShort(s, _pktId);
      util::putUShort(s, _dataLength);
      
      if(_dataLength > 0){
        if(_ldaTypeSubsystem == LdaRegisterSubsystem && (_ldaTypeOperation == ReadOperation || _ldaTypeOperation == WriteOperation)){
          for(int i=0;i<_dataLength;i++){
            util::putUShort(s, _registers[i].first);
            util::putUInt(s, _registers[i].second);
          }
        }else if(_ldaTypeSubsystem == DifTransportSubsystem){
          for(int i=0;i<_dataLength;i++){
            string ss = _difPackets[i]->processToArray();
            s << ss;
          }
        }else{
          cerr << "Error: AcqSiLdaPacket::processToArray(): packet format not supported!" << endl;
          return string();
        }
      }
    }else{
      cerr << "Error: AcqSiLdaPacket::processToArray(): ethernetType " << _ethernetType << " not supported!" << endl;
      return string();
    }
    return s.str();
  }
  
  int AcqSiLdaPacket::processFromArray(const std::string &data)
  {
    _dstMac.setBinary(data.substr(0,6).c_str());
    _srcMac.setBinary(data.substr(6,6).c_str());
    
    istringstream s(data);
    s.ignore(12);
    _ethernetType = util::getUShort(s);

    if(_ethernetType == 0x0809){
      cerr << "AcqSiLdaPacket::processFromArray(): fast command packet arrived, which is not supported. just ignore the packet." << endl;
      return -1;
    }
    else if (_ethernetType != 0x0810 && _ethernetType != 0x0811){
      cerr << "Error: AcqSiLdaPacket::processFromArray(): ethernetType " << _ethernetType << " not supported! packet discarded." << endl;
      return -1;
    }

    _ldaTypeSubsystem = s.get();
    _ldaTypeOperation = s.get();

    _difId = util::getUShort(s);
    _pktId = util::getUShort(s);
    _dataLength = util::getUShort(s);
    
    cerr << "AcqSiLdaPacket::processFromArray(): dataLength = " << _dataLength << endl;
    if(_dataLength > 0){
      if(_ldaTypeSubsystem == LdaRegisterSubsystem && _ldaTypeOperation == ReadReplyOperation){
        int dl = _dataLength;
        while(dl-- > 0 && data.size() - s.tellg() >= 6){

          unsigned short address_n = util::getUShort(s);
          unsigned int d_n = util::getUInt(s);
          _registers.push_back(LdaRegister(address_n,d_n));
        }
        if(dl == -1){
          if(s.tellg() < 60) s.seekg(60);
          // all register read out
          cerr << "AcqSiLdaPacket::processFromArray(): packet obtained. " << data.size() - s.tellg() << " bytes remaining." << endl;
          return data.size() - s.tellg();
        }else{
          // readout not completed
          cerr << "AcqSiLdaPacket::processFromArray(): packet is imcomplete! packet discarded." << endl;
          return -1; // all data discarded
        }
      }else{
        cerr << "Error: AcqSiLdaPacket::processFromArray(): packet format not supported! packet discarded." << endl;
        return -1;
      }
    }
    // no data: readout completed
    return data.size() - s.tellg();
  }
  
  bool AcqSiLdaPacket::evenParity(unsigned char c)const
  {
    unsigned char ret = 0;
    for(int n=0;n<8;n++){
      ret ^= (c>>n)&1;
    }
    return ret&1;
  }

  unsigned short AcqSiLdaPacket::fcCalcParity()const
  {
    unsigned short result = 0;
    
    result  = evenParity(0x57);
    result |= evenParity(0xfa) << 1;
    result |= evenParity(_difId & 0xff) << 2;
    result |= evenParity((_difId>>8)& 0xff) << 3;
    result |= evenParity(_fcComma) << 4;
    result |= evenParity(_fcData) << 5;

    return result;
  }
  
  void AcqSiLdaPacket::printPacket()
  {
    // TODO
  }
  
  void AcqSiLda::initialize(int ldaId, const char *ifName, const char *macAddr)
  {
    _ldaId = ldaId;

    // initialize socket
    MacAddress mac(macAddr, false);
    _soc.initSocket(ifName, mac);

    // start thread
    int fd = _soc.getFd();
    initializeThread(fd);
  }
      
  unsigned int AcqSiLda::readLdaRegister(unsigned short address)
  {
    AcqPacket *packet = createLdaRegisterPacket(true, address, 0);
    AcqPacket *reply;
    sendMessage(packet, true, &reply, DefaultTimeout);
    
    AcqSiLdaPacket *rep = static_cast<AcqSiLdaPacket *>(reply);
    if(!rep || rep->_ldaTypeOperation != AcqSiLdaPacket::ReadReplyOperation || rep->_dataLength != 1){
      cerr << "Error: AcqSiLda::readLdaRegister: reply invalid!" << endl;
    }
    if(rep->_registers.size() != 1){
      cerr << "Error: AcqSiLda::readLdaRegister: reply register size invalid! 1 required: " << rep->_registers.size() << " obtained." << endl;
    }
    
    return rep->_registers[0].second;
  }
  
  void AcqSiLda::writeLdaRegister(unsigned short address, unsigned int data)
  {
    AcqPacket *packet = createLdaRegisterPacket(false, address, data);
    AcqPacket *reply;
    sendMessage(packet, true, &reply, DefaultTimeout);
    
    if(!static_cast<AcqSiLdaPacket *>(reply) || static_cast<AcqSiLdaPacket *>(reply)->_ldaTypeOperation != AcqSiLdaPacket::WriteAckOperation){
      cerr << "Error: AcqSiLda::writeLdaRegister: reply invalid!" << endl;
    }
  }

  void AcqSiLda::sendFastCommand(int difId, int comma, int cmdId)
  {
    AcqPacket *packet = createFastCommandPacket(difId, comma, cmdId);
    sendMessage(packet, false, 0, DefaultTimeout);
    //delete packet;
  }

  AcqPacket * AcqSiLda::createLdaRegisterPacket(bool readLda, unsigned short address, unsigned int data)
  {
    AcqSiLdaPacket *packet = new AcqSiLdaPacket;
    packet->_dstMac = _soc.getDstMac();
    packet->_srcMac = _soc.getSrcMac();
    packet->_ethernetType = AcqSiLdaPacket::NormalLdaType;
    packet->_ldaTypeSubsystem = AcqSiLdaPacket::LdaRegisterSubsystem;
    packet->_ldaTypeOperation = (readLda ? AcqSiLdaPacket::ReadOperation : AcqSiLdaPacket::WriteOperation);
    packet->_difId = 0; // not used
    packet->_pktId = getNextId();
    packet->_dataLength = 1; // only one packet
    packet->_registers.push_back(AcqSiLdaPacket::LdaRegister(address, data));
    
    return packet;
  }

  AcqPacket * AcqSiLda::createFastCommandPacket(int difId, int comma, int cmdId)
  {
    AcqSiLdaPacket *packet = new AcqSiLdaPacket;
    packet->_dstMac = _soc.getDstMac();
    packet->_srcMac = _soc.getSrcMac();
    packet->_ethernetType = AcqSiLdaPacket::FastCommandType;
    packet->_difId = difId;
    packet->_fcComma = comma;
    packet->_fcData = cmdId;

    return packet;
  }
  
  AcqPacket * AcqSiLda::createDifCommandPacket(int difId, const AcqPacket *difPacket)
  {
    AcqSiLdaPacket *packet = new AcqSiLdaPacket;
    packet->_dstMac = _soc.getDstMac();
    packet->_srcMac = _soc.getSrcMac();
    packet->_ethernetType = AcqSiLdaPacket::NormalLdaType;
    packet->_ldaTypeSubsystem = AcqSiLdaPacket::DifTransportSubsystem;
    packet->_ldaTypeOperation = 0;
    packet->_difId = difId;
    packet->_pktId = getNextId();
    packet->_dataLength = 1; // only one packet
    packet->_difPackets.push_back(difPacket);
    
    return packet;
  }
  
  int AcqSiLda::read(const std::string &data)
  {
    cerr << "AcqSiLda::read(): " << dec << data.size() << " bytes arrived." << endl;
    if(data.size() < 22){
      cerr << "AcqSiLda::read(): data size too short, pending. datasize = " << data.size() << endl;
      return data.size();
    }
    
    AcqSiLdaPacket *packet = new AcqSiLdaPacket;
    int n = packet->processFromArray(data);
    if(n == (int)data.size()){
      cerr << "AcqSiLda::read(): packet creation has not finished. Waiting more data." << endl;
      delete packet;
    }
    else if (n>=0){
      cerr << "AcqSiLda::read(): packet obtained." << endl;
      setReply(packet->_pktId, packet);
    }
    else{
      n = 0;
    }
    
    return n;
  }
  
  void AcqSiLda::write(int fd, const AcqPacket *msg)
  {
    string buf = msg->processToArray();
    
    cerr << "AcqSiLda::write(): sending ";
    for(unsigned int i=0;i<buf.size();i++){
      cerr << hex << (int)(unsigned char)buf.c_str()[i] << " ";
    }
    cerr << endl;
    
    int ret = _soc.sendTo(&buf[0], buf.size());
    if(ret < 0){
      cerr << "Error: AcqSiLda::write(): write failed! errno = " << errno << endl;
    }      
  }
  
}
