// AcqSiLda.cc
#include "modq/AcqSiLda.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <iostream>
#include <errno.h>

using namespace std;

namespace modq{

  vector<char> AcqSiLdaPacket::processToArray()const
  {
    vector<char> ret;
    copy(_dstMac.getBinary(), _dstMac.getBinary()+6, back_inserter(ret));
    copy(_srcMac.getBinary(), _srcMac.getBinary()+6, back_inserter(ret));
    ret.push_back(_ethernetType / 0x100);
    ret.push_back(_ethernetType % 0x100);
    if(_ethernetType == 0x0809){
      ret.push_back(0xfa);
      ret.push_back(0x57);
      ret.push_back(_difId / 0x100);
      ret.push_back(_difId % 0x100);
      ret.push_back(_fcComma);
      ret.push_back(_fcData);
      unsigned short parity = fcCalcParity();
      ret.push_back(parity / 0x100);
      ret.push_back(parity % 0x100);
    }else if(_ethernetType == 0x0810){
      ret.push_back(_ldaTypeSubsystem);
      ret.push_back(_ldaTypeOperation);
      ret.push_back(_difId / 0x100);
      ret.push_back(_difId % 0x100);
      ret.push_back(_pktId / 0x100);
      ret.push_back(_pktId % 0x100);
      ret.push_back(_dataLength / 0x100);
      ret.push_back(_dataLength % 0x100);
      
      if(_dataLength > 0){
        if(_ldaTypeSubsystem == LdaRegisterSubsystem && (_ldaTypeOperation == ReadOperation || _ldaTypeOperation == WriteOperation)){
          for(int i=0;i<_dataLength;i++){
            ret.push_back(_registers[i].first / 0x100);
            ret.push_back(_registers[i].first % 0x100);
            ret.push_back(_registers[i].second / 0x1000000);
            ret.push_back((_registers[i].second % 0x1000000) / 0x10000);
            ret.push_back((_registers[i].second % 0x10000) / 0x100);
            ret.push_back(_registers[i].second % 0x100);
          }
        }else if(_ldaTypeSubsystem == DifTransportSubsystem){
          for(int i=0;i<_dataLength;i++){
            vector<char> difret = _difPackets[i]->processToArray();
            copy(difret.begin(), difret.end(), back_inserter(ret));
          }
        }else{
          cerr << "Error: AcqSiLdaPacket::processToArray(): packet format not supported!" << endl;
          ret.clear();
        }
      }
    }else{
      cerr << "Error: AcqSiLdaPacket::processToArray(): ethernetType " << _ethernetType << " not supported!" << endl;
      ret.clear();
    }
    return ret;
  }
  
  int AcqSiLdaPacket::processFromArray(const std::vector<char> &data)
  {
    _dstMac.setBinary(&data[0]);
    _srcMac.setBinary(&data[6]);
    
    int i = 12;
    _ethernetType = data[i++] * 0x100;
    _ethernetType += data[i++];
    if(_ethernetType == 0x0809){
      cerr << "AcqSiLdaPacket::processFromArray(): fast command packet arrived, which is not supported. just ignore the packet." << endl;
      return data.size()-22;
    }
    else if (_ethernetType != 0x0810 && _ethernetType != 0x0811){
      cerr << "Error: AcqSiLdaPacket::processFromArray(): ethernetType " << _ethernetType << " not supported!" << endl;
      return -1;
    }
    
    _ldaTypeSubsystem = data[i++];
    _ldaTypeOperation = data[i++];
    _difId = data[i++] * 0x100;
    _difId += data[i++];
    _pktId = data[i++] * 0x100;
    _pktId += data[i++];
    _dataLength = data[i++] * 0x100;
    _dataLength += data[i++];

    cerr << "AcqSiLdaPacket::processFromArray(): dataLength = " << _dataLength << endl;
    if(_dataLength > 0){
      if(_ldaTypeSubsystem == LdaRegisterSubsystem && _ldaTypeOperation == ReadReplyOperation){
        int dl = _dataLength;
        while(dl-- > 0 && data.size() - i >= 6){
          unsigned short address = data[i++] * 0x100;
          address += data[i++];
          unsigned int d = data[i++] * 0x1000000;
          d += data[i++] * 0x10000;
          d += data[i++] * 0x100;
          d += data[i++];
          _registers.push_back(LdaRegister(address,d));
        }
        if(dl == 0){
          // all register read out
          return data.size() - i;
        }else{
          // readout not completed
          return data.size();
        }
      }else{
        cerr << "Error: AcqSiLdaPacket::processFromArray(): packet format not supported!" << endl;
        return -1;
      }
    }
    // no data: readout completed
    return data.size() - i;
  }
  
  unsigned short AcqSiLdaPacket::fcCalcParity()const
  {
    return 0; // TODO
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
    delete packet;
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
  
  int AcqSiLda::read(std::vector<char> &data)
  {
    cerr << "AcqSiLda::read(): " << data.size() << " bytes arrived." << endl;
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
    else if (n>0){
      setReply(packet->_pktId, packet);
    }
    else{
      n = 0;
    }
    
    return n;
  }
  
  void AcqSiLda::write(int fd, const AcqPacket *msg)
  {
    vector<char> buf = msg->processToArray();
    
    cerr << "AcqSiLda::write(): sending ";
    for(unsigned int i=0;i<buf.size();i++){
      cerr << hex << (int)(unsigned char)buf[i] << " ";
    }
    cerr << endl;
    
    int ret = _soc.sendTo(&buf[0], buf.size());
    if(ret < 0){
      cerr << "Error: AcqSiLda::write(): write failed! errno = " << errno << endl;
    }      
  }
  
}
