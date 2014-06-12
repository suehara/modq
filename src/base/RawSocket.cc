// RawSocket.cc
#include "modq/RawSocket.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <arpa/inet.h>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <errno.h>

using namespace std;

namespace modq{

  void MacAddress::binaryToString(){
    ostringstream ss;
    for(int i=0;i<6;i++)
      ss << hex << setfill('0') << setw(2) << (unsigned short)_addr[i] << ":";
    
    _addrStr = ss.str();
  }
  
  void MacAddress::stringToBinary(){
    istringstream ss(_addrStr);
    unsigned short s;
    char c;
    for(int i=0;i<6;i++){
      ss >> hex >> s;
      _addr[i] = (char)s;
      ss.get(c);
      if(c != ':' && c != '-'){
        cerr << "Error: MacAddress::stringToBinary(): separator not detected: format should be wrong." << endl;
      }
    }
  }
  
  void RawSocket::initSocket(const char *ifName, MacAddress &dstMac)
  {
    _dstMac = dstMac;
    
    // obtain mac addr of current interface
    char srcMac[6];
    ifreq ifr;
    int fd = ::socket(AF_INET, SOCK_DGRAM,0);
    if(fd == -1){
      cerr << "Error: RawSocket::initialize(): test socket cannot be open." << endl;
      return;
    }
    strcpy(ifr.ifr_name, ifName);
    
    if(ioctl(fd, SIOCGIFHWADDR, &ifr) !=0){
      cerr << "Error: RawSocket::initialize(): ifName invalid." << endl;
      return;
    }
    memcpy(srcMac,ifr.ifr_hwaddr.sa_data,6);
    _srcMac.setBinary(srcMac);

    // obtain interface index
    if(ioctl(fd, SIOCGIFINDEX, &ifr) != 0){
      cerr << "Error: RawSocket::initialize(): SDOCGIFINDEX failed." << endl;
      return;
    }
    int ifindex = ifr.ifr_ifindex;
    
    // open socket: root required
    _fd = ::socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if(_fd == -1){
      cerr << "Error: RawSocket::initialize(): socket cannot be open. You need to be root." << endl;
      return;
    }

    // bind socket to the interface
    _sockAddr.sll_family = AF_PACKET;
    _sockAddr.sll_protocol = htons(ETH_P_ALL);
    _sockAddr.sll_ifindex = ifindex;

    if(::bind(_fd, (struct sockaddr *)&_sockAddr,sizeof(_sockAddr))!=0){
      cerr << "Error: RawSocket::initialize(): socket binding failed. errno = " << errno << endl;
      return;
    }
    
    // drop all packets
    char junk[256];
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(_fd,&rfds);
    timespec t;
    memset(&t, 0, sizeof(t)); // timeout = 0; return immediately
    while(pselect(_fd+1, &rfds, NULL, NULL, &t, NULL) >0)
      ::read(_fd, junk, sizeof(junk));
    
    // initialization finished
  }
  
  int RawSocket::sendTo(const void *buf, unsigned int len)
  {
    return ::sendto(_fd, buf, len, 0, (sockaddr *)&_sockAddr,sizeof(_sockAddr));
  }

  RawSocket::~RawSocket(){
    if(_fd>=0)::close(_fd);
  }
}

