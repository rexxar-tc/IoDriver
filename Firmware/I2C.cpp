#include <Arduino.h>
#include <Wire.h>

#include <iodriver.hpp>
#include <I2C.hpp>

void writeLED( int, int, int, int );
void calReference();

bool calibrated = false;

///reference datasheets for bq29700 and MAX15478 for meanings of these binary literals.

void setupCharge()
{
    /*
    delay( 625 );                       //delay to allow detection to complete
    Wire.beginTransmission( 0x59 );     //connect to charge type detector at 0x59
    Wire.write( 0 );                    //write 0 to tell device we want to read
    Wire.endTransmission( 0x59 );       //disconnect I2C
    Wire.requestFrom( 0x59, 2 );        //read register 0x02; charger type
    setCurrent( Wire.read() );          //and pass it to setCurrent

    //set a few things on the charge detector
    Wire.beginTransmission( 0x58 );     //connect to charge detector
    Wire.write( 0 );                    //select register 1
    Wire.write( 0b00101000 );           //enable low-power mode and close USB switches
    Wire.endTransmission( 0x58 );       //disconnect
    */

    setCurrent( 0b01000000 );   //set 500 for testing
}

void setCurrent( char chg_typ )         //decide current capability of charger, and send to charger
{

    chg_typ = chg_typ >> 5;

    switch ( chg_typ )
    {
    case 0b010:
        configCharger( 0b100000, 0 );              //sets current to 500 and enables charging
        break;

    case 0b011:
        configCharger( 0b01010000, 0b11110000);    //sets current to 2000 and enables charging
        break;

    case 0b100:
        configCharger( 0b00100000, 0 );            //sets current to 500 and enables charging
        break;

    case 0b101:
        configCharger( 0b00110000, 0b01010000 );   //sets input current to 1000 and enables charging
        break;

    case 0b001:
        configCharger( 0, 0 );                     //sets current to 100 and enables charging
        break;

    default:
        configCharger( 0b00000010, 0);             //sets input current to 0 and disables charging
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
        case 0:
            //charger ready. should not happen
            writeLED( 20, 0, 20, 0 );
            break;

        case 16:
            //charging
            writeLED( 20, 20, 0, 0 );
            break;

        case 32:
            //charging done
            writeLED( 0, 20, 0, 0 );
            if (!calibrated)
            {
                calReference();
                calibrated = true;
            }
            break;

        default:
            //watchdog error or general charge fault
            writeLED( 0, 0, 20, 0 );
            break;
    }
}

void configCharger( int input, int current )
{
    Wire.beginTransmission( 0x6A );     //configure charger
    Wire.write( 0 );                    //select register 1
    Wire.write( 0 );                    //disable watchdog
    Wire.write( input );                //set input current and TERM
    Wire.write( 0b01111011 );           //4.1V battery regulation
    Wire.write( current );              //set charge current
    Wire.write( 2 );                    //register 5 defaults
    Wire.write( 0b00101000 );           //6 hour charge timer
    Wire.write( 0b00010000 );           //6V OVP
    Wire.endTransmission( 0x6A );       //finished
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
}
