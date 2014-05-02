//#include <stlport.h>
#include <string>
#include <vector>

#include <Arduino.h>
#include <avr/sleep.h>
#include <EEPROM.h>
#include <Wire.h>

#include <sabre.hpp>
#include <profile.hpp>

bool check_button = false;
unsigned long button_time = 0;
bool plugged = false;

/*
#######################################
#######################################

This code is ONLY for Rev.C hardware.

It will NOT work AT ALL on Rev.A!

########################################
########################################
*/

//bool voltageLow = false; //I think this is from an older version?

int getProfiles();
void freeProfiles();
Sabre::Profile* seekProfile( unsigned int );
void buttonPressed();
long checkVoltage( bool );
void blinkRed();
void checkSerial();
void checkPG();
void setupCharge();
void chargeState();
void serialRead( std::string );
void serialWrite( std::string );
unsigned int parse_int( std::string );
void chargeStandalone ();
void enableOscillator();
void disableOscillator();


std::vector<Sabre::Profile*> profiles;
std::vector<Sabre::Profile*>::iterator activeProfile;

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void activate_profile( std::vector<Sabre::Profile*>::iterator setprofile ) {
    activeProfile = setprofile;
    (*activeProfile)->first_gradient();
}

void next_profile() {
    if ( activeProfile+1 < profiles.end() ) {
        activate_profile( activeProfile+1 );
    } else {
        activate_profile( profiles.begin() );
    }
}



void updateLED() {
    Sabre::rgbw grad = (*activeProfile)->get_gradient_value();
    analogWrite( PIN_LED_R, grad.r );
    analogWrite( PIN_LED_G, grad.g );
    analogWrite( PIN_LED_B, grad.b );
}

void setup() {
    //Serial.begin(4800); //enable here for debug, otherwise when plugged in

    set_sleep_mode( SLEEP_MODE_PWR_DOWN );

    // Find all the profiles' start addresses and store them.
    // Needs to always be called on setup, or else profiles have no
    // initialized value, not even -1 which indicates an empty profile.
    if ( getProfiles() ) {
        // getProfiles() returns error code 1 if no profiles were loaded
        // do something about that here
    } else {
        // Set first profile active
        activate_profile( profiles.begin() );
    }

    for (int i = 2; i <= 13; ++i){ //set pullups on all digital pins
        pinMode(i, INPUT_PULLUP);  //except 0 and 1, because that would bork the serial port
    }
    pinMode( PIN_LED_R, OUTPUT);
    pinMode( PIN_LED_G, OUTPUT);
    pinMode( PIN_LED_B, OUTPUT);
    pinMode( PIN_LED_W, OUTPUT);
    digitalWrite ( PIN_LED_W, LOW); //remove this line when white is implimented in the GUI
    attachInterrupt(4, buttonPressed, FALLING); //go to buttonPressed() when button on pin 7 goes from high to low
    Wire.begin();

    //digitalWrite( 12, LOW);
    //disabled this line for now, just in case it breaks things, will re-enable after hardware tests.
}

void loop(){

    if (checkVoltage( true ) < BATTERY_CRITICAL && !plugged)
    {
        sleep_enable(); //turn off the saber to protect the battery
        sleep_mode();
    }
    else if (checkVoltage( true ) < BATTERY_MINIMUM && !plugged)
    {
        blinkRed();
    }

    checkPG();
    if ( !plugged)
    {
        if ( check_button ) {
            // If button is up, stop checking, increment profile, and re-enable button interrupts
            if ( digitalRead( PIN_BUTTON ) != 0 ) {
                check_button = false;
                next_profile();
                //attachInterrupt(1, buttonPressed, FALLING);

            // If button is still down, check whether it has
            // been down for 2 seconds
            } else {
                unsigned long time_now = millis();
                if ( time_now - button_time >= 2000 ) {
                    // Turn off the LEDs
                    digitalWrite( PIN_LED_R, LOW );
                    digitalWrite( PIN_LED_G, LOW );
                    digitalWrite( PIN_LED_B, LOW );
                    digitalWrite( PIN_LED_W, LOW );
                    // Re-attach button interrupt
                    check_button = false;
                    //attachInterrupt(1, buttonPressed, FALLING);
                    // Go to sleep
                    sleep_enable();
                    sleep_mode();
                }
            }
        }
        ///Serial.println( freeRam() );
        //checkSerial();

        // Update LED components
        updateLED();
    }
    if (plugged)
    {
        checkSerial();
        chargeState();
    }
}

