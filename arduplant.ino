
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <DS3231.h>
#include <Wire.h>

//***************DHT
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)
sensors_event_t event;
sensor_t sensor;
float humi,temp;
float umi_temp [2];
#define DHTPIN 4     // DHT PIN
DHT_Unified dht(DHTPIN, DHTTYPE);
//***************Relay
#define relay_light 12
#define relay_water 11
#define relay_fan  8
#define relay_heater  7
//****************Photo resistor
#define photo_resi1 A0
#define photo_resi2 A1
#define photo_resi3 A2
int photo_read1;
int photo_read2;
int photo_read3;
//***************Distance
#define echoPin 9 
#define trigPin 10
long duration; // variable for the duration of sound wave travel
int distance; // variable for the distance measurement
//***************Soil humidity
#define soil_Pin A3
int soil_read1;
//***************Rtc
DS3231 clock;
RTClib RTC;
bool century = false;
bool h12Flag;
bool pmFlag;
int year_read;
int month_read;
int day_read;
int hour_read;
int minute_read;
int second_read;
byte year_set;
byte month_set;
byte day_set;
byte dow_set;
byte hour_set;
byte minute_set;
byte second_set;

//*****************Time_check
unsigned long time_now;
unsigned long time_period;
unsigned long time_period_dark;
unsigned long time_period_dark_now;
unsigned long time_dark_unix;
unsigned long time_minute_dark_unix;
unsigned long time_dark_on;
unsigned long time_period_temp;
unsigned long time_period_temp_now;
unsigned long time_period_soil;
unsigned long time_period_soil_now;

int time_dark_hour_begin;
int time_dark_hour_end;
int time_soil_humidity;
int time_temp_humi;
int time_heater_on;
int time_hour_rtc_ntp;
//*****************Temperature
float temp_min;
float temp_max;


void setup() {
  Serial.begin(9600);
  Wire.begin();

  //Set Time
  //set_date("2008 10 21 95600"); //YEAR(2) MONTH(2) DAY(2) DAY_OF_WEEK(1) HOUR(2) MIN(2) SEC(2)
  //set_date("20 08 10 2 15 58 00");
  //set_date("20 10 08 5 18 20 00");
  //set_date("2104111152500");


  //Relay
  relay_set(relay_water,LOW);
  relay_set(relay_fan,LOW);
  relay_set(relay_heater,LOW);
  relay_set(relay_light,LOW);
  pinMode(relay_light,OUTPUT);
  pinMode(relay_water,OUTPUT);
  pinMode(relay_fan,OUTPUT);
  pinMode(relay_heater,OUTPUT);
  delay(2000);
  
  // Temp 
  temp_min = 17;
  temp_max = 23;// temp set fan on, default = 30
  
  // Time check
  time_now = millis(); // setting the time now 
  
  time_dark_hour_begin = 8; // begin dark hour, default = 8
  time_dark_hour_end = 16; // end dark hour, default = 16
  time_soil_humidity = 60; // time between reads in minutes, default = 60
  time_temp_humi = 15; // time between reads in minutes, default = 30
  time_hour_rtc_ntp = 23; // hour to sync the rtc with ntp, default = 23
  time_heater_on = 15; // minutes heater stay on in minutes, default = 5

  time_period = 1.0; // check period in minutes
  time_period *= 60.0 * 1000.0; // transform check period seconds in minutes (seconds * 60 seconds * 1000 millis)
  rtc_read();
  Serial.print("Actual hour --->");
  Serial.print(hour_read);
  Serial.print(":");
  Serial.print(minute_read);
  Serial.print(":");
  Serial.println(second_read);
  Serial.print("Hour begin --->");
  Serial.println(time_dark_hour_begin);
  Serial.print("Hour end --->");
  Serial.println(time_dark_hour_end);
  ////////////////////////////////////////////////if (hour_read <= time_dark_hour_begin || hour_read >= time_dark_hour_end)
  if (hour_read < time_dark_hour_begin || hour_read >= time_dark_hour_end){
    
  if (hour_read < time_dark_hour_begin ){
        Serial.println("Entrou no IF <=");
    if (minute_read != 0){
      Serial.println("!=0");
      time_period_dark = (time_dark_hour_begin-1) - hour_read ;
      time_minute_dark_unix = 60-minute_read;
    }
    else{
      Serial.println("==0");
      time_period_dark = 24.0 - hour_read + time_dark_hour_begin;
      time_minute_dark_unix=0;
    }
    delay(4000);
  }

  
  if (hour_read > time_dark_hour_begin ){
    Serial.println("Entrou no IF >=");
    if (minute_read != 0){
      Serial.println("!=0");
      time_period_dark = 24-hour_read + (time_dark_hour_begin - 1);
      time_minute_dark_unix = 60-minute_read;
    }
    else{
      Serial.println("==0");
      time_period_dark = 24.0 - hour_read + time_dark_hour_begin;
      time_minute_dark_unix = 0;
    }
    Serial.println(time_period_dark);

  }
      
      relay_set(relay_light,HIGH);
      Serial.println("relay_light set ON");
  }
  ///////////////////////////////////////////////////////////////////if(hour_read > time_dark_hour_begin && hour_read < time_dark_hour_end)
 if(hour_read >= time_dark_hour_begin && hour_read < time_dark_hour_end) {
  relay_set(relay_light,LOW);
  if (minute_read != 0){
  time_period_dark = (time_dark_hour_end-1) - hour_read;
  time_minute_dark_unix = 60-minute_read; 
  Serial.print("LOGGED in the ELSE -- ");
  Serial.println(time_period_dark);
  Serial.println("relay_light set OFF");
  } 
  else {
    time_period_dark = time_dark_hour_end-hour_read;
    time_minute_dark_unix = 0;
  }
 }
  
  
  time_period_dark *= 60.0;
  time_period_dark *= 60.0 * 1000.0;
  time_period_dark += time_minute_dark_unix * 60 * 1000;
  Serial.println(time_period_dark);
  time_period_dark_now = millis(); // setting the time now

  time_period_temp = time_temp_humi;
  time_period_temp *= 60.0 * 1000.0;
  time_period_temp_now = millis();

  
  time_period_soil = 360.0;
  time_period_soil *= 60.0 * 1000.0;
  time_period_soil_now = millis();
  

  
  // Temp, Humi
  dht.begin();
  
  //Distance
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  //Soil
  pinMode(soil_Pin,INPUT);
}


