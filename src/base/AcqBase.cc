// AcqBase.cc
#include "modq/AcqBase.h"

using namespace std;

namespace modq{
  void *AcqBase::entryPoint(void *th){
    ((AcqBase *)th)->entryPoint();
    return 0;
  }

  void AcqBase::entryPoint(){
    
  }

  void AcqBase::sendMessage(const AcqPacket *msg, bool needReply, AcqPacket *msgReply, int timeout)
  {
  }
  
  void AcqBase::exitThread()
  {
  }
  
  void AcqBase::read(int fd)
  {
  }
  
  void AcqBase::write(int fd, const AcqPacket *msg)
  {
  }
  
  void AcqBase::initailizeThread(int fd)
  {
  }
  
  void AcqBase::setReply(int id, AcqPacket *msgReply)
  {
  }

}
