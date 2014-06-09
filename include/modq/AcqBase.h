// AcqBase.h

#ifndef ACQBASE_H
#define ACQBASE_H

#include <vector>
#include <map>
#include <pthread.h>

namespace modq{

  class AcqPacket{
    public:
      AcqPacket(){}
      virtual ~AcqPacket(){}

      virtual int getId()const = 0;
      virtual std::vector<char> processToArray()const = 0;
      virtual void processFromArray(const std::vector<char> &str) = 0;
  };

  class AcqBase {
    private:
      struct AcqMessageBox{
        int id;
        const AcqPacket *send;
        AcqPacket *reply;
        pthread_cond_t cond;
        bool needReply;
      };

    protected:
      AcqBase(){}
    public:
      virtual ~AcqBase(){}
    
    public:
      // send a packet to acq hardware, waiting a reply
      void sendMessage(const AcqPacket *msg, bool needReply, AcqPacket **msgReply, int timeout);

      // call to finalize thread
      void exitThread();
    
    protected:
      // main func of derived class: parse the packet
      virtual int read(std::vector<char> &data) = 0; // return number of bytes remaining
      virtual void write(std::vector<char> &dataOut, const AcqPacket *msg) = 0;

      // lower-level IO command with the file descriptor: override if above not satisfactory
      virtual void read(int fd); 
      virtual void write(int fd, const AcqPacket *msg);
      
      // called after configuration of fd
      void initailizeThread(int fd);

      // called from read(): to send back reply to sendMessage()
      void setReply(int id, AcqPacket *msgReply);
    
    private:
      static void *entryPoint(void *th);
      virtual void entryPoint(); // can be overwritten: assume USB, no need for ethernet
    
    private:
      std::map<int, AcqMessageBox *> _messageMap;
      pthread_mutex_t _messageMapMutex;

      int _fdAcq;
      int _fdControlIn;
      int _fdControlOut;
      
      pthread_t _threadId;
      
      std::vector<char> _bufRead; // buffer for read() to keep imcomplete packet data
      std::vector<char> _bufWrite; // buffer for write() to be filled from inherited classes
  };

  
}

#endif
