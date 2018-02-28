/***
 * DS1307 VCC -> 5V!, Gnd -> Gnd, SCL -> level converter! -> GPIO5, SDA -> level converter! -> GPIO4
 */
//*****Подключаем необходимые библиотеки и файлы*****//
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h> //В данном скетче можно не использовать
#include <Wire.h>
#include "RtcDS3231.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

#define PIN 2 //Пин на который подключена матрица
uint8_t brightness = 250; //Яркость матрицы
boolean WIFI_connected = false; //Переменная для запоминания статуса WiFi (в данном скетче никак не используется) 
bool RTC_OK; //Переменная для проверки состояния обновления времени в RTC модуле
bool tFlag; //флаг для конструкции "не чаще 1 раза в секунду"
uint8_t Hour_old; //хранение текущего часа (для проверки на изменение)
uint8_t RTC_Minute; //хранение минут модуля RTC
uint8_t RTC_Second; //хранение секунд модуля RTC

char TimeStr[12]; //Переменная для хранения строки времени, для вывода на матрицу
char TempStr[10]; //Переменная для хранения строки температуры, для вывода на матрицу
unsigned long timeout2;//Переменная для таймера
String txtInfo = ""; 
int txtInfoLen;
const char* ssid = "xxxxxxxxxxxx"; // your network SSID (name)
const char* password = "xxxxxxxxxxxxxxxxxx"; // your network password

               //**Данные для сервера MQTT**// 
const char* mqtt_server = "192.168.0.106";//Адрес сервера MQTT
unsigned int mqtt_port = 1883;
const char* mqtt_name = "D1_Client";
const char* mqtt_user = "";
const char* mqtt_pass = "";
const char* mqtt_sub_inform = "/pogoda/sensors/tempDS";//Топик в котором находится температура на улице
const char* mqtt_pub_inform = "/D1_Client/OutMes";
WiFiClient ESPclient;
PubSubClient MQTTclient(ESPclient);

                 //** Данные для NTP сервера **//
unsigned int localPort = 2390; // local port to listen for UDP packet
IPAddress timeServerIP;        // time.nist.gov NTP server address
const char* ntpServerName = "pool.ntp.org";
const int NTP_PACKET_SIZE = 48;// NTP time stamp is in the first 48 bytes ofme
const int timeZone = 3;        // GMT+3 for Moscow Time
const long timeZoneOffset = timeZone * 3600;
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

WiFiUDP udp;
RtcDS3231 rtc;
            //** Данные для инициализации матрицы светодиодов **//
Adafruit_NeoPixel strip = Adafruit_NeoPixel(144, PIN, NEO_GRB + NEO_KHZ800); //В данном скетче можно не использовать
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(12, 12, PIN,
  NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT +  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB           + NEO_KHZ800);
int x    = matrix.width();

prog_uint8_t smile[] { //Массив для хранения смайлика
  0x0,0x0,
  0x3,0x80,
  0xc,0x60,
  0x10,0x10,
  0x24,0x48,
  0x44,0x44,
  0x40,0x4,
  0x48,0x24,
  0x48,0x24,
  0x27,0xc8,
  0x10,0x10,
  0xc,0x60,
  0x3,0x80,
  0x0,0x0
};

