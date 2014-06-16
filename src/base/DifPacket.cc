// DifPacket.cc
#include "modq/DifPacket.h"

#include <algorithm>

using namespace std;

namespace modq{

  vector<char> DifPacket::processToArray()const
  {
    vector<char> ret;
    return ret;
  }
  
  int DifPacket::processFromArray(const std::vector<char> &data)
  {
    return data.size();
  }
  

  void DifPacket::printPacket()
  {
    // TODO
  }
  
  void DifPacket::createPacket(unsigned short type, unsigned short id, unsigned short modifier, std::vector<unsigned short> &data)
  {
    _type = type;
    _id = id;
    _modifier = modifier;
    copy(data.begin(), data.end(), back_inserter(_data));
  }

}
