// MainLoop.h

#ifndef MAINLOOP_H
#define MAINLOOP_H

#include "modq/Singleton.h"
#include <string>

namespace modq{

  class MainLoop;
  class MainLoop : public Singleton<MainLoop> {
  private: // singleton: constructor not public
    MainLoop(const char *modName) : _modName(modName){}
    ~MainLoop(){}

  public:  
    static void init(const char *modName){
      MainLoop *th = new MainLoop(modName);
      setThis(th);
    }

    void entryPoint();
    const std::string & getModName()const{return _modName;}
    
  private:
    std::string _modName;

  };

}

#endif
