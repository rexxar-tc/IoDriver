#include <cstdlib>

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/power.h>
//#include <EEPROM.h> // Don't remove this include. Needed for pre-build step parsing.
#include <Wire.h>

#include <iodriver.hpp>
#include <profile.hpp>
#include <profilehandler.hpp>
#include <EEPROMHandler.hpp>
#include <I2C.hpp>

bool plugged = false;
bool check_button = false;
unsigned long button_time = 0;

void buttonPressed();
long checkVoltage( bool );
void checkSerial();
void checkPG();
void enableOscillator();
void disableOscillator();

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
    Serial.begin( 4800 ); //enable here for debug, otherwise when plugged in

    set_sleep_mode( SLEEP_MODE_PWR_DOWN );

    // Find all profiles' start addresses and store them,
    // then activate the first profile.
    (void)ph.enum_profiles();
    (void)ph.first_profile();

    for (int i = 2; i <= 13; ++i){  //set pullups on all digital pins
        pinMode(i, INPUT_PULLUP );  //except 0 and 1, because that would bork the serial port
    }
    pinMode( PIN_LED_R, OUTPUT );
    pinMode( PIN_LED_G, OUTPUT );
    pinMode( PIN_LED_B, OUTPUT );
    pinMode( PIN_LED_W, OUTPUT );
    pinMode( 7, INPUT );            //for prototype board, remove this later
    digitalWrite( PIN_Q_BUTTON, HIGH ); //switch in the button on pin 2
    pinMode( PIN_OSCILLATOR, OUTPUT );
    digitalWrite( PIN_OSCILLATOR, HIGH );
    attachInterrupt( 1, buttonPressed, FALLING ); //go to buttonPressed() when button on pin 2 goes from high to low
    Wire.begin();

    if( digitalRead( PIN_FORMAT ) == LOW )      //format all EEPROM when PCB contacts are bridged on power up
    {
        for( int i = 0; i < EEPROM_TOTAL; i++ )
        {
            EEPROM_H.write( i, 0 );
        }
    }
}

void loop(){
//Serial.println( freeRam() );
/*
    if (checkVoltage( true ) < BATTERY_CRITICAL && !plugged)
    {
        sleep_enable(); //turn off the saber to protect the battery
        sleep_mode();
    }
    else if (checkVoltage( true ) < BATTERY_MINIMUM && !plugged)
    {
        blinkRed();
    }
*/
    //checkPG();
    if ( !plugged )
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
                    // Re-attach button interrupt
                    check_button = false;
                    // Go to sleep
                    disableCharge();
                    disableOscillator();
                    sleep_enable();
                    sleep_mode();
                    enableOscillator();
                }
            }
        }
        checkSerial();

        updateLED();
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

void checkPG(){
    if ( digitalRead (PIN_POWERGOOD) == LOW && !plugged )       //if charge IC reports plugged in, and we weren't previously
    {
        plugged = true;
        Serial.begin(14400);
        setupCharge();
        detachInterrupt(1);                                     //detach button interrupt
        digitalWrite( PIN_Q_BUTTON, LOW );                      //switch out the button so it doesn't interfere with SDA line
    }

    else if ( digitalRead (PIN_POWERGOOD) == LOW && plugged)    //if charge IC reports plugged in, and we were previously
    {
        //do nothing?
    }

    else if ( digitalRead (PIN_POWERGOOD) == HIGH && plugged)   //if charge IC reports unplugged
    {
        plugged = false;
        Serial.end();
        disableCharge();
        digitalWrite( PIN_Q_BUTTON, HIGH );                     //switch back in the button
        attachInterrupt( 1, buttonPressed, FALLING );           //and re-enable the interrupt
    }

    else
    {
        plugged = false;        //just in case.
    }
}

//magic. do not touch.

void disableOscillator()
{
    bitClear( PLLCSR, 0 );              //PLLE = 0 disable PLL
    bitSet( CLKSEL0, 3 );               //RCE = 1 enable internal oscillator
    while( bitRead(CLKSTA, 1) != 1);    //wait for RCON internal clock ready
    bitClear( CLKSEL0, 0 );             //CLKS = 0 select internal clock
    bitSet( CLKSEL0, 2 );               //EXTE = 0 disable external clock
    digitalWrite(PIN_OSCILLATOR, LOW);  //turn off external oscillator
}

void enableOscillator()
{
    digitalWrite( PIN_OSCILLATOR, HIGH );//turn on external oscillator
    delay(100);                          //wait for oscillator to stabilize
    bitSet( CLKSEL0, 2 );                //EXTE = 1 ebable external clock
    while( bitRead( CLKSTA, 0 ) != 1);   //wait for EXTON external clock ready
    bitSet(CLKSEL0, 0 );                 //CLKS = 1 select external clock
    bitSet( PLLCSR, 1 );                 //PLLE = 1 enable PLL
    bitClear( CLKSEL0, 3 );              //RCE = 0 disable internal oscillator
    while( bitRead( PLLCSR, 0 ) != 1 );  //wait for PLOCK pll ready
}

