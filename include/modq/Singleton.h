// Singleton.h
#ifndef SINGLETON_H
#define SINGLETON_H

namespace modq{

  // singleton: just a interface of singleton
  // inherent classes must implement create()/destroy() function
  
  template<class T> class Singleton{
  protected: // singleton: constructor not public
    Singleton(){}
    ~Singleton(){}

  // singleton implementation
  public:
    static T * instance(){return _this;}
  protected:
    static void setThis(T *th){
      if(_this) throw; // TODO
      _this = th;
    }

  private:
    static T *_this;
  };

  template<class T> T* Singleton<T>::_this = 0;

  // simplesingleton: with create()/destroy(), but
  // no parameter allowed for create()
  
  template<class T> class SimpleSingleton{
   protected: // singleton: constructor not public
    SimpleSingleton(){}
    ~SimpleSingleton(){}

  // singleton implementation
  public:
    static T * instance(){return _this;}

    void create(T *th){
      if(_this) throw; // TODO
      _this = new T;
    }
    void destroy(){
      if(!_this) throw; // TODO
      _this = 0;
    }

  private:
    static T *_this;
  };

  template<class T> T* SimpleSingleton<T>::_this = 0;

}

#endif
