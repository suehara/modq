// AcqBase.cc
#include "modq/AcqBase.h"

// system calls
#include <sys/select.h>
#include <unistd.h>

// STL
#include <iostream>

using namespace std;

namespace modq{
  void *AcqBase::entryPoint(void *th){
    ((AcqBase *)th)->entryPoint();
    return 0;
  }

  void AcqBase::entryPoint(){
    fd_set rfds;
    
    FD_ZERO(&rfds);
    FD_SET(_fdAcq,&rfds);
    FD_SET(_fdControlIn, &rfds);
    
    // main loop
    while(1){
      int ret = pselect(2, &rfds, NULL, NULL, NULL, NULL);
      if(ret < 0){// error
        cerr << "Fatal error: AcqBase::entryPoint: pselect failed!" << endl;
        return;
      }
      if(FD_ISSET(_fdControlIn, &rfds)){
        // control socket is active
        
        // read first byte
        char c;
        if(::read(_fdControlIn, &c, 1) <= 0){
          cerr << "Fatal error: AcqBase::entryPoint: control socket closed!" << endl;
          return;
        }
        if(c == 'Q'){// quit
          cerr << "AcqBase::entryPoint: Quit command received: exiting the thread." << endl;
          break;
        }
        else if(c == 'M'){// message
          // looking for packets stored
          pthread_mutex_lock(&_messageMapMutex);
          
          std::map<int, AcqMessageBox *>::iterator it;
          for(it = _messageMap.begin(); it != _messageMap.end(); it++){
            AcqPacket *&packet = it->second->send;
            if(packet){// not cleared - ready to send
              write(_fdAcq, packet);
              delete packet;
              packet = NULL; // clear
            }
          }
          
          pthread_mutex_unlock(&_messageMapMutex);
        }else{
          cerr << "Fatal error: AcqBase::entryPoint: unknown control command received!" << endl;
          return;
        }
      }
      if(FD_ISSET(_fdAcq,&rfds)){
        // data socket is active
        read(_fdAcq);
      }
    }
    
  }

  void AcqBase::sendMessage(const AcqPacket *msg, bool needReply, AcqPacket *msgReply, int timeout)
  {
  }
  
  void AcqBase::setReply(int id, AcqPacket *msgReply)
  {
  }

  void AcqBase::initailizeThread(int fd)
  {
    // create control filedesc
    int pipefd[2];
    pipe(pipefd);
    _fdControlIn = pipefd[0];
    _fdControlOut = pipefd[1];
    
    _fdAcq = fd;
    
    pthread_create(&_threadId, NULL, AcqBase::entryPoint, (void *)this);
  }
  
  void AcqBase::exitThread()
  {
    // sending 'Q' message to the acq thread
    char c = 'Q';
    ::write(_fdControlOut, &c, 1);
    
    // waiting thread to end
    void *ret;
    pthread_join(_threadId, &ret);
    
    _threadId = 0;
  }
  
  void AcqBase::read(int fd)
  {
  }
  
  void AcqBase::write(int fd, const AcqPacket *msg)
  {
  }

}