void setup() { 
  RTC_OK = 0; 
  tFlag = false;
  timeout2 = millis();
  pinMode(BUILTIN_LED, OUTPUT);  
  Serial.begin(115200);
  Serial.println();
  //***Выводим информацию о модуле ESP***//
  Serial.println();
  Serial.print("ESP chip ID: ");
  Serial.println(ESP.getChipId());
  Serial.print("ESP FlashChip ID: ");
  Serial.println(ESP.getFlashChipId());
  Serial.print("ESP size of the current sketch: ");
  Serial.println(ESP.getSketchSize());
  Serial.print("ESP flash chip size/real size: ");
  Serial.print(ESP.getFlashChipSize());
  Serial.print(" / ");
  Serial.println(ESP.getFlashChipRealSize());  
  setup_wifi(); //Подключаемся к WiFi

  //***Выводим СМАЙЛИК***//
  uint16_t bitmapColour = drawRGB24toRGB565(random(0,255),random(0,255),random(0,255));
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(brightness);
  pulseBitmap(smile,3,bitmapColour);
  matrix.show();
  delay(20);
  //*****************************
  
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
  Wire.begin(SDA, SCL); // "Эти Константы определены в файле платы ESP которую Вы выбрали в настройках
  rtc.begin();
  
 // *********** MQTT client *********************
  MQTTclient.setServer(mqtt_server, mqtt_port);
  MQTTclient.setCallback(callback);
  MQTTclient.connect(mqtt_name);
  MQTTclient.subscribe(mqtt_sub_inform);
}

void ntp_update(){ //функция обновления NTP времени

  WiFi.hostByName(ntpServerName, timeServerIP); 
  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  
  unsigned long timeout = millis() + 1000;
  while ((udp.available() < NTP_PACKET_SIZE) && (millis() < timeout)) {}
  int cb = udp.parsePacket();
  if (! cb) {
    Serial.println("no packet yet");
  } 
   else  {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);      
    uint32_t rtcEpoch = rtc.getEpoch();
    Serial.print("RTC epoch = ");
    Serial.println(rtcEpoch);
    rtc.setEpoch(epoch + timeZoneOffset);   
   if (abs(rtcEpoch - timeZoneOffset - epoch) > 1) {
      Serial.print("Updating RTC (epoch difference is ");
      Serial.print(abs(rtcEpoch - timeZoneOffset - epoch));
      Serial.println(')');
      digitalWrite(BUILTIN_LED, LOW); // Turn off builtin LED      
    } else {
      RTC_OK = 1;  
      Hour_old = rtc.getHour();    
      Serial.println("RTC date and time are synchronized.");
      digitalWrite(BUILTIN_LED, HIGH); // Turn on builtin LED
      }
    }
 }

unsigned long sendNTPpacket(IPAddress& address) { //Пакет NTP
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

 void setup_wifi() { //функция подключения к WiFi
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(BUILTIN_LED, ! digitalRead(BUILTIN_LED)); // Toggle builtin LED;
    delay(1000);
    Serial.print(".");
  }
  digitalWrite(BUILTIN_LED, HIGH); // Turn off builtin LED
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  WIFI_connected = true;
} 

void callback(char* topic, byte* payload, unsigned int length) {//callback MQTT
  if(String(topic) == mqtt_sub_inform) {
     txtInfoLen =  length;
     txtInfo = ""; 
    for (int i = 0; i < length; i++) {                                               
       txtInfo += ((char)payload[i]);   
    } 
       txtInfo.toCharArray(TempStr, 10);  // копируем 10 символов в массив TempStr   
       float temp = atof(TempStr);
       txtInfo = (String(temp,1)+"\xB0"+"C"); //Добавляем к температуре  "°С" (в сериал выводит квадратик)
    } 
}

