#include <Arduino.h>

#include <iodriver.hpp>
#include <I2C.hpp>


bool calibrated = false;
long chargeTime = 0;
long chargingTime = 0;
//extern bool chg_preview;
///reference datasheet for bq29700 for meanings of these binary literals.

void setupCharge()
{
    ///read in values from BCD0 and BCD1 to get current capability
    if(( digitalRead( PIN_BCD0 ) == 1 ) && ( digitalRead( PIN_BCD1) == 0 ))
        setCurrent( 100 );      //unconfigured, draw 100mA
    else if(( digitalRead( PIN_BCD0 ) == 0 ) && ( digitalRead( PIN_BCD1 ) == 1 ))
        setCurrent( 500 );      //SDP, draw 500mA
    else if(( digitalRead( PIN_BCD0 ) == 1 ) && ( digitalRead( PIN_BCD1 ) == 1 ))
        setCurrent( 2000 );     //CDP/DCP, draw 1500mA
    else
        setCurrent( 100 );      //default to 100mA

    //setCurrent( 1000 ); //set 1A for testing
}

void setCurrent( int chg_typ )         //decide current capability of charger, and send to charger
{
    switch ( chg_typ )
    {

    case 2000:
        configCharger( 0b01010100, 0b11110010 );   //sets input current to 2000 and enables charging
        break;

    case 1500:
        configCharger( 0b01000100, 0b10010010 );   //sets input current to 1500 and enables charging
        break;

    case 1000:
        configCharger( 0b00110100, 0b01010010 );   //sets input current to 1000 and enables charging
        break;

    case 500:
        configCharger( 0b00100100, 0b00000010 );   //sets current to 500 and enables charging
        break;

    default:
        configCharger( 0b00000100, 0b00000010 );   //sets input current to 100 and enables charging
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
    Wire.requestFrom( address, bytes );

    for ( int r = 0; r < bytes; r++ )
    {
        char regBuf = Wire.read();
        Serial.print( r );
        Serial.print("\t0b");
        Serial.println( regBuf, BIN );
    }
}

void buttonPressedCharge()
{
    chargeTime = 0;
    chargingTime = 0;
    chg_preview = false;
}
