// AcqBase.h

#ifndef ACQBASE_H
#define ACQBASE_H

#include <vector>
#include <map>
#include <pthread.h>

namespace modq{

  class AcqMessage{
    public:
      AcqMessage(){}
      virtual ~AcqMessage(){}

      virtual int getId()const = 0;
      virtual std::vector<char> processToArray()const = 0;
      virtual void processFromArray(const std::vector<char> &str) = 0;
  };

  class AcqBase {

    protected: // singleton: constructor not public
      AcqBase(){}
    public:
      ~AcqBase(){}
    
    public:
      void sendMessage(const AcqMessage *msg, bool needReply, AcqMessage *msgReply, int timeout);
    
    protected:
      void initailizeThread();
    
    private:
      static void *entryPoint(void *this);
      void entryPoint();
    
    private:
      std::map<int, std::pair<pthread_cond_t, AcqMessage *> > _messageMap;
      pthread_mutex_t _messageMapMutex;
  };

  
}

#endif