int getProfiles(){
  // Clear any extant profiles before loading new ones
  freeProfiles();

  signed int p_address[PROFILE_COUNT];

  // Iterate through each profile address
  for (int i = 0; i < PROFILE_COUNT; ++i){
    signed int pa = i * PROFILE_SIZE;
    uint8_t pindex = EEPROM.read( pa );

    // Store profile address if good
    if ( pindex >= 1 && pindex <= PROFILE_COUNT ) {
      p_address[i] = i * PROFILE_SIZE;

    // Store an obviously bogus profile address for
    // any obviously bogus profile indices
    } else { // pindex < 1 || pindex > PROFILE_COUNT
      p_address[i] = -1;
    }
  }

    // Iterate through profiles in their selection order
    for ( int i = 0; i < PROFILE_COUNT; ++i ) {
        // If the profile is valid, initialize it
        // and add to profile container
        if ( -1 != p_address[i] ) {
            Sabre::Profile* prof = new Sabre::Profile;
            prof->init( p_address[i] );
            profiles.push_back( prof );
        }
    }

    // Let us know if no profiles were loaded
    return ( 0 == profiles.size() ? 1 : 0 );
}

void freeProfiles() {
    // Iterate through stored profiles and free their memory
    for ( std::vector<Sabre::Profile*>::iterator iter = profiles.begin(); iter < profiles.end(); ++iter ) {
        delete *iter;
    }
    profiles.clear();
}

Sabre::Profile* seekProfile( unsigned int index ) {
    // Iterate through stored profiles to find the specified index
    for ( std::vector<Sabre::Profile*>::iterator iter = profiles.begin(); iter < profiles.end(); ++iter ) {
        if ( (*iter)->get_index() == index ) {
            return *iter;
        }
    }

    // If not found, return null pointer
    return NULL;
}

void buttonPressed()
{
    if ( check_button )
        return;

    //Serial.println( "buttonPressed()" );

    sleep_disable();
    check_button = true;
    button_time  = millis();

//    detachInterrupt( 1 );
}

void checkPG(){
    if ( digitalRead (PIN_POWERGOOD) == LOW && !plugged )       //if charge IC reports plugged in, and we weren't previously
    {
        plugged = true;
        Serial.begin(4600);
        setupCharge();
        enableOscillator();
    }

    else if ( digitalRead (PIN_POWERGOOD) == LOW && plugged)    //if charge IC reports plugged in, and we were previously
    {
        //do nothing?
    }

    else if ( digitalRead (PIN_POWERGOOD) == HIGH && plugged)   //if charge IC reports unplugged
    {
        plugged = false;
        Serial.end();
        chargeStandalone();
        disableOscillator();
    }

    else
    {
        plugged = false;
        //just in case.
    }
}

//dark magicks ahead

void disableOscillator()
{
   UDINT = UDINT & 0b01101111;
   USBCON = USBCON | 0b00100000;
   PLLCSR = PLLCSR & 0b00010001;
   CLKSEL0 = CLKSEL0 | 0b00001000;
   while ( (CLKSTA & 0b00000010) !=1 );
   CLKSEL0 = CLKSEL0 & 0b11111000;
   digitalWrite( 12, LOW ); //turns off external oscillator
}

void enableOscillator()
{
    digitalWrite( 12, HIGH ); //turns on external oscillator
    UDINT = UDINT & 0b01101111;
    CLKSEL0 = CLKSEL0 | 0b00000100;
    while ( (CLKSTA & 0b00000001) != 1);
    CLKSEL0 = CLKSEL0 | 0b00000001;
    PLLCSR = PLLCSR | 0b00000010;
    CLKSEL0 = CLKSEL0 & 0b11110100;
    while ( (PLLCSR & 1) != 1);
    USBCON = USBCON & 0b110010001;
}

