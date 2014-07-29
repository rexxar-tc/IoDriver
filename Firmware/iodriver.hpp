#ifndef _IODRIVER_HPP
#define _IODRIVER_HPP

#define FIRMWARE_VERSION_NUMBER	        "0.3.1"

#ifndef HARDWARE_VERSION_NUMBER
#define HARDWARE_VERSION_NUMBER         "C"
#endif

//Pin definitions

#define PIN_BUTTON                  	2
#define PIN_Q_BUTTON                	A5
#define PIN_POWERGOOD               	4
#define PIN_LED_R                   	6
#define PIN_LED_G                   	9
#define PIN_LED_B                   	5
#define PIN_LED_W                   	10
#define PIN_OSCILLATOR              	12
#define PIN_FORMAT                      13
#define PIN_CS_E                        11
#define PIN_CS_BT                       7
#define PIN_CS_AUX                      A4
#define PIN_CS_AUX1                     A0
#define PIN_CS_AUX2                     A1
#define PIN_Vee                         8
#define PIN_Vexp                        A2

//power settings

#define TIME_TO_SLEEP                   2000 // milliseconds
#define BATTERY_FULL                    4100
#define BATTERY_MINIMUM             	3200
#define BATTERY_CRITICAL            	3000

#define TIME_INTERVAL_CONVERSION_FACTOR	10	// Multiply time interval by this to get milliseconds

/**
 * Data storage parameters
 **/
// Data formatting specs
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
#define EEPROM_TOTAL                        1024

#define EEPROM_H_VIRTUAL_SIZE               PROFILE_MAX_SIZE // Used in EEPROMHandler

#endif // _IODRIVER_HPP