void reconnect(){//функция реконнекта к серверу MQTT
  if(!ESPclient.connected() && WiFi.status() == WL_CONNECTED) {
    Serial.print("Attempting MQTT connection...");
    if(MQTTclient.connect(mqtt_name, mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      MQTTclient.subscribe(mqtt_sub_inform);
    } else {
      Serial.print("failed, rc= ");
      Serial.println(MQTTclient.state());
      delay(3000);
      ESP.restart(); //Если не удалось подключиться к MQTTht,ребутимся через 3 сек. (костыль, потому что я рукожоп)
    }
  }
}

void showText( String t, uint16_t color, int wait, int len){//функция вывода текста на матрицу
  matrix.setTextColor(color);
  
  while (--x > (len*(-1)))
  {
    matrix.fillScreen(0);
    matrix.setBrightness(brightness);
    matrix.setCursor(x, 4);
    matrix.print(utf8rus(t));
    matrix.show();
    delay(wait);
  } 
    x = matrix.width(); 
}

void DisplayTime(void){//функция вывода времени на матрицу
  
   int m = rtc.getMinute();
   int h = rtc.getHour();
    
if (m < 10) {  
   Serial.print("Time: ");
   Serial.print(h);
   Serial.print(":0");
   Serial.println(m);
   sprintf(TimeStr,"   %d:0%d ",h,m );
   showText(TimeStr,matrix.Color(199, 21, 133), 100, 60);
   }
   else {
   Serial.print("Time: ");
   Serial.print(h);
   Serial.print(":");
   Serial.println(m);
   sprintf(TimeStr,"   %d:%d ",h,m );
   showText(TimeStr,matrix.Color(199, 21, 133), 100, 60);
   }
}

/*
const char* dayOfWeek(uint8_t dow) {
  static const char _days[][4] PROGMEM = { "?", "Sun", "Mon", "Tur", "Wed", "Thu", "Fri", "Sat" };
  static char _day[4];

  if (dow > 7)
    dow = 0;
  strcpy_P(_day, (const char *)&_days[dow]);

  return _day;
}
*/
void pulseBitmap(uint8_t* bmp,int times, uint16_t colour){ //функция вывода символа из массива smile[]
  for(int x=1;x<=times;x++){
    for(int i=0;i<=10;i++){
       matrix.setBrightness(i*25);
       matrix.drawBitmap(0,0,bmp,15,14,colour);
       matrix.show();
       delay(20);
    }
    delay(200);
    for(int i=10;i>=0;i--){
       matrix.setBrightness(i*25);
       matrix.drawBitmap(0,0,bmp,15,14,colour);
       matrix.show();
       delay(20);
    }
  }
}
uint16_t drawRGB24toRGB565(uint8_t r, uint8_t g, uint8_t b){ //Плюшка для цветового эффекта
  return ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
}
void loop() {  //ОСНОВНОЙ ЦИКЛ !!!
  
if (WiFi.status() != WL_CONNECTED) { //Проверяем подключение к WiFi
    setup_wifi();
  }  

  //раз в 10 сек. публикуем размер оставшийся в памяти в топик 
  long now = millis();
  long lastMsg = 0;  
  if (now - lastMsg > 10000) {
    char msg[10];
    lastMsg = now;
    sprintf(msg, "%d", (ESP.getFreeHeap()));
    MQTTclient.publish(mqtt_pub_inform, msg);
  }
  
if (RTC_OK == 0) { // Если флаг обновления времени == 0, то запускаем синхронизацию с NTP сервером.
ntp_update();  
}

RTC_Minute =  rtc.getMinute();
RTC_Second =  rtc.getSecond();

if (Hour_old != rtc.getHour()){ //если обновился час, сверяем время с NTP сервером
  Serial.println(Hour_old + "  не равен " + rtc.getHour());
  RTC_OK = 0;  //Раз в час обнуляем переменную, что бы сверить время с Ntp серверо
}

if (!MQTTclient.connected()) { // проверяем подключение к MQTT
    reconnect();
  }

MQTTclient.loop();

if  (RTC_Second == 10 || RTC_Second == 30  && tFlag == false){ // на 10 и 40 секунде выводим время на матрицу
    DisplayTime();
    tFlag = true;  
    timeout2 = millis() + 1000;
    
 } else {
      if (millis() > timeout2 ) tFlag = false; //Эта конструкция не дает выполнять условие чаще 1 раза в секунду
       }  
     
if  (RTC_Second == 20 || RTC_Second == 40 && tFlag == false){ // на 20 и 40 секунде выводим температуру из топика на матрицу
    
    showText(txtInfo,matrix.Color(0, 0, 139), 100, 52);
  
    Serial.println(txtInfo);
    tFlag = true;  
    timeout2 = millis() + 2000;
    
 } else {
      if (millis() > timeout2 ) tFlag = false;
       }      

}

