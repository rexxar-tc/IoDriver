#include <Arduino.h>
#include <wire.h>
#include <sabre.hpp>

void setupCharge();
void setCurrent( char );
void chargeState();
void chargeStandalone();

void setupCharge()
{
    delay( 100 );                       //delay to allow detection to complete
    Wire.beginTransmission( 0x59 );     //connect to charge type detector at 0x59
    Wire.write( 0 );                    //write 0 to tell device we want to read
    Wire.endTransmission( 0x59 );       //disconnect I2C
    Wire.requestFrom( 2, 1 );           //read register 0x02; charger type
    setCurrent( Wire.read() );          //and pass it to setCurrent

    //set a few things on the charge detector
    Wire.beginTransmission( 0x58 );     //connect to charge detector
    Wire.write( 1 );                    //select register 1
    Wire.write( 0b00101000 );           //enable low-power mode and close USB switches
    Wire.endTransmission( 0x58 );       //disconnect
}

void setCurrent( char chg_typ )         //decide current capability of charger, and send to charger
{
    char I2Cbuf = 0;
    char I2Cbuf1 = 0;

    if ( chg_typ >> 5 == 0b010 )
    {
        I2Cbuf = 0b00100000;            //sets input current to 500 and enables charging
        I2Cbuf1 = 0;                    //set charge current to 500
    }

    if ( chg_typ >> 5 == 0b011 )
    {
        I2Cbuf = 0b01010000;            //sets input current to 2000 and enables charging
        I2Cbuf1 = 0b11110000;           //set charge current to 2000
    }

    if ( chg_typ >> 5 == 0b100 )
    {
        I2Cbuf = 0b00100000;            //sets input current to 500 and enables charging
        I2Cbuf1 = 0;                    //set charge current to 500
    }

    if ( chg_typ >> 5 == 0b101 )
    {
        I2Cbuf = 0b00110000;            //sets input current to 1000 and enables charging
        I2Cbuf1 = 0b01010000;           //set charge current to 1000
    }

    if ( chg_typ >> 5 == 0b001 )
    {
        I2Cbuf = 0b00000000;            //sets input current to 100 and enables charging
        I2Cbuf1 = 0;                    //500 is the lowest setting, so set that here
    }                                   //charge current *should* be limited to 100 by the input limit

    else
    {
        Serial.println( "Charge detect error");
        Serial.println( chg_typ );
        I2Cbuf = I2Cbuf | 0b00000010;   //sets input current to 0 and disables charging
    }

    //set the other 4 bits in I2Cbuf

    /*
    I2Cbuf bit 7, 0 //reset bit, leave this at 0
    I2Cbuf bit 3, 0 //disable STAT pin
    I2Cbuf bit 2, 1 //enable charge termination alert
    I2Cbuf bit 0, 0 //do not put charger in standby
    */

    I2Cbuf = I2Cbuf | 0b00000100;

    Wire.beginTransmission( 0x6A );     //address charge controller
    Wire.write( 1 );                    //select register 1
    Wire.write( 0 );                    //disable watchdog timer
    Wire.write( I2Cbuf );               //send input limit config to register 2
    Wire.write( 0b10001111 );           //send defaults for register 3
    Wire.write( I2Cbuf1 );              //send charge limit config to register 4
    Wire.write( 0b00000010 );           //send defaults for register 5
    Wire.write( 0b10101000 );           //enable SYSOFF in register 6 (bit 4)
    Wire.endTransmission( 0x6A );       //close connection
}

void chargeState()
{
    byte chgStat = 0;

    Wire.beginTransmission( 0x6B );     //address charger
    Wire.write( 0 );                    //write 0 to set read
    Wire.endTransmission( 0x6B );       //close connection
    Wire.requestFrom( 1, 1 );           //request the byte in register 1
    chgStat = Wire.read();              //read the byte into chgStat

    if ( chgStat >> 4 == 0 )
        //charger ready. should not happen
        Serial.println( "Charger ready" );

    if ( chgStat >> 4 == 0b0001 )
    {
        //charging
        analogWrite ( PIN_LED_B, 0 );
        analogWrite ( PIN_LED_W, 0 );
        analogWrite ( PIN_LED_R, 20 );
        analogWrite ( PIN_LED_G, 20 );
    }

    if ( chgStat >> 4 == 0b0010 )
    {
        //charging done
        analogWrite ( PIN_LED_B, 0 );
        analogWrite ( PIN_LED_W, 0 );
        analogWrite ( PIN_LED_R, 0 );
        analogWrite ( PIN_LED_G, 20 );
    }

    if ( chgStat >> 4 == 0b0011 )
    {
        //charge fault
        Serial.println ( "CHARGE FAULT" );
        Serial.println ( chgStat );
        //do something about that maybe
        analogWrite ( PIN_LED_B, 0 );
        analogWrite ( PIN_LED_W, 0 );
        analogWrite ( PIN_LED_R, 20 );
        analogWrite ( PIN_LED_G, 0 );
    }

    else
    {
        Serial.println ( "Unknown state in chgStat" );
        Serial.println ( chgStat );
        analogWrite ( PIN_LED_B, 0 );
        analogWrite ( PIN_LED_W, 0 );
        analogWrite ( PIN_LED_R, 20 );
        analogWrite ( PIN_LED_G, 0 );
    }
}

void chargeStandalone()
{
    Wire.beginTransmission( 0x6A );     //address charge controller
    Wire.write( 1 );                    //select register 1
    Wire.write( 0b01000000 );           //enable watchdog timer
    Wire.endTransmission( 0x6A );       //disconnect.
}
