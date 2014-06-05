// AcqSiLda.h

#ifndef ACQSILDA_H
#define ACQSILDA_H

#include "modq/AcqBase.h"

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
      virtual void read(int fd); 
      virtual void write(const AcqMessage *msg);
      
    private:
      int _ldaId;
      std::string _macAddr;
  };

  
}

#endif
