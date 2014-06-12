// main.cc
#include "modq/MainLoop.h"
#include "modq/Log.h"
#include <iostream>

using namespace std;
using namespace modq;

int main(int argc, char *argv[])
{
  MainLoop::init("hoge");
  
  MainLoop::instance()->entryPoint();
  
  //(*Log::instance())(0) << "abc" << endl;
  
  return 0;
}
