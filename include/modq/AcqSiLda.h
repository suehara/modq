// AcqSiLda.h

#ifndef ACQSILDA_H
#define ACQSILDA_H

#include "modq/AcqBase.h"
#include <string>

namespace modq{

  /*
  struct AcqSiLdaConfig {
    int ldaId;
    std::string macAddr;
  };
  */
  
  class AcqSiLda {

    public:
      AcqSiLda(){}
      ~AcqSiLda(){}
    
    public:
      void initialize(int ldaId, const char *macAddr);
      
    protected:
      // main func of derived class: parse the packet
      virtual int read(std::vector<char> &data); // return number of bytes remaining
      virtual void write(std::vector<char> &dataOut, const AcqPacket *msg);
      
    private:
      int _ldaId;
      std::string _macAddr;
  };

  
}

#endif
