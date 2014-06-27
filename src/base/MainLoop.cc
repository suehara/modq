// MainLoop.cc

#include "modq/MainLoop.h"
#include <iostream>
#include "modq/AcqSiLda.h"
#include "modq/DifPacket.h"
#include "modq/AcqUsb.h"

using namespace std;

namespace modq{

  void MainLoop::entryPoint(){

    /*
    AcqSiLda *lda = new AcqSiLda;
    lda->initialize(0, "eth1","5e:70:0c:d2:78:3d");
    
    lda->sendFastCommand(0,0x7c,0x62);
    
    cout << "LDA revision " << lda->readLdaRegister(0x400e) << endl;
    cout << "LDA version " << lda->readLdaRegister(0x400f) << endl;

    */

    AcqUsb usb;
    usb.initialize("/dev/ttyUSB0");

    // dif test
    DifPacket *dp = new DifPacket;
    dp->createPacket(0, 0, 0xcc04, 2);
    AcqPacket *reply = usb.sendDifPacket(dp);
    
    reply->printPacket();

  }
}
