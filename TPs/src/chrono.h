

#ifndef ___CHRONO_H__
#define ___CHRONO_H__

#include "time.h"

class chrono
{
  public:
    chrono():_t0(0){}    
    void start(){_t0 = ::clock();}
    double elapsed(){return double(::clock() - _t0) / CLOCKS_PER_SEC;}
    
    static int verbose;
    
  private:
    clock_t _t0;
}; 

#endif
