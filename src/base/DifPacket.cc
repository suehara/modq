// DifPacket.cc
#include "modq/DifPacket.h"

#include <algorithm>
#include <sstream>
#include <arpa/inet.h>
#include <iostream>

using namespace std;

namespace modq{

  string DifPacket::processToArray()const
  {
    ostringstream s;

    if(_useTypeId){
      util::putUShort(s,_type,_littleEndian);
      util::putUShort(s,_id,_littleEndian);
    }
    util::putUShort(s,_modifier,_littleEndian);
    util::putUShort(s,_data.size(),_littleEndian);
    for(unsigned int i=0; i < _data.size();i++)
      util::putUShort(s,_data[i],_littleEndian);

    return s.str();
  }
  
  int DifPacket::processFromArray(const string &data)
  {
    istringstream s(data);
    if(_useTypeId){
      _type = util::getUShort(s,_littleEndian);
      _id = util::getUShort(s,_littleEndian);
    }
    _modifier = util::getUShort(s);

    unsigned short size = util::getUShort(s,_littleEndian);
    
    for(unsigned short i=0;i<size;i++){
      _data.push_back(util::getUShort(s,_littleEndian));
    }
    
    cerr << "Dif packet extracted: type = " << _type << ", id = " << _id << ", modifier = " << _modifier << ", data = ";
    for(unsigned int i=0;i<size;i++){
      cerr << _data[i] << ", ";
    }
    cerr << endl;
    
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
