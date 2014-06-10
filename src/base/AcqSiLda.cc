// AcqSiLda.cc
#include "modq/AcqSiLda.h"

using namespace std;

namespace modq{

  vector<char> AcqSiLdaPacket::processToArray()
  {
    vector<char> ret;
    return ret;
  }
  
  void AcqSiLdaPacket::processFromArray(const std::vector<char> &str)
  {
  }
  
  void AcqSiLda::initialize(int ldaId, const char *macAddr)
  {
  }
      
  unsigned int AcqSiLda::readLdaRegister(unsigned short address)
  {
    return 0;
  }
  
  void AcqSiLda::writeLdaRegister(unsigned short address, unsigned int data)
  {
  }

  void AcqSiLda::sendFastCommand(int difId, int cmdId)
  {
  }

  AcqPacket * AcqSiLda::createLdaRegisterPacket(bool read, unsigned short address, unsigned int data)
  {
    return NULL;
  }

  AcqPacket * AcqSiLda::createFastCommandPacket(int difId, int cmdId)
  {
    return NULL;
  }
  
  AcqPacket * AcqSiLda::createDifCommandPacket(int difId, const AcqPacket *difPacket)
  {
    return NULL;
  }
  
  int AcqSiLda::read(std::vector<char> &data)
  {
    return 0;
  }
  
  void AcqSiLda::write(std::vector<char> &data, const AcqPacket *msg)
  {
  }
  
}
