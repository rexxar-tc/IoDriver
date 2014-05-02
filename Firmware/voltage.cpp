#include <Arduino.h>

#include <sabre.hpp>

long calOffset = 1125300; //default value

void blinkRed();
void calReference();
int getSOC();

long checkVoltage( bool withOffset ){
  /*if (voltageLow){
     return voltageLow;
   }  */
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;


  return ( withOffset ? calOffset / result : result );
}

void blinkRed(){
    analogWrite(PIN_LED_R, 0);
    analogWrite(PIN_LED_G, 0);
    analogWrite(PIN_LED_B, 0);
    analogWrite(PIN_LED_W, 0);  //turn off all LEDs
    analogWrite(PIN_LED_R, 50); //turn on red LED
    delay(1000);                //wait one second
    analogWrite(PIN_LED_R, 0);  //turn it back off
    delay(2000);                //wait two seconds
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

    In practice, this is very inaccurate, but good enough
    for some basic indication of charge level.
    */
}
