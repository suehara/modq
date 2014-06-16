// AcqBase.cc
#include "modq/AcqBase.h"

// system calls
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

// C headers
#include <errno.h>

// STL
#include <iostream>
#include <algorithm>

using namespace std;

namespace modq{
  void *AcqBase::entryPoint(void *th){
    ((AcqBase *)th)->entryPoint();
    return 0;
  }

  void AcqBase::entryPoint(){
    fd_set rfds;
   
    // main loop
    while(1){
      FD_ZERO(&rfds);
      FD_SET(_fdAcq,&rfds);
      FD_SET(_fdControlIn, &rfds);
      int nfds = max(_fdControlIn, _fdAcq)+1;

      int ret = pselect(nfds, &rfds, NULL, NULL, NULL, NULL);
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
          for(it = _messageMap.begin(); it != _messageMap.end();){
            const AcqPacket *&packet = it->second->send;
            if(packet){// not cleared - ready to send
              write(_fdAcq, packet);
              delete packet;
              packet = NULL; // clear
              if(it->second->needReply == false){ // no reply - packet finished
                delete it->second;
                _messageMap.erase(it++);
                continue; // skip it++
              }
            }
            it++;
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

  void AcqBase::sendMessage(const AcqPacket *msg, bool needReply, AcqPacket **msgReply, int timeout)
  {
    // initialize message box
    AcqMessageBox *box = new AcqMessageBox;
    box->send = msg;
    box->reply = NULL;
    box->needReply = needReply;
    if(needReply)
      pthread_cond_init(&box->cond,NULL);
    
    // lock mutex
    pthread_mutex_lock(&_messageMapMutex);
    
    int id = msg->getId();
    _messageMap[id] = box;
    char c = 'M';
    ::write(_fdControlOut,&c,1);
    
    if(needReply){
      // reply needed
      timespec ts;
      timeval now;
      ::gettimeofday(&now,NULL);

      ts.tv_sec = now.tv_sec + timeout / 1000;
      ts.tv_nsec = now.tv_usec * 1000 + (timeout % 1000) * 1e+6;

      // wait for reply to come
      int ret = pthread_cond_timedwait(&box->cond, &_messageMapMutex, &ts); 

      if(ret == ETIMEDOUT){
        cerr << "Error: AcqBase::sendMessage: reply timedout!" << endl;
      }
      else if (ret != 0){
        cerr << "Error: AcqBase::sendMessage: unknown error! " << ret << endl;
      }


      // reply obtained - or NULL
      (*msgReply) = box->reply;

      // delete reply
      pthread_cond_destroy(&box->cond);
      delete box;
      _messageMap.erase(id);

    }
    
    pthread_mutex_unlock(&_messageMapMutex);
  }
  
  void AcqBase::setReply(int id, AcqPacket *msgReply)
  {
    // lock mutex
    pthread_mutex_lock(&_messageMapMutex);

    // looking for the reply ID
    if(_messageMap.find(id) != _messageMap.end()){
      AcqMessageBox *box = _messageMap.find(id)->second;
      box->reply = msgReply;
      
      // activate signal - I hope this will resume pthread_cond_timedwait() after releasing mutex
      pthread_cond_signal(&box->cond);
    }else{
      cerr << "Error: unknown reply obtained!" << endl;
    }
    pthread_mutex_unlock(&_messageMapMutex);
  }

  void AcqBase::initializeThread(int fd)
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
    // this implementation assumes that all packets are atomic - not interrupted by another packet before finishing to receive
    // if it's not the case please override me.
    
    // change fd to non-blocking
/*    int mode = ::fcntl(_fdAcq, F_GETFL);
    ::fcntl(fd, F_SETFL, mode | O_NONBLOCK);
  */

    const int bufsize = 4096;
    // copy to C array, then to vector
    char buf[bufsize];
    int size = 0;
    do{
      // system call: from socket to C array
      size = ::read(fd, buf, bufsize);
      // C array to vector
      //copy(buf, buf + size, back_inserter(_bufRead));
      _bufRead.append(buf, bufsize);
    }while(size == bufsize);
    
    // call the main part (virtual function)
    unsigned int remainsize = read(_bufRead);
    
    // remove used part of the buffer
    if(remainsize == 0) // all used
      _bufRead.clear();
    else if(remainsize < _bufRead.size()){
      _bufRead.substr(_bufRead.size() - remainsize);
    }
  }
  
  void AcqBase::write(int fd, const AcqPacket *msg)
  {
    _bufWrite.clear();
    // call the main part (virtual function)
    write(_bufWrite, msg);
    
    if(_bufWrite.size() == 0){
      cerr << "Error: AcqBase::write(): no data received!" << endl;
      return;
    }
    
    unsigned int writesize = ::write(fd, _bufWrite.c_str(), _bufWrite.size());
    
    if(writesize != _bufWrite.size()){
      cerr << "Error: AcqBase::write(): write system call failed! nWrite = " << writesize << ", nToWrite = " << _bufWrite.size() << ", err = " << errno << endl;
      return;
    }
  }  

}
