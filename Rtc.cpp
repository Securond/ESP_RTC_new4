#ifdef ESP8266
#include <pgmspace.h>
#else
#include <avr/pgmspace.h>
#endif
#include "Rtc.h"

static const uint8_t daysInMonth[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/***
 * Utility functions
 */

static uint16_t date2days(uint16_t y, uint8_t m, uint8_t d) { // Number of days since 2000/01/01, valid for 2001..2099
  if (y >= 2000)
    y -= 2000;
  uint16_t days = d;
  for (uint8_t i = 1; i < m; ++i)
    days += pgm_read_byte(daysInMonth + i - 1);
  if ((m > 2) && (y % 4 == 0))
    ++days;

  return days + 365 * y + (y + 3) / 4 - 1;
}

static uint32_t time2long(uint16_t days, uint8_t h, uint8_t m, uint8_t s) {
  return ((days * 24L + h) * 60 + m) * 60 + s;
}

static uint8_t conv2d(const char* p) {
  uint8_t v = 0;
  if (('0' <= *p) && (*p <= '9'))
    v = *p - '0';

  return 10 * v + *++p - '0';
}

static char* conv2s(char* s, uint8_t v) {
  *s++ = v / 10 + '0';
  *s++ = v % 10 + '0';

  return s;
}

/***
 * RtcBase class implementation
 */

uint32_t RtcBase::getSecondsSince2000() {
  uint8_t hour, minute, second;
  uint16_t year;
  uint8_t month, day;
  uint16_t days;

  get(hour, minute, second, year, month, day);
  days = date2days(year, month, day);

  return time2long(days, hour, minute, second);
}

void RtcBase::get(uint8_t& hour, uint8_t& minute, uint8_t& second, uint16_t& year, uint8_t& month, uint8_t& day) {
  uint8_t dow;

  get(hour, minute, second, year, month, day, dow);
}

void RtcBase::getDate(uint16_t& year, uint8_t& month, uint8_t& day) {
  uint8_t dow;

  getDate(year, month, day, dow);
}

void RtcBase::setSecondsSince2000(uint32_t t) {
  uint8_t hour, minute, second;
  uint16_t year;
  uint8_t month, day, dow;

  second = t % 60;
  t /= 60;
  minute = t % 60;
  t /= 60;
  hour = t % 24;
  uint16_t days = t / 24;
  dow = (days + 6) % 7 + 1;
  uint8_t leap;
  for (year = 2000; ; ++year) {
    leap = year % 4 == 0;
    if (days < 365 + leap)
      break;
    days -= 365 + leap;
  }
  for (month = 1; ; ++month) {
    uint8_t daysPerMonth = pgm_read_byte(daysInMonth + month - 1);
    if (leap && (month == 2))
      ++daysPerMonth;
    if (days < daysPerMonth)
      break;
    days -= daysPerMonth;
  }
  day = days + 1;
  set(hour, minute, second, year, month, day, dow);
}

void RtcBase::set(uint8_t hour, uint8_t minute, uint8_t second, uint16_t year, uint8_t month, uint8_t day) {
  uint8_t dow = (date2days(year, month, day) + 6) % 7 + 1;

  set(hour, minute, second, year, month, day, dow);
}

void RtcBase::set(const char* date, const char* time) {
  uint8_t hour, minute, second;
  uint16_t year;
  uint8_t month, day, dow;

  // Sample input: date = "Dec 26 2009", time = "12:34:56"
  year = conv2d(date + 9) + 2000;
  // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec 
  switch (date[0]) {
    case 'J':
      month = date[1] == 'a' ? 1 : date[2] == 'n' ? 6 : 7;
      break;
    case 'F':
      month = 2;
      break;
    case 'A':
      month = date[2] == 'r' ? 4 : 8;
      break;
    case 'M':
      month = date[2] == 'r' ? 3 : 5;
      break;
    case 'S':
      month = 9;
      break;
    case 'O':
      month = 10;
      break;
    case 'N':
      month = 11;
      break;
    case 'D':
      month = 12;
      break;
  }
  day = conv2d(date + 4);
  hour = conv2d(time);
  minute = conv2d(time + 3);
  second = conv2d(time + 6);
  dow = (date2days(year, month, day) + 6) % 7 + 1;
  set(hour, minute, second, year, month, day, dow);
}

void RtcBase::set(const __FlashStringHelper* date, const __FlashStringHelper* time) {
  char _date[11], _time[8];

  memcpy_P(_date, date, 11);
  memcpy_P(_time, time, 8);
  set(_date, _time);
}

void RtcBase::setDate(uint16_t year, uint8_t month, uint8_t day) {
  uint8_t dow = (date2days(year, month, day) + 6) % 7 + 1;

  setDate(year, month, day, dow);
}

void RtcBase::setDate(const char* date) {
  uint16_t year;
  uint8_t month, day, dow;

  // Sample input: date = "Dec 26 2009"
  year = conv2d(date + 9) + 2000;
  // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec 
  switch (date[0]) {
    case 'J':
      month = date[1] == 'a' ? 1 : date[2] == 'n' ? 6 : 7;
      break;
    case 'F':
      month = 2;
      break;
    case 'A':
      month = date[2] == 'r' ? 4 : 8;
      break;
    case 'M':
      month = date[2] == 'r' ? 3 : 5;
      break;
    case 'S':
      month = 9;
      break;
    case 'O':
      month = 10;
      break;
    case 'N':
      month = 11;
      break;
    case 'D':
      month = 12;
      break;
  }
  day = conv2d(date + 4);
  dow = (date2days(year, month, day) + 6) % 7 + 1;
  setDate(year, month, day, dow);
}

void RtcBase::setDate(const __FlashStringHelper* date) {
  char _date[11];

  memcpy_P(_date, date, 11);
  setDate(_date);
}

void RtcBase::setTime(const char* time) {
  uint8_t hour, minute, second;

  // Sample input: time = "12:34:56"
  hour = conv2d(time);
  minute = conv2d(time + 3);
  second = conv2d(time + 6);
  setTime(hour, minute, second);
}

void RtcBase::setTime(const __FlashStringHelper* time) {
  char _time[8];

  memcpy_P(_time, time, 8);
  setTime(_time);
}

const char dateDelimiter = '.';
const char timeDelimiter = ':';

char* RtcBase::dateTimeToStr(char* str) { // dd.mm.yyyy hh:mm:ss
  char* p = str;
  uint8_t hour, minute, second;
  uint16_t year;
  uint8_t month, day;

  get(hour, minute, second, year, month, day);
  p = conv2s(p, day);
  *p++ = dateDelimiter;
  p = conv2s(p, month);
  *p++ = dateDelimiter;
  *p++ = '2'; // year from 2000 to 2099
  *p++ = '0';
  p = conv2s(p, year - 2000);
  *p++ = ' ';
  p = conv2s(p, hour);
  *p++ = timeDelimiter;
  p = conv2s(p, minute);
  *p++ = timeDelimiter;
  p = conv2s(p, second);
  *p = 0; // NULL

  return str;
}

char* RtcBase::dateToStr(char* str) { // dd.mm.yyyy
  char* p = str;
  uint16_t year;
  uint8_t month, day;

  getDate(year, month, day);
  p = conv2s(p, day);
  *p++ = dateDelimiter;
  p = conv2s(p, month);
  *p++ = dateDelimiter;
  *p++ = '2'; // year from 2000 to 2099
  *p++ = '0';
  p = conv2s(p, year - 2000);
  *p = 0; // NULL

  return str;
}

char* RtcBase::timeToStr(char* str) { // hh:mm:ss
  char* p = str;
  uint8_t hour, minute, second;

  getTime(hour, minute, second);
  p = conv2s(p, hour);
  *p++ = timeDelimiter;
  p = conv2s(p, minute);
  *p++ = timeDelimiter;
  p = conv2s(p, second);
  *p = 0; // NULL

  return str;
}
