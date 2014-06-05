// AcqBase.cc
#include "modq/AcqBase.h"

using namespace std;

namespace modq{
  void *AcqBase::entryPoint(void *this){
    ((AcaBase *)this)->EntryPoint();
  }

  void AcqBase::entryPoint(){
    
  }
}
