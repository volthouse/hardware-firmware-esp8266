#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <Arduino.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
//#include <String.h>

class ClockClass {
private:
  uint32_t sleepTimeMs;
  void add(const timeval& a, const timeval& b, timeval& result);

public:
	ClockClass(uint32_t sleepTimeMs);
  uint32_t hasSecondsChanged();  
	static void setClock(uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t minutes, uint32_t seconds);
	static String toString();
  static void save();
  
};

#endif _CLOCK_H_
