#include <cstdlib>

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
//#include <EEPROM.h> // Don't remove this (commented) include. Needed for pre-build step parsing.
#include <Wire.h>
#include <SPI.h>
#include <SPI_io.hpp>
#include <iodriver.hpp>
#include <profile.hpp>
#include <profilehandler.hpp>
#include <EEPROMHandler.hpp>
#include <I2C.hpp>
#include <voltage.hpp>

bool plugged = false;
bool check_button = false;
unsigned long button_time = 0;

IoDriver::ProfileHandler ph;

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void updateLED() {
    IoDriver::rgbw color;
    IoDriver::Profile* ap = ph.active();
    if ( ap ) {
        color = ap->get_color_value();
    } else {
        color = IoDriver::rgbw( 0, 0, 0, 0 );
    }
    analogWrite( PIN_LED_R, color.r );
    analogWrite( PIN_LED_G, color.g );
    analogWrite( PIN_LED_B, color.b );
    analogWrite( PIN_LED_W, color.w );
}

void setup() {
    //Serial.begin( 4800 ); //enable here for debug, otherwise when plugged in

    set_sleep_mode( SLEEP_MODE_PWR_DOWN );

    // Find all profiles' start addresses and store them,
    // then activate the first profile.
    (void)ph.enum_profiles();
    (void)ph.first_profile();

    //set pin modes
    pinMode( PIN_FORMAT, INPUT_PULLUP );
    pinMode( PIN_BUTTON, INPUT_PULLUP );
    pinMode( PIN_POWERGOOD, INPUT_PULLUP );
    pinMode( PIN_BCD0, INPUT_PULLUP );
    pinMode( PIN_BCD1, INPUT_PULLUP );
    pinMode( PIN_LED_R, OUTPUT );
    pinMode( PIN_LED_G, OUTPUT );
    pinMode( PIN_LED_B, OUTPUT );
    pinMode( PIN_LED_W, OUTPUT );
    pinMode( PIN_CS_E, OUTPUT );

    attachInterrupt( 0, buttonPressed, FALLING ); //go to buttonPressed() when button on pin 2 goes from high to low

    Wire.begin();
    SPI.begin();


    if( digitalRead( PIN_FORMAT ) == LOW )      //format all EEPROM when PCB contacts are bridged on power up
    {
        analogWrite( PIN_LED_R, 128 );
        analogWrite( PIN_LED_G, 0 );
        analogWrite( PIN_LED_B, 0 );
        analogWrite( PIN_LED_W, 0 );
        for( int i = 0; i < EEPROM1_SIZE; i++ )
        {
            EEPROM_H.write( i, 0 );
        }
        erase_EE2();
        analogWrite( PIN_LED_R, 0 );
    }
}

void loop()
{
    checkPG();
    if ( !plugged )
    {
        //check battery voltage
        /*
        if (checkVoltage() < BATTERY_CRITICAL )
        {
            sleep_enable(); //turn off the saber to protect the battery
            sleep_mode();
        }
        else if (checkVoltage() < BATTERY_MINIMUM )
        {
            blinkRed();
        }

        else
        {
        */
            if ( check_button ) {
                // If button is up, stop checking, increment profile, and re-enable button interrupts
                if ( digitalRead( PIN_BUTTON ) != 0 ) {
                    check_button = false;
                    ph.next_profile();

                // If button is still down, check whether it has
                // been down for 2 seconds
                } else {
                    unsigned long time_now = millis();
                    if ( time_now - button_time >= TIME_TO_SLEEP ) {
                        // Turn off the LEDs
                        digitalWrite( PIN_LED_R, LOW );
                        digitalWrite( PIN_LED_G, LOW );
                        digitalWrite( PIN_LED_B, LOW );
                        digitalWrite( PIN_LED_W, LOW );
                        // Re-attach button interrupt
                        check_button = false;
                        // Go to sleep
                        disableCharge();
                        sleep_enable();
                        sleep_mode();
                    }
                }
            }
        updateLED();
        //checkSerial(); ///debug line
        //}
    }
    else if ( plugged )
    {
        checkSerial();
        chargeState();
    }
}


void buttonPressed()
{
    if ( check_button )
        return;

    sleep_disable();
    check_button = true;
    button_time  = millis();
}

void checkPG()
{
    if ( digitalRead (PIN_POWERGOOD) == LOW && !plugged )       //if charge IC reports plugged in, and we weren't previously
    {
        plugged = true;
        Serial.begin(BAUDRATE);
        setupCharge();
        detachInterrupt(0);                                     //detach button interrupt
        attachInterrupt(0, buttonPressedCharge, FALLING );      //attach new interrupt to check battery state while charging
        //Serial.println("DEBUG LINE");
    }

    else if ( digitalRead (PIN_POWERGOOD) == LOW && plugged)    //if charge IC reports plugged in, and we were previously
    {
        //do nothing
    }

    else if ( digitalRead (PIN_POWERGOOD) == HIGH && plugged)   //if charge IC reports unplugged
    {
        plugged = false;
        Serial.end();
        disableCharge();
        detachInterrupt(0);
        attachInterrupt( 0, buttonPressed, FALLING );           //and re-enable the interrupt
    }

    else
    {
        plugged = false;        //just in case.
    }
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
