#include <Arduino.h>

#include <sabre.hpp>

//bool voltageLow = false;
long calOffset = 1125300;

void blinkRed();
void calReference();
int getSOC();

long checkVoltage( bool withOffset ){
  /*if (voltageLow){
     return voltageLow;
   }  */
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

 // result = calOffset / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  //return result; // Vcc in millivolts
  //don't return anything

  return ( withOffset ? calOffset / result : result );
}

void blinkRed(){
    analogWrite(PIN_LED_R, 0);
    analogWrite(PIN_LED_G, 0);
    analogWrite(PIN_LED_B, 0);
    analogWrite(PIN_LED_W, 0); //turn off all LEDs
    analogWrite(PIN_LED_R, 50); //turn on red LED
    delay(1000); //wait one second
    analogWrite(PIN_LED_R, 0); //turn it back off
    delay(2000); //wait two seconds
}
void calReference()
{
    calOffset = checkVoltage( false ) * 4200;
}

int getSOC(){
    return (((BATTERY_MINIMUM - checkVoltage(true))/(-1*.08))/100);
    //crude State of Charge estimation. Returns int 0-100.

    /*
    This is based on a linear voltage drop from 4.2V (100%)
    to BATTERY_MINIMUM (0%).

    In practice, this is horrendously inaccurate, but good enough
    for some basic indication of charge level.
    */
}
