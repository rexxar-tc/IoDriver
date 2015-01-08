#include <Arduino.h>
#include <EEPROMHandler.hpp>
#include <iodriver.hpp>

uint32_t calFactor = 1125300; //default value
char calCount = 0;

void blinkRed();
uint32_t calReference( bool );
void initCalFactor();

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

    return ( withOffset ? calFactor / result : result );
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
uint32_t calReference( bool bufReq )
{
    long voltAvg = 0;
    uint32_t calBuf = 0;
    char bytes[4];
    char* wbytes;

    for( int b = 0; b <= 3; b++ )
    {
        bytes[b] = EEPROM_H.read( CAL_OFFSET + b );
    }

    for ( int r = 3; r >= 0; r-- )
    {
        calBuf = ( calBuf << 8 ) + bytes[r];
    }

    if( bufReq )
        return calBuf;

    if ( calBuf == 0 || calBuf == 4294967295 || calCount == 4 )  //if cal value in EEPROM is all 0 or all 1 OR we've called cal 5 times
    {                                                            //begin calibration routine
        for (int i = 0; i < 10; i++)                             //average 10 samples to cancel sample jitter
        {
            voltAvg += checkVoltage( false );
            delay(100);
        }

        calFactor = ( voltAvg / 10 ) * BATTERY_FULL;
        calCount = 0;                               //reset calCount to 0
        ///write calFactor into EEPROM
        wbytes = (char*)&calFactor;            //put the bytes of calFactor into an array
        /*
        for( int w = 0; w <= 3; w++ )
        {
            wbytes[w] = EEPROM_H.write( (CAL_OFFSET + w), wbytes[w] );
        }
        */

        EEPROM_H.write( CAL_OFFSET, wbytes[0] );
        EEPROM_H.write( CAL_OFFSET + 1, wbytes[1] );
        EEPROM_H.write( CAL_OFFSET + 2, wbytes[2] );
        EEPROM_H.write( CAL_OFFSET + 3, wbytes[3] );

    }

    else
    {
        calCount ++;
        calFactor = calBuf;
    }

}

void initCalFactor()
{
    uint32_t calBuf = 0;
    char bytes[4];

    for( int b = 0; b <= 3; b++ )
    {
        bytes[b] = EEPROM_H.read( CAL_OFFSET + b );
    }

    for ( int r = 3; r >= 0; r-- )
    {
        calBuf = ( calBuf << 8 ) + bytes[r];
    }
    if( calBuf == 0 || calBuf == 4294967295 )
    {
        //do nothing, leave calFactor at default value
    }

    else
        calFactor = calBuf;
}
