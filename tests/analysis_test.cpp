#include "../Source/Analysis.h"
#include <cassert>
int main(){Analysis silence;silence.push(0,0);assert(std::isfinite(silence.rmsDb()));Analysis mono,side;for(int i=0;i<48000;++i){float v=.5f*std::sin(float(i)*.1f);mono.push(v,v);side.push(v,-v);}assert(std::abs(mono.rmsDb()+9.03f)<.02f);assert(mono.sideRatio()==0);assert(std::abs(mono.correlation()-1)<.001f);assert(std::abs(side.sideRatio()-1)<.001f);assert(std::abs(side.correlation()+1)<.001f);}
