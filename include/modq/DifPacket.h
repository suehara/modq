// DifPacket.h

#ifndef DIFPACKET_H
#define DIFPACKET_H

#include "modq/AcqBase.h"

namespace modq{

  class DifPacket : public AcqPacket{
    public:
      enum DifTypeSi{
        DataType = 0x0001,
        BlockTransferType = 0x0002,
        RandomGeneratorType = 0x0003,
      }; // other types are not well documented
      
      enum DifModifierSi{
        SpillModifier = 0x001c,
        ReadStatusModifier = 0x0012,
        ReadOutModifier = 0x000e,
        ScLoadModifier = 0x000c,
        ScEnableModifier = 0x000a,
        ModeModifier = 0x0006,
        ResetModifier = 0x0004,
        PowerModifier = 0x0002
      };
      
    public:
      DifPacket() : _littleEndian(true), _useTypeId(true){}
      ~DifPacket(){}

      void createPacket(unsigned short type, unsigned short id, unsigned short modifier, std::vector<unsigned short> &data);
      void createPacket(unsigned short type, unsigned short id, unsigned short modifier, unsigned short data); // for 1-word command
      void setId(int id){_id = id;}
      
      void setMode(bool littleEndian, bool useTypeId){_littleEndian = littleEndian; _useTypeId = useTypeId;}

      // output to cout
      virtual void printPacket();

    private:
      unsigned short _type;
      unsigned short _id;
      unsigned short _modifier;
      std::vector<unsigned short> _data;
      
      // modes
      bool _littleEndian; // Si: little endian, Sc: big endian
      bool _useTypeId;   // Si: with type&id, Sc: without type&id

    public:
      virtual int getId()const{return _id;}
      virtual std::string processToArray()const;
      virtual int processFromArray(const std::string &str);
  };
    
}

#endif
