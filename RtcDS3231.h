#ifndef __RTCDS3231_H
#define __RTCDS3231_H

#include "Rtc.h"

class RtcDS3231 : public RtcBase {
public:
  RtcDS3231() {}
  virtual bool begin();
  virtual void get(uint8_t& hour, uint8_t& minute, uint8_t& second, uint16_t& year, uint8_t& month, uint8_t& day, uint8_t& dow);
  virtual void getDate(uint16_t& year, uint8_t& month, uint8_t& day, uint8_t& dow);
  virtual void getTime(uint8_t& hour, uint8_t& minute, uint8_t& second);
  virtual uint8_t getHour();
  virtual uint8_t getMinute();
  virtual uint8_t getSecond();
  virtual uint16_t getYear();
  virtual uint8_t getMonth();
  virtual uint8_t getDay();
  virtual uint8_t getDow();
  virtual void set(uint8_t hour, uint8_t minute, uint8_t second, uint16_t year, uint8_t month, uint8_t day, uint8_t dow);
  virtual void setDate(uint16_t year, uint8_t month, uint8_t day, uint8_t dow);
  virtual void setTime(uint8_t hour, uint8_t minute, uint8_t second);
  virtual void setHour(uint8_t hour);
  virtual void setMinute(uint8_t minute);
  virtual void setSecond(uint8_t second);
  virtual void setYear(uint16_t year);
  virtual void setMonth(uint8_t month);
  virtual void setDay(uint8_t day);
  virtual void setDow(uint8_t dow);
protected:
  uint8_t _read(byte address);
  void _write(byte address, byte value);
};

#endif
