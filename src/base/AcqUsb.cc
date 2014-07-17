// AcqUsb.cc
#include "modq/AcqUsb.h"
#include "modq/DifPacket.h"

#include <iostream>
#include <errno.h>

#include <fcntl.h>
#include <termios.h>
#include <memory.h>

#include <sstream>

using namespace std;


namespace modq{

  void AcqUsb::initialize(const char *port)
  {
    int fd = open(port,O_RDWR,O_SYNC);

    if(fd == -1){
      cerr << "AcqUsb::initialize: Failed to open tty port" << endl;
      return;
    }

    termios oldios;
    tcgetattr(fd,&oldios);

    termios newios;
    memset(&newios,0,sizeof(termios));

    newios.c_iflag = 0;
    newios.c_oflag = 0;
    newios.c_cflag = B115200 | CS8 | CLOCAL | CREAD;
    newios.c_lflag = 0;//ICANON; // non-canonical    
    newios.c_cc[VTIME] = 0;   // timer not used
    newios.c_cc[VMIN]  = 1;   // block until 1 char received

    tcsetattr(fd,TCSAFLUSH,&newios);

    initializeThread(fd);
  }

  AcqPacket * AcqUsb::sendDifPacket(DifPacket * difPacket)
  {
    AcqPacket *reply = NULL;
    difPacket->setId(PacketId);
    difPacket->setMode(false, false);
    sendMessage(difPacket, true, &reply, DefaultTimeout);
    
    return reply;
  }

  int AcqUsb::read(const std::string &data)
  {
    cerr << "AcqUsb::read(): " << dec << data.size() << " bytes arrived." << endl;
    for(unsigned int i=0;i<data.size();i++){
      cerr << hex << (int)(unsigned char)data.c_str()[i] << " ";
    }
    cerr << endl;

    DifPacket *packet = new DifPacket;
    packet->setMode(false, false); // little endian, use type id
    packet->setId(PacketId);
    
    int n = packet->processFromArray(data);
    if(n == (int)data.size()){
      cerr << "AcqUsb::read(): packet creation has not finished. Waiting more data." << endl;
      delete packet;
    }
    else if (n>=0){
      cerr << "AcqUsb::read(): packet obtained." << endl;
      setReply(PacketId, packet);
    }
    else{
      n = 0;
    }
    
    return n;
  }

  void AcqUsb::write(std::string &dataOut, const AcqPacket *msg)
 {
    dataOut = msg->processToArray();
    
    cerr << "AcqSiLda::write(): sending ";
    for(unsigned int i=0;i<dataOut.size();i++){
      cerr << hex << (int)(unsigned char)dataOut.c_str()[i] << " ";
    }
    cerr << endl;
  }

}
