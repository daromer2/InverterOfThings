#ifndef TICKCOUNTER_H
#define TICKCOUNTER_H
#include <Arduino.h>

//Tick count that takes longer to roll over
class TickCounter
{
  private:
  
    unsigned long long _tickCounter = 0;
    unsigned long _lastCpuTickCount = 0;

  public:
  
    //The tick counter will roll over in 53 seconds, this must be called at least every 26 sec
    unsigned long long getTicks()
    {
      unsigned long ticks = millis();
      unsigned long delta = (ticks - _lastCpuTickCount);
      _lastCpuTickCount = ticks;
      _tickCounter += delta;
      return _tickCounter;
    }

    unsigned long long getMillis()
    {
      return millis();
    }

    unsigned long getSeconds()
    {
      return millis() /1000;
    }
};

class PollDelay
{
  private:
    
    unsigned long long _startMillis;
    TickCounter* _tickCounter;

  public:
    
    PollDelay(TickCounter& tickCounter)
    {
      _tickCounter = &tickCounter;
      _startMillis = _tickCounter->getMillis();
    }
  
    void reset()
    {
      _startMillis = _tickCounter->getMillis();
    }

    //Call this once every 26 seconds or it'll roll over
    int compare(unsigned int millisSinceStart)
    {
      return (int)( _tickCounter->getMillis() - (_startMillis + millisSinceStart) );
    }
};

#endif