void loop(){
  if (millis() - time_now > time_period ){
//      time_now = millis();
//
//rtc_read();
//DateTime now = RTC.now();
//DateTime add_minutes;
//Serial.println("Year " + String(year_read));
//Serial.println("Month " + String(month_read));
//Serial.println("Day " + String(day_read));
//Serial.println("Hour " + String(hour_read));
//Serial.println("Minute " + String(minute_read));
//Serial.println("Second " + String(second_read));
//Serial.println("Unix Time ");
//Serial.print(now.unixtime(),DEC);
//add_minutes=now.unixtime()+60*60;
//Serial.print(" Tempo + 60 minutos ");
//Serial.print(add_minutes.hour(),DEC);
//Serial.print(":");
//Serial.print(add_minutes.minute(),DEC);
//Serial.println("--------------------------------");
    time_now = millis();
   
    Serial.print("time_now ");
    Serial.print(millis());
    Serial.print(" --->  ");
    Serial.println(digitalRead(relay_light));
    Serial.println(hour_read);
  }

  if (millis() - time_period_dark_now >= time_period_dark){
    rtc_read();
    DateTime now = RTC.now();
    Serial.println("Next check RTC");
    
    if (hour_read >= time_dark_hour_begin && hour_read < time_dark_hour_end){
        if (digitalRead(relay_light)==LOW){
          relay_set(relay_light,LOW);  
          Serial.println("LIGHT OFF");
          Serial.println(digitalRead(relay_light));
          Serial.print(hour_read);
          Serial.print(":");
          Serial.print(minute_read);
          Serial.print(":");
          Serial.println(second_read);
          time_period_dark = time_dark_hour_end-time_dark_hour_begin;
          time_minute_dark_unix = 0.0;
        }
      else{
        Serial.print(hour_read);
        Serial.print(":");
        Serial.print(minute_read);
        Serial.print(":");
        Serial.println(second_read);
        Serial.println("ADD 1 SECONDS in LIGHT ON");
        time_minute_dark_unix = 0.51;//Default = 0.5
        time_period_dark = 0.0;
      }
  }
  else{
      if (hour_read < time_dark_hour_begin || hour_read >= time_dark_hour_end ){
          if (digitalRead(relay_light)==HIGH){
            relay_set(relay_light,HIGH); 
            Serial.println("LIGHT ON"); 
            Serial.println(digitalRead(relay_light));
            Serial.print(hour_read);
            Serial.print(":");
            Serial.print(minute_read);
            Serial.print(":");
            Serial.println(second_read);
            time_period_dark = 24-(time_dark_hour_end-time_dark_hour_begin);
            time_minute_dark_unix = 0.0;
          }
          
        else{
          Serial.print(hour_read);
          Serial.print(":");
          Serial.print(minute_read);
          Serial.print(":");
          Serial.println(second_read);
          Serial.println("ADD 1 SECONDS in LIGHT OFF");
          time_minute_dark_unix = 0.51;//Default = 0.5
          time_period_dark = 0.0;
        }
      }
    }
    
    time_period_dark *= 60.0 * 60.0 * 1000.0;
    time_period_dark += time_minute_dark_unix * 60 * 1000;
    time_period_dark_now = millis();
    Serial.print("TIME IN NEW TIME_PERIOD_DARK --->");
    Serial.println(time_period_dark);
    Serial.print("TIME IN NEW TIME_PERIOD_DARK_NOW--->");
    Serial.println(millis() - time_period_dark_now);
  }

  if (millis() - time_period_temp_now > time_period_temp){
    humi_temp_read();   
    Serial.print("Humidity= ");
    Serial.print(humi);
    Serial.print("    Temperature= ");
    Serial.println(temp);
    if (temp > temp_min && temp < temp_max){
      relay_set(relay_fan,LOW);
      relay_set(relay_heater,LOW);
      Serial.println("TEMPERATURE NORMAL    Relay Fan LOW  ---  Relay Fan LOW");
      time_period_temp = time_temp_humi;  
    }
      else if (temp <= temp_min){
        time_period_temp = time_heater_on;
        relay_set(relay_heater,HIGH);
        Serial.println("Relay Heater HIGH");    
      }
      else if (temp >= temp_max){
        time_period_temp = time_heater_on ;
        if (digitalRead(relay_fan)==HIGH)
        {
          relay_set(relay_fan,HIGH);
          Serial.println("Relay Fan HIGH");
        }
      }
      time_period_temp *= 1000.0 * 60.0;
      time_period_temp_now = millis();
      distance_read();
      Serial.print("ECHO ----->  ");
      Serial.print(distance);
      Serial.println(" cm ");
  }
  
   if (millis() - time_period_soil_now > time_period_soil){
    time_period_soil_now = millis();
    Serial.println("time_period_soil");
  }
  
  
////****************RELAY
//  relay_set(relay_light, LOW);
//  delay(2000);
//
//  relay_set(relay_water, LOW);
//  delay(2000);
//
//  relay_set(relay_fan, LOW);
//  delay(2000);
//
//  relay_set(relay_heater, LOW);
//  delay(2000);
//  
//  relay_set(relay_light, HIGH);
//  delay(2000);
//
//  relay_set(relay_water, HIGH);
//  delay(2000);
//
//  relay_set(relay_fan, HIGH);
//  delay(1000);
//
//  relay_set(relay_heater, HIGH);
//  delay(1000);
  
////***************TEMP HUMI
//  humi_temp_read();
//  if (isnan(event.temperature)) 
//  {
//  Serial.println(F("Error reading temperature!"));
//  }
//  else 
//  {
//    Serial.print(F("Temperature: "));
//    Serial.print(event.temperature);
//    Serial.println(F("°C"));
//  }
//  // Get humidity event and print its value.
//  dht.humidity().getEvent(&event);
//  if (isnan(event.relative_humidity)) 
//  {
//    Serial.println(F("Error reading humidity!"));
//  }
//  else 
//  {
//    Serial.print(F("Humidity: "));
//    Serial.print(event.relative_humidity);
//    Serial.println(F("%"));
//  }

//***************PHOTO RESISTOR
//photo_read();
//  if (photo_read1 < 900)
//  {
//    Serial.println("1.LIGHT/ON " + String(photo_read1)); 
//  }
//  else if (photo_read1 > 1000)
//  {
//    Serial.println("1.LIGHT/OFF "+ String(photo_read1));
//  }
//    if (photo_read2 < 900)
//  {
//    Serial.println("2.LIGHT/ON " + String(photo_read2)); 
//  }
//  else if (photo_read2 > 1000)
//  {
//    Serial.println("2.LIGHT/OFF "+ String(photo_read2));
//  }
//    if (photo_read3 < 900)
//  {
//    Serial.println("3.LIGHT/ON " + String(photo_read3)); 
//  }
//  else if (photo_read3 > 1000)
//  {
//    Serial.println("3.LIGHT/OFF "+ String(photo_read3));
//  }

////***************DISTANCE
//  distance_read();
//  Serial.print("Distance: ");
//  Serial.print(distance);
//  Serial.println(" cm");

////***************SOIL
//soil_read();
//  if (soil_read1 < 1000)
//  {
//    Serial.println("WET SOIL " + String(soil_read1));
//  }
//  else if (soil_read1 > 1000)
//  {
//    Serial.println("DRY SOIL "+ String(soil_read1));
//  }

//***************DATE
//rtc_read();
//DateTime now = RTC.now();
//DateTime add_minutes;
//Serial.println("Year " + String(year_read));
//Serial.println("Month " + String(month_read));
//Serial.println("Day " + String(day_read));
//Serial.println("Hour " + String(hour_read));
//Serial.println("Minute " + String(minute_read));
//Serial.println("Second " + String(second_read));
//Serial.println("Unix Time ");
//Serial.print(now.unixtime(),DEC);
//add_minutes=now.unixtime()+60*60;
//Serial.print("Tempo + 60 minutos ");
//Serial.print(add_minutes.hour(),DEC);
//Serial.print(":");
//Serial.print(add_minutes.minute(),DEC);
//Serial.println("--------------------------------");
//    
}

