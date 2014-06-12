// Log.h

#ifndef LOG_H
#define LOG_H

#include "modq/Singleton.h"
#include <string>
#include <ostream>

namespace modq{

  class LogFormatter{
  public:
    LogFormatter()
    : _format("[%T %L %N] %M")
    // %T: time (according to the _timeFormat)
    // %L: message level (DEBUG/INFO/WARN/ERROR)
    // %N: module name
    // %M: message
    , _timeFormat("%a, %d %b %Y %T %z") // RFC2822
    {}
    ~LogFormatter(){}
    
    virtual std::string format(const std::string &message, int level){
      std::string ret;
      return ret; // TODO
    }
  protected:
    std::string _format;
    std::string _timeFormat;
  };
  
  class LogBuf : public std::basic_streambuf<char> {
  public:
    LogBuf(){}
    ~LogBuf(){}
  };
  
  class Log;
  class Log : public SimpleSingleton<Log> {
    enum {maxLevel = 4};

  protected: // singleton: constructor not public
    Log(){
      for(int i=0;i<maxLevel; i++)
        _log[i] = new std::ostream(new LogBuf);
    }
    ~Log(){
      for(int i=0;i<maxLevel; i++)
        delete _log[i];
    }
    
  public:  
    std::ostream & operator()(int level){
      if(level < 0 || level >= maxLevel)throw;// TODO
      return *(_log[level]);
    }
    
  private:
    std::ostream *_log[maxLevel];
  };

}

#endif
