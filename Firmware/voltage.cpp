#include <Arduino.h>

#include <iodriver.hpp>

long calOffset = 1125300; //default value

void blinkRed();
void calReference();

long checkVoltage( bool withOffset )
{
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

void blinkRed()
{   analogWrite(PIN_LED_R, 0);
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
    long voltAvg = 0;

    for (int i = 0; i < 10; i++)        //average 10 samples to cancel sample jitter
    {
        voltAvg += checkVoltage( false );
        delay(100);
    }

    calOffset = ( voltAvg / 10 ) * BATTERY_FULL;
}