void photo_read(){
  photo_read1 = analogRead(photo_resi1);
  photo_read2 = analogRead(photo_resi2);
  photo_read3 = analogRead(photo_resi3);
}

int relay_set(int relay_nu,int relay_state){
  relay_state= !relay_state;
  digitalWrite(relay_nu,relay_state);
  Serial.print("ENTROU NO RELAY   ---- > ");
  Serial.println(digitalRead(relay_light));
}

void humi_temp_read(){
  dht.temperature().getSensor(&sensor);
  dht.humidity().getSensor(&sensor);
  dht.temperature().getEvent(&event);
    if (isnan(event.temperature)){
      temp=999.99;
      humi=999.99;
    }
    else{
      temp = event.temperature;
      //temp=30;//Only for testing
      dht.humidity().getEvent(&event);
      humi = event.relative_humidity; 
    }
}
 
void distance_read(){
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = (duration-10) * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
}

void soil_read(){
  soil_read1 = analogRead(soil_Pin);
}

void rtc_read(){
  year_read =clock.getYear();
  month_read = clock.getMonth(century);
  day_read = clock.getDate();
  // dow_read=2;
  hour_read = clock.getHour(h12Flag, pmFlag); //24-hr
  minute_read = clock.getMinute();
  second_read = clock.getSecond();
  }
  
