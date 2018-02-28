#include <Wire.h>
#include "RtcDS3231.h"

#define DS3231_ADDRESS    0x68 // I2C Slave address

/* DS3231 Registers. Refer Sec 8.2 of application manual */
#define DS3231_SEC_REG    0x00
#define DS3231_MIN_REG    0x01
#define DS3231_HOUR_REG   0x02
#define DS3231_WDAY_REG   0x03
#define DS3231_MDAY_REG   0x04
#define DS3231_MONTH_REG  0x05
#define DS3231_YEAR_REG   0x06

#define DS3231_CONTROL_REG      0x0E
#define DS3231_STATUS_REG       0x0F
#define DS3231_AGING_OFFSET_REG 0x0F
#define DS3231_TMP_UP_REG       0x11
#define DS3231_TMP_LOW_REG      0x12

/***
 * RtcDS3231 class implementation
 */

bool RtcDS3231::begin() {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)DS3231_CONTROL_REG);
  Wire.write((byte)0b00011100);
  byte status = Wire.endTransmission();

  return (status == 0);
}

void RtcDS3231::get(uint8_t& hour, uint8_t& minute, uint8_t& second, uint16_t& year, uint8_t& month, uint8_t& day, uint8_t& dow) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)DS3231_SEC_REG);
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 7);
  second = bcd2bin(Wire.read());
  minute = bcd2bin(Wire.read());
  hour = bcd2bin(Wire.read() & ~0b11000000); // Ignore 24 Hour bit
  dow = Wire.read();
  day = bcd2bin(Wire.read());
  month = bcd2bin(Wire.read() & ~0b10000000);
  year = bcd2bin(Wire.read()) + 2000;
}

void RtcDS3231::getDate(uint16_t& year, uint8_t& month, uint8_t& day, uint8_t& dow) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)DS3231_WDAY_REG);
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 4);
  dow = Wire.read();
  day = bcd2bin(Wire.read());
  month = bcd2bin(Wire.read() & ~0b10000000);
  year = bcd2bin(Wire.read()) + 2000;
}

void RtcDS3231::getTime(uint8_t& hour, uint8_t& minute, uint8_t& second) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)DS3231_SEC_REG);
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 3);
  second = bcd2bin(Wire.read());
  minute = bcd2bin(Wire.read());
  hour = bcd2bin(Wire.read() & ~0b11000000); // Ignore 24 Hour bit
}

inline uint8_t RtcDS3231::getHour() {
  return bcd2bin(_read(DS3231_HOUR_REG) & ~0b11000000); // Ignore 24 Hour bit
}

inline uint8_t RtcDS3231::getMinute() {
  return bcd2bin(_read(DS3231_MIN_REG));
}

inline uint8_t RtcDS3231::getSecond() {
  return bcd2bin(_read(DS3231_SEC_REG));
}

inline uint16_t RtcDS3231::getYear() {
  return bcd2bin(_read(DS3231_YEAR_REG)) + 2000;
}

inline uint8_t RtcDS3231::getMonth() {
  return bcd2bin(_read(DS3231_MONTH_REG) & ~0b10000000);
}

inline uint8_t RtcDS3231::getDay() {
  return bcd2bin(_read(DS3231_MDAY_REG));
}

inline uint8_t RtcDS3231::getDow() {
  return _read(DS3231_WDAY_REG);
}

void RtcDS3231::set(uint8_t hour, uint8_t minute, uint8_t second, uint16_t year, uint8_t month, uint8_t day, uint8_t dow) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)DS3231_SEC_REG);  // beginning from SEC Register address

  Wire.write((byte)bin2bcd(second));
  Wire.write((byte)bin2bcd(minute));
  Wire.write((byte)bin2bcd(hour));
  Wire.write((byte)dow);
  Wire.write((byte)bin2bcd(day));
  Wire.write((byte)bin2bcd(month));
  Wire.write((byte)bin2bcd(year - 2000));
  Wire.endTransmission();
}

void RtcDS3231::setDate(uint16_t year, uint8_t month, uint8_t day, uint8_t dow) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)DS3231_WDAY_REG);

  Wire.write((byte)dow);
  Wire.write((byte)bin2bcd(day));
  Wire.write((byte)bin2bcd(month));
  Wire.write((byte)bin2bcd(year - 2000));
  Wire.endTransmission();
}

void RtcDS3231::setTime(uint8_t hour, uint8_t minute, uint8_t second) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)DS3231_SEC_REG);  // beginning from SEC Register address

  Wire.write((byte)bin2bcd(second));
  Wire.write((byte)bin2bcd(minute));
  Wire.write((byte)bin2bcd(hour));
  Wire.endTransmission();
}

inline void RtcDS3231::setHour(uint8_t hour) {
  _write(DS3231_HOUR_REG, bin2bcd(hour));
}

inline void RtcDS3231::setMinute(uint8_t minute) {
  _write(DS3231_MIN_REG, bin2bcd(minute));
}

inline void RtcDS3231::setSecond(uint8_t second) {
  _write(DS3231_SEC_REG, bin2bcd(second));
}

inline void RtcDS3231::setYear(uint16_t year) {
  _write(DS3231_YEAR_REG, bin2bcd(year - 2000));
}

inline void RtcDS3231::setMonth(uint8_t month) {
  _write(DS3231_MONTH_REG, bin2bcd(month));
}

inline void RtcDS3231::setDay(uint8_t day) {
  _write(DS3231_MDAY_REG, bin2bcd(day));
}

inline void RtcDS3231::setDow(uint8_t dow) {
  _write(DS3231_WDAY_REG, dow);
}

uint8_t RtcDS3231::_read(byte address) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)address);
  Wire.endTransmission();

  Wire.requestFrom(DS3231_ADDRESS, 1);

  return Wire.read();
}

void RtcDS3231::_write(byte address, byte value) {
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write((byte)address);
  Wire.write((byte)value);
  Wire.endTransmission();
}
