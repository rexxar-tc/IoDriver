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

bool plugged = false;
bool check_button = false;
bool bat_low = false;
unsigned long button_time = 0;
unsigned long average_time = 0;
int average_array[10] = {700,700,700,700,700,700,700,700,700,700};
int average_count = 0;
bool chg_preview = false;
bool sleep_tst = false;
bool is_asleep = false;

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
    pinMode( A1, INPUT );
    pinMode( PIN_BUTTON, INPUT_PULLUP );
    pinMode( PIN_POWERGOOD, INPUT_PULLUP );
    pinMode( PIN_BCD0, INPUT_PULLUP );
    pinMode( PIN_BCD1, INPUT_PULLUP );
    pinMode( PIN_LED_R, OUTPUT );
    pinMode( PIN_LED_G, OUTPUT );
    pinMode( PIN_LED_B, OUTPUT );
    pinMode( PIN_LED_W, OUTPUT );
    pinMode( PIN_CS_E, OUTPUT );
    pinMode( PIN_Vexp, OUTPUT );

    digitalWrite( PIN_Vexp, HIGH );     //turn on power to expansion bus

    attachInterrupt( 0, buttonPressed, FALLING );   //go to buttonPressed() when button on pin 2 goes from high to low
    attachInterrupt( 1, wakePlugged, FALLING );     //gp to wakePlugged() when device plugged into usb

    Wire.begin();
    SPI.begin();
}

void loop()
{
    checkPG();
    if ( !plugged )
    {
        //check battery voltage
        if (checkVoltage() < BATTERY_MINIMUM || bat_low == true )
        {
            bat_low = true;
            for( int b = 0; b < 5; ++b)
                blinkRed();

            disableCharge();
            digitalWrite( PIN_Vexp, LOW );   //turn off power to expansion bus
            sleep_enable();
            sleep_mode();
            digitalWrite( PIN_Vexp, HIGH );  //turn on power to expansion bus
        }

        else
        {
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
                        check_button = false;
                        is_asleep = true;
                        digitalWrite( PIN_Vexp, LOW );      //turn off power to expansion bus
                        PRR |= 0b10000111;                  //turn off ADC, I2C, and UART
                        sleep_enable();
                        sleep_mode();
                        PRR &= 0b01111000;                  //turn on ADC, I2C, and UART
                        digitalWrite( PIN_Vexp, HIGH );     //turn on power to expansion bus
                    }
                }
            }
        updateLED();
        }
    }
    else if ( plugged && !chg_preview )
    {
        checkSerial();
        chargeState();
    }
    else if ( plugged && chg_preview == true )
    {
        checkSerial();
        updateLED();
    }
}

void buttonPressed()
{
    sleep_disable();
    //if we've just resumed from sleep, set check_button false so we don't change profile
    if ( is_asleep )
    {
        is_asleep = false;
        check_button = false;
        return;
    }
    if ( check_button )
        return;

    check_button = true;
    button_time  = millis();
}

void wakePlugged()
{
    sleep_disable();    //wake up system from sleep on charger connect
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
        bat_low = false;                                        //reset bat_low
        is_asleep = false;
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
        chg_preview = false;                                    //cancel profile preview
        ph.first_profile();                                     //switch to first profile
    }

    else
    {
        plugged = false;        //just in case.
    }
}

void blinkRed()
{
    analogWrite(PIN_LED_R, 0);
    analogWrite(PIN_LED_G, 0);
    analogWrite(PIN_LED_B, 0);
    analogWrite(PIN_LED_W, 0);  //turn off all LEDs
    analogWrite(PIN_LED_R, 50); //turn on red LED
    delay(1000);                //wait one second
    analogWrite(PIN_LED_R, 0);  //turn it back off
    delay(2000);                //wait two seconds
    analogWrite(PIN_LED_R, 0);
    analogWrite(PIN_LED_G, 0);
    analogWrite(PIN_LED_B, 0);
    analogWrite(PIN_LED_W, 0);  //turn off all LEDs
    analogWrite(PIN_LED_R, 50); //turn on red LED
    delay(1000);                //wait one second
    analogWrite(PIN_LED_R, 0);  //turn it back off
    delay(2000);                //wait two seconds
}

int checkVoltage()
{
    long average_hold = 0;
    if( millis() - average_time >= 1000 )
    {
        average_array[average_count] = analogRead( PIN_BATTERY );
        average_time = millis();
        for( int a = 0; a < 10; ++a)
        {
            average_hold += average_array[a];
        }
        average_hold /= 10;

        if( average_count == 9 )
            average_count = 0;
        else
            average_count ++;

        return average_hold;
    }
    else
    {
        for( int a = 0; a < 10; ++a)
        {
            average_hold += average_array[a];
        }
        average_hold /= 10;
        return average_hold;
    }
}
