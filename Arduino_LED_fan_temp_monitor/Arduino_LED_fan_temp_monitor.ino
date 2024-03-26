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
#define FANMAXTEMP 35

#include <elapsedMillis.h>
float thermistor_temp[2];
const uint8_t FAN_PIN[2] = {10, 11};
elapsedMillis serial_timer;

void setup(void) {
  Serial.begin(9600);
  DDRD = B11110000;
  PORTD = B01100000;
}

void loop(void) {
  uint8_t i;
  float ADC_temp[2];
  float fan_speed[2];
  uint8_t fan_pwm;

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
    if(fan_speed[i] <= 0) fan_pwm = 0;
    else if(fan_speed[i] >= 255) fan_pwm = 255;
    else fan_pwm = round(fan_speed[i]);
    if(serial_timer > 1000){
      Serial.print("LED#");
      Serial.print(i);
      Serial.print(": ");
      Serial.print("Temperature "); 
      Serial.print(ADC_temp[i]);
      Serial.print(" °C, ");
      Serial.print("Fan PWM "); 
      Serial.print(fan_pwm);
      Serial.print(". ");
      analogWrite(FAN_PIN[i], fan_pwm);
    }
  }
  if(serial_timer > 1000){
    Serial.println();
    serial_timer = 0;
  }
}
