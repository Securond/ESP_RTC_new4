#ifndef __RTC_H
#define __RTC_H

#include <Arduino.h>

#define EPOCH_TIME_OFF  946684800L // This is 2000-jan-01 00:00:00 in epoch time
#define SECONDS_PER_DAY 86400L

class RtcBase {
public:
  RtcBase() {}
  virtual bool begin() = 0;
  virtual uint32_t getSecondsSince2000();
  virtual uint32_t getEpoch() { return getSecondsSince2000() + EPOCH_TIME_OFF; } // UNIX time
  virtual void get(uint8_t& hour, uint8_t& minute, uint8_t& second, uint16_t& year, uint8_t& month, uint8_t& day, uint8_t& dow) = 0;
  virtual void get(uint8_t& hour, uint8_t& minute, uint8_t& second, uint16_t& year, uint8_t& month, uint8_t& day);
  virtual void getDate(uint16_t& year, uint8_t& month, uint8_t& day, uint8_t& dow) = 0;
  virtual void getDate(uint16_t& year, uint8_t& month, uint8_t& day);
  virtual void getTime(uint8_t& hour, uint8_t& minute, uint8_t& second) = 0;
  virtual uint8_t getHour() = 0;
  virtual uint8_t getMinute() = 0;
  virtual uint8_t getSecond() = 0;
  virtual uint16_t getYear() = 0;
  virtual uint8_t getMonth() = 0;
  virtual uint8_t getDay() = 0;
  virtual uint8_t getDow() = 0;
  virtual void setSecondsSince2000(uint32_t t);
  virtual void setEpoch(uint32_t epoch) { setSecondsSince2000(epoch - EPOCH_TIME_OFF); } // UNIX time
  virtual void set(uint8_t hour, uint8_t minute, uint8_t second, uint16_t year, uint8_t month, uint8_t day, uint8_t dow) = 0;
  virtual void set(uint8_t hour, uint8_t minute, uint8_t second, uint16_t year, uint8_t month, uint8_t day);
  virtual void set(const char* date, const char* time);
  virtual void set(const __FlashStringHelper* date, const __FlashStringHelper* time);
  virtual void setDate(uint16_t year, uint8_t month, uint8_t day, uint8_t dow) = 0;
  virtual void setDate(uint16_t year, uint8_t month, uint8_t day);
  virtual void setDate(const char* date);
  virtual void setDate(const __FlashStringHelper* date);
  virtual void setTime(uint8_t hour, uint8_t minute, uint8_t second) = 0;
  virtual void setTime(const char* time);
  virtual void setTime(const __FlashStringHelper* time);
  virtual void setHour(uint8_t hour) = 0;
  virtual void setMinute(uint8_t minute) = 0;
  virtual void setSecond(uint8_t second) = 0;
  virtual void setYear(uint16_t year) = 0;
  virtual void setMonth(uint8_t month) = 0;
  virtual void setDay(uint8_t day) = 0;
  virtual void setDow(uint8_t dow) = 0;
  virtual char *dateTimeToStr(char* str);
  virtual char *dateToStr(char* str);
  virtual char *timeToStr(char* str);
protected:
  uint8_t bcd2bin(uint8_t val) { return val - 6 * (val >> 4); }
  uint8_t bin2bcd(uint8_t val) { return val + 6 * (val / 10); }
};

#endif
