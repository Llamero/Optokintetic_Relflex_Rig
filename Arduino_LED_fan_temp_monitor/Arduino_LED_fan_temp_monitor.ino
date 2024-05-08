// SPDX-FileCopyrightText: 2011 Limor Fried/ladyada for Adafruit Industries
//
// SPDX-License-Identifier: MIT

// Thermistor Example #3 from the Adafruit Learning System guide on Thermistors 
// https://learn.adafruit.com/thermistor/overview by Limor Fried, Adafruit Industries
// MIT License - please keep attribution and consider buying parts from Adafruit
       
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 64
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3428
// the value of the 'other' resistor
#define SERIESRESISTOR 4700
// Temp (°C) for min fan speed
#define FANMINTEMP 25
// Temp (°C) for max fan speed
#define FANMAXTEMP 27
//Maximum allowed fanspeed (0-255).  Low = quieter + hotter.
#define MAXFANSPEED 200
//Temperature threshold for audible alarm
#define ALARMTEMP 50

#include <elapsedMillis.h>
float thermistor_temp[2];
const uint8_t FAN_PIN[3] = {10, 9, 11};
elapsedMillis serial_timer;
elapsedMicros alarm_timer;

void setup(void) {
  Serial.begin(9600);
  DDRD = B11111100;
  PORTD = B01100000;
  pinMode(12, OUTPUT);
}

void loop(void) {
  uint8_t i;
  float ADC_temp[2];
  float fan_speed[2];
  uint8_t fan_pwm;
  bool alarm = false;

  // take N samples in a row, with a slight delay
  i = NUMSAMPLES;
  analogRead(A0); //Clear ADC
  ADC_temp[0] = 0;
  while(i--) ADC_temp[0] += analogRead(A0);
  i = NUMSAMPLES;
  analogRead(A1); //Clear ADC
  ADC_temp[1] = 0;
  while(i--) ADC_temp[1] += analogRead(A1);
  
  // convert the value to resistance
  i=2;
  while(i--){
    ADC_temp[i] = 65535 / ADC_temp[i] - 1;
    ADC_temp[i] = SERIESRESISTOR / ADC_temp[i];   
    ADC_temp[i] = ADC_temp[i] / THERMISTORNOMINAL;     // (R/Ro)
    ADC_temp[i] = log(ADC_temp[i]);                  // ln(R/Ro)
    ADC_temp[i] /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
    ADC_temp[i] += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
    ADC_temp[i] = 1.0 / ADC_temp[i];                 // Invert
    ADC_temp[i] -= 273.15;                         // convert absolute temp to C
    
    fan_speed[i] = 255*((ADC_temp[i]-FANMINTEMP)/(FANMAXTEMP-FANMINTEMP));
    if(fan_speed[i] <= 0) fan_speed[i] = 0;
    else if(fan_speed[i] >= MAXFANSPEED) fan_speed[i] = MAXFANSPEED;
    fan_pwm = round(fan_speed[i]);
    if(serial_timer > 1000){
      Serial.print("LED#");
      Serial.print(i);
      Serial.print(": "); 
      Serial.print(ADC_temp[i]);
      Serial.print(" °C, ");
      Serial.print("Fan: "); 
      Serial.print(fan_pwm);
      Serial.print(". ");
      analogWrite(FAN_PIN[i], fan_pwm);
    }
  }
  //Set driver fan speed to match fastest LED fan speed
  if(serial_timer > 1000){
    if (fan_speed[0] > fan_speed[1]) fan_pwm = round(fan_speed[0]);
    else fan_pwm = round(fan_speed[1]);
    Serial.print("Driver fan: ");
    Serial.print(fan_pwm);
    Serial.print(". ");
    analogWrite(FAN_PIN[2], fan_pwm);
    Serial.println();
    serial_timer = 0;
    if (fan_pwm > 0) digitalWrite(12, HIGH); 
    else digitalWrite(12, LOW); 
  }
  if(ADC_temp[0] >= ALARMTEMP || ADC_temp[1] >= ALARMTEMP){
    uint32_t start = serial_timer;
    while(serial_timer - start < 500){
      if(alarm_timer < 166) PORTD = B01101000; 
      else if(alarm_timer < 333) PORTD = B01100100; 
      else alarm_timer = 0;
    }
  }
  else{
    PORTD = B01100000; 
  }
}
