#ifndef _IODRIVER_HPP
#define _IODRIVER_HPP

#define FIRMWARE_VERSION_NUMBER	        "0.5.2"

#ifndef HARDWARE_VERSION_NUMBER
#define HARDWARE_VERSION_NUMBER         "G"
#endif

#define BAUDRATE                        57600 //serial port speed
#define SERIAL_DELAY                    1    //ms
//Pin definitions

#define PIN_BUTTON                  	2
#define PIN_POWERGOOD               	3
#define PIN_LED_W                   	6
#define PIN_LED_B                   	9
#define PIN_LED_G                   	5
#define PIN_LED_R                   	10
#define PIN_BCD0                        A0
#define PIN_BCD1                        A2
#define PIN_BATTERY                     A7

//power settings

#define TIME_TO_SLEEP                   2000 // milliseconds

                                             // ADC units
#define BATTERY_FULL                    839  // 4.1V
#define BATTERY_MINIMUM             	655  // 3.2V
#define BATTERY_CRITICAL            	614  // 3V
#define ADC_CONVERSION                  4.883  //converts ADC units to mV

#define TIME_INTERVAL_CONVERSION_FACTOR	10	// Multiply time interval by this to get milliseconds

//declarations

void buttonPressed();
void checkSerial();
void checkPG();
void setup_EE2();
void blinkRed();
int checkVoltage();
void checkVoltDebug();
void buttonPressedCharge();
void wakePlugged();

extern bool chg_preview;

/**
 * Data storage parameters
 **/
// Data formatting specs
#define CAL_OFFSET                       1025
#define BAT_LOW
#define EEPROM1_FIRST_PROFILE_OFFSET        0
#define PROFILE_INDEX_OFFSET                0
#define PROFILE_INDEX_SIZE                  1
#define PROFILE_NAME_OFFSET                 1
#define PROFILE_FIRST_DATA_OFFSET          32
#define PROFILE_COLOR_UNIT_SIZE             8
#define PROFILE_DATA_UNIT_SIZE              8
//#define PROFILE_COLOR_RGBW_OFFSET           0
//#define PROFILE_COLOR_TIME_INTERVAL_OFFSET  4
//#define PROFILE_COLOR_TIME_INTERVAL_SIZE    2
//#define PROFILE_COLOR_END_FLAG_OFFSET       6
#define PROFILE_DATA_TYPE_OFFSET            0
#define PROFILE_COLOR_RGBW_OFFSET           1
#define PROFILE_COLOR_TIME_INTERVAL_OFFSET  5
#define PROFILE_COLOR_TIME_INTERVAL_SIZE    2
#define PROFILE_COLOR_EFFECT_FLAGS_OFFSET   7
#define PROFILE_DATA_TYPE_COLOR             1
// Data limitations
#define EEPROM1_MAX_PROFILES                5
#define PROFILE_MAX_SIZE                    200
#define PROFILE_NAME_MAX_CHARS              31
#define PROFILE_MAX_DATACOUNT               21
// Specifications
#define EEPROM1_SIZE                        1024 // Used in EEPROMHandler
#define EEPROM2_SIZE                        16384
#define EEPROM_TOTAL                        EEPROM1_SIZE + EEPROM2_SIZE

#define EEPROM_H_VIRTUAL_SIZE               PROFILE_MAX_SIZE // Used in EEPROMHandler
#define HAS_SECONDARY_EEPROM

#endif // _IODRIVER_HPP
