// AcqBase.h

#ifndef ACQBASE_H
#define ACQBASE_H

#include <vector>
#include <map>
#include <pthread.h>
#include <string>
#include <istream>
#include <ostream>

namespace modq{

  namespace util{
    // a bit adhoc...
    inline void putUShort(std::ostream &st, unsigned short s){st.put(s/0x100); st.put(s%0x100);}
    inline void putUInt(std::ostream &st, unsigned int i){
      st.put(i/0x1000000); st.put(i/0x10000%0x100);
      st.put(i/0x100%0x100); st.put(i%0x100);
    }
    inline unsigned short getUShort(std::istream &st){return st.get() * 0x100 + st.get();}
    inline unsigned int getUInt(std::istream &st){return st.get() * 0x1000000 + st.get() * 0x10000 + st.get() * 0x100 + st.get();}
  }
  
  class AcqPacket{
    public:
      AcqPacket(){}
      virtual ~AcqPacket(){}

      virtual int getId()const = 0;
      virtual std::string processToArray()const = 0;
      virtual int processFromArray(const std::string &str) = 0;
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
    
    
    // Functions called from primary thread /////////////////////////////////////
    public:
      // send a packet to acq hardware, waiting a reply
      void sendMessage(const AcqPacket *msg, bool needReply, AcqPacket **msgReply, int timeout);

      // call to finalize thread
      void exitThread();
    protected:
      // called after configuration of fd
      void initializeThread(int fd);

    // Functions called from acquisition thread /////////////////////////////////
    protected:
      // main func of derived class: parse the packet
      virtual int read(const std::string &data) = 0; // return number of bytes remaining
      virtual void write(std::string &dataOut, const AcqPacket *msg) = 0;

      // lower-level IO command with the file descriptor: override if above not satisfactory
      virtual void read(int fd); 
      virtual void write(int fd, const AcqPacket *msg);
      
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
      
      std::string _bufRead; // buffer for read() to keep imcomplete packet data
      std::string _bufWrite; // buffer for write() to be filled from inherited classes
  };

  
}

#endif
