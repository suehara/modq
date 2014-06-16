// DifPacket.cc
#include "modq/DifPacket.h"

#include <algorithm>
#include <sstream>
#include <arpa/inet.h>

using namespace std;

namespace modq{

  string DifPacket::processToArray()const
  {
    ostringstream s;
    s << htons(_type);
    s << htons(_id);
    s << htons(_modifier);
    s << htons(_data.size());
    for(unsigned int i=0; i < _data.size();i++)
      s << htons(_data[i]);
    
    return s.str();
  }
  
  int DifPacket::processFromArray(const string &data)
  {
    istringstream s(data);
    unsigned short temp;
    s >> temp; _type = ntohs(temp);
    s >> temp; _id = ntohs(temp);
    s >> temp; _modifier = ntohs(temp);

    unsigned short size;
    s >> size;
    
    for(unsigned short i=0;i<size;i++){
      s >> temp; _data.push_back(temp);
    }
    
    return data.size() - s.tellg();
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

  void DifPacket::createPacket(unsigned short type, unsigned short id, unsigned short modifier, unsigned short data)
  {
    _type = type;
    _id = id;
    _modifier = modifier;
    _data.push_back(data);
  }

}
