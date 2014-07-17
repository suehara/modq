// AcqUsb.h

#ifndef ACQUSB_H
#define ACQUSB_H

#include "modq/AcqBase.h"

namespace modq{

  class DifPacket;
  
  class AcqUsb : public AcqBase {

    public:
      AcqUsb(){}
      ~AcqUsb(){}
      
      enum{DefaultTimeout = 1000};
      enum{PacketId = 0xbaba}; // use fixed id
    
    public:
      void initialize(const char *port);
      AcqPacket * sendDifPacket(DifPacket *difPacket);

    protected:
      // main func of derived class: parse the packet
      virtual int read(const std::string &data); // return number of bytes remaining
      virtual void write(std::string &dataOut, const AcqPacket *msg);
  };

  
}

#endif
