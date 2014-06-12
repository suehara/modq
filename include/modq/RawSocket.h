// RawSocket.h

#ifndef RAWSOCKET_H
#define RAWSOCKET_H

#include <string>
#include <memory.h>
#include <netpacket/packet.h>

namespace modq{

  // support classes
  class MacAddress{
    public:
      MacAddress(){}
      MacAddress(const char *addr, bool binary){binary ? setBinary(addr) : setString(addr);}
      ~MacAddress(){}
      
      void setBinary(const char *addr){memcpy(_addr, addr,6);binaryToString();}
      void setString(const char *addr){_addrStr = addr; stringToBinary();}

      const char *getBinary()const{return _addr;}
      const std::string &getString()const{return _addrStr;}

    private:
      void binaryToString();
      void stringToBinary();

    private:
      char _addr[6];
      std::string _addrStr;
  };
  
  class RawSocket{
    public:
      RawSocket() : _fd(-1){}
      ~RawSocket();

      void initSocket(const char *ifName, MacAddress &dstMac);
      int sendTo(const void *buf, unsigned int len);

      const MacAddress &getSrcMac()const{return _srcMac;}
      const MacAddress &getDstMac()const{return _dstMac;}
      
      int getFd(){return _fd;}
      
    private:
      int _fd;
      MacAddress _srcMac;
      MacAddress _dstMac;
      sockaddr_ll _sockAddr;
  };
 
}

#endif
