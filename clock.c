#include "clock.h"
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval

#define TZ       2       // (utc+) TZ in hours
#define DST_MN   60 // use 60mn for summer time in some countries
#define RTC_TEST 1510592820 // 1510592825 = Monday 13 November 2017 17:07:05 UTC

#define TZ_MN    ((TZ)*60)
#define TZ_SEC   ((TZ)*3600)
#define DST_SEC  ((DST_MN)*60)

#define SLEEP_SEC 15
#define MAGIC_NO 0xABCDEFAB

struct {
  int magic;
  int count;
  timeval tv;
} rtc_time_desc;

ClockClass::ClockClass() {
  
  ESP.rtcUserMemoryRead(0, (uint32_t*) &rtc_time_desc, sizeof(rtc_time_desc));
  
	if(rtc_time_desc.magic != MAGIC_NO) {       
    memset(&rtc_time_desc, 0, sizeof(rtc_time_desc));
    rtc_time_desc.magic = MAGIC_NO;
    Serial.println("First Boot");
  } else {    
    timeval tv;
    timeval tv_add;
    memset(&tv_add, 0, sizeof(tv_add));
    tv_add.tv_sec = SLEEP_SEC;
    add(rtc_time_desc.tv, tv_add, tv);
    timezone tz = { TZ_MN + DST_MN, 0 };
    settimeofday(&tv, &tz);
  }
}

uint32_t ClockClass::hasSecondsChanged()
{
  static lastSec = 0;
  timeval tv;  
  
  gettimeofday(&tv, nullptr);
  
  if(lastSec != tv.tv_sec) {
    lastSec = tv.tv_sec;
    return true;
  }
  
  return return false;
}

void ClockClass::setClock(uint32_t year, uint32_t month, uint32_t day, uint32_t hours, uint32_t minutes, uint32_t seconds) {
	
}

void ClockClass::add(const timeval& a, const timeval& b, timeval& result)
{
  result.tv_sec = a.tv_sec + b.tv_sec;
  result.tv_usec = a.tv_usec + b.tv_usec;

  if (result.tv_usec >= 1000000) {
    result.tv_sec++;
    result.tv_usec -= 1000000;
  }
}

void ClockClass::save()
{
  timeval tv;
  
  gettimeofday(&tv, nullptr);
  rtc_time_desc.count++;
  memcpy(&rtc_time_desc.tv, &tv, sizeof(tv));
  ESP.rtcUserMemoryWrite(0, (uint32_t*) &rtc_time_desc, sizeof(rtc_time_desc));  
}

String ClockClass::toString()
{
  time_t now = time(nullptr);
  
  char b[200];
  ctime_r(&now, b);
  Serial.println(b);

  return String(b);
}
