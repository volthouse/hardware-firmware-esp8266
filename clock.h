#ifndef _CLOCK_H_
#define _CLOCK_H_

#include <Arduino.h>

class ClockClass {
private:
  add(const timeval& a, const timeval& b, timeval& result);  

public:
	ClockClass();

  uint32_t hasSecondsChanged();

	static void setClock(uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t minutes, uint32_t seconds);
	static String toString();
  static void save();
  
};

extern ClockClass Clock;

#endif _CLOCK_H_