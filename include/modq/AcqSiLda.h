// AcqSiLda.h

#ifndef ACQSILDA_H
#define ACQSILDA_H

#include "modq/AcqBase.h"
#include "modq/RawSocket.h"
#include <string>

namespace modq{

  class AcqSiLda;
  
  class AcqSiLdaPacket : public AcqPacket{
    friend class AcqSiLda;

    private:
      enum LdaEhternetType{
        FastCommandType = 0x809,
        NormalLdaType = 0x810,
        DifDataType = 0x811
      };
      
      enum LdaSubsystem{
        LdaRegisterSubsystem = 0x00,
        DifTransportSubsystem = 0x01,
        DiagnosticSubsystem = 0x02,
        LdaPacketGeneratorSubsystem = 0xff,
      };
      
      enum LdaOperation{
        LdaGenDataOperation = 0x00,
        WriteOperation = 0x01,
        ReadOperation = 0x02,
        WriteAckOperation = 0x03,
        ReadReplyOperation = 0x04,
        ReadNackOperation = 0x05,
        BadPacketOperation = 0xff,
      };
      
      // basic header components
      MacAddress _dstMac;
      MacAddress _srcMac;
      unsigned short _ethernetType;
      unsigned char _ldaTypeSubsystem;
      unsigned char _ldaTypeOperation;
      unsigned short _difId;
      unsigned short _pktId;
      unsigned short _dataLength;
      
      // fast command components
      unsigned char _fcComma;
      unsigned char _fcData;
      
      // register access
      typedef std::pair<unsigned short, unsigned int> LdaRegister;
      std::vector<LdaRegister> _registers;

      // DIF access
      std::vector<const AcqPacket *> _difPackets;

      AcqSiLdaPacket(){}
      ~AcqSiLdaPacket(){}

      // output to cout
      void printPacket();

      // parity calculation
      unsigned short fcCalcParity()const;

    protected:
      virtual int getId()const{return _pktId;}
      virtual std::vector<char> processToArray()const;
      virtual int processFromArray(const std::vector<char> &str);
  };
  
  /*
  struct AcqSiLdaConfig {
    int ldaId;
    std::string macAddr;
  };
  */

  class AcqSiLda : public AcqBase {

    public:
      AcqSiLda() : _curId(10){}
      ~AcqSiLda(){}
      
      enum{DefaultTimeout = 1000};
    
    public:
      void initialize(int ldaId, const char *ifName, const char *macAddr);
      
      // r/w LDA registers
      unsigned int readLdaRegister(unsigned short address);
      void writeLdaRegister(unsigned short address, unsigned int data);

      // fast command
      void sendFastCommand(int difId, int comma, int cmdId); // command ID is defined in DIF class

    private:
      // create packets
      AcqPacket * createLdaRegisterPacket(bool read, unsigned short address, unsigned int data);
      AcqPacket * createFastCommandPacket(int difId, int comma, int cmdId);
      AcqPacket * createDifCommandPacket(int difId, const AcqPacket *difPacket);
      
    protected:
      // main func of derived class: parse the packet
      virtual int read(std::vector<char> &data); // return number of bytes remaining

      virtual void write(std::vector<char> &dataOut, const AcqPacket *msg){} // no-op, need sendto() instead of write()
      virtual void write(int fd, const AcqPacket *msg);

    private:
      unsigned short getNextId(){return _curId++;}
    private:
      int _ldaId;
      RawSocket _soc;
      unsigned short _curId;
      
  };

  
}

#endif
