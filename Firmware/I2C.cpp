#include <Arduino.h>

#include <iodriver.hpp>
#include <I2C.hpp>


bool calibrated = false;
long chargeTime = 0;
long chargingTime = 0;

///reference datasheet for bq29700 for meanings of these binary literals.

/**
***
TODO: write shift register method
change bin current strings to something more useful
***
**/

void setupCharge()
{
    ///read in values from BCD0 and BCD1 to get current capability
    setCurrent( 0b01100000 ); //set 1A for testing
}

void setCurrent( char chg_typ )         //decide current capability of charger, and send to charger
{
    chg_typ = chg_typ >> 5;

    switch ( chg_typ )
    {
    case 0b010:
        configCharger( 0b01010100, 0b11110100 );    //sets current to 2000 and enables charging
        break;

    case 0b011:
        configCharger( 0b00110100, 0b01010100 );   //sets input current to 1000 and enables charging
        break;

    case 0b100:
        configCharger( 0b00100100, 0b00000100 );            //sets current to 500 and enables charging
        break;

    case 0b101:
        configCharger( 0b00110100, 0b01010100 );   //sets input current to 1000 and enables charging
        break;

    case 0b001:
        configCharger( 0b00100100, 0b00000100 );            //sets current to 500 and enables charging
        break;

    default:
        configCharger( 0b00000100, 0b00000100 );             //sets input current to 100 and enables charging
        break;
    }
}

void chargeState()
{
    byte chgStat = 0;

    Wire.beginTransmission( 0x6A );     //address charger
    Wire.write( 0 );                    //write 0 to set read
    Wire.endTransmission( 0x6A );       //close connection
    Wire.requestFrom( 0x6A, 1 );        //request the byte in register 1
    chgStat = Wire.read();              //read the byte into chgStat

    switch ( chgStat )
    {
        case 0:                         //charger ready. should not happen
            writeLED( 20, 0, 20, 0 );
            break;

        case 16:                        //charging
            if ( !chargingTimeout() )     //charging LED should only be on for five minutes, check that here
                writeLED( 20, 20, 0, 0 );
            else if ( chargingTimeout() )
                writeLED( 0, 0, 0, 0 );
            break;

        case 32:                        //charging done
            if ( !calibrated )          //if Vref not already calibrated, do it now
            {
                calReference( false );
                calibrated = true;
            }
            if ( !chargeTimeout() )     //charge done LED should only be on for five minutes, check that here
                writeLED( 0, 20, 0, 0 );
            else if ( chargeTimeout() )
                writeLED( 0, 0, 0, 0 );
            else
                writeLED( 0, 20, 0, 20 );
            break;

        default:
            //watchdog error or general charge fault
            writeLED( 0, 0, 20, 0 );
            break;
    }
}

void configCharger( int input, int current )
{
    delay( 100 );
    Wire.beginTransmission( 0x6A );     //configure charger
    Wire.write( 0 );                    //select register 1
    Wire.write( 0 );                    //disable watchdog
    Wire.write( input );                //set input current and TERM
    Wire.write( 0b01111011 );           //4.1V battery regulation
    Wire.write( current );              //set charge current
    Wire.write( 2 );                    //register 5 defaults
    Wire.write( 0b10100000 );           //6 hour charge timer
    Wire.write( 0b00001000 );           //6V OVP
    Wire.endTransmission( 0x6A );       //finished
    delay( 100 );
}

void writeLED ( int red, int green, int blue, int white )
{
    analogWrite( PIN_LED_R, red   );
    analogWrite( PIN_LED_G, green );
    analogWrite( PIN_LED_B, blue  );
    analogWrite( PIN_LED_W, white );
}

void disableCharge()
{
    Wire.beginTransmission( 0x6A );     //address charge controller
    Wire.write( 0 );                    //select register 1
    Wire.write( 0b01000000 );           //enable watchdog timer
    Wire.write( 0b00000101 );
    Wire.endTransmission( 0x6A );       //disconnect.
    chargeTime = 0;                     //reset timer variable
}

bool chargeTimeout()
{
    if ( chargeTime == 0 )
    {
        chargeTime = millis();
        return false;
    }

    else if ( ( millis() - chargeTime ) > 300000 )
        return true;                    //if charge has been done for five minutes, return true

    else                                //else return false
        return false;
}

bool chargingTimeout()
{
    if ( chargingTime == 0 )
    {
        chargingTime = millis();
        return false;
    }

    else if ( ( millis() - chargingTime ) > 300000 )
        return true;                    //if charge has been done for five minutes, return true

    else                                //else return false
        return false;
}

void getRegister( int address, int bytes )
{
    Wire.beginTransmission( address );
    Wire.write( 0 );
    Wire.endTransmission( address );
    char regBuf = 0;
    Wire.requestFrom( address, bytes );

    for ( int r = 0; r < bytes; r++ )
    {
        regBuf=0;
        regBuf = Wire.read();
        Serial.print( r );
        Serial.print(" ");
        for( int b = 7; b >= 0; b-- )
            Serial.print( bitRead( regBuf, b ));
        Serial.println(" ");
    }
}