void read_all(){
  double teste[3];
  teste[0]=1123.4;
  teste[1]=256.78;
  teste[2]=3.910;
  
  int soil_all;
  float temp_all,humi_all;
  ////Soil
  soil_read();
  soil_all=soil_read1;

  humi_temp_read();
  temp_all = event.temperature;
  humi_all = event.relative_humidity;
  return teste; 
}

String set_date(String input_date){
  char date_set[20];
  input_date.toCharArray(date_set, 20);
  byte temp1, temp2;
  //Year
  temp1 = (byte)date_set[0] - 48;
  temp2 = (byte)date_set[1] - 48;
  year_set = temp1 * 10 + temp2;
  //month
  temp1 = (byte)date_set[2] - 48;
  temp2 = (byte)date_set[3] - 48;
  month_set = temp1 * 10 + temp2;
  //date
  temp1 = (byte)date_set[4] - 48;
  temp2 = (byte)date_set[5] - 48;
  day_set = temp1 * 10 + temp2;
  //Day of Week
  dow_set = (byte)date_set[6] - 48;   
  //Hour
  temp1 = (byte)date_set[7] - 48;
  temp2 = (byte)date_set[8] - 48;
  hour_set = temp1 * 10 + temp2;
  //Minute
  temp1 = (byte)date_set[9] - 48;
  temp2 = (byte)date_set[10] - 48;
  minute_set = temp1 * 10 + temp2;
  //Second
  temp1 = (byte)date_set[11] - 48;
  temp2 = (byte)date_set[12] - 48;
  second_set = temp1 * 10 + temp2;
  
  //Set Time/Date
  clock.setClockMode(false);  // set to 24h
  clock.setYear(year_set);
  clock.setMonth(month_set);
  clock.setDate(day_set);
  clock.setDoW(dow_set);
  clock.setHour(hour_set);
  clock.setMinute(minute_set);
  clock.setSecond(second_set);
 }

void time_check(){
  //empty now
}
