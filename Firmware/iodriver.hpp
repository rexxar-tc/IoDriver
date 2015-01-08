#ifndef _IODRIVER_HPP
#define _IODRIVER_HPP

#define FIRMWARE_VERSION_NUMBER	        "0.5.0"

#ifndef HARDWARE_VERSION_NUMBER
#define HARDWARE_VERSION_NUMBER         "G"
#endif

//Pin definitions

#define PIN_BUTTON                  	2
#define PIN_POWERGOOD               	4
#define PIN_LED_R                   	6
#define PIN_LED_G                   	9
#define PIN_LED_B                   	5
#define PIN_LED_W                   	10
#define PIN_FORMAT                      A1
#define PIN_BCD0                        A0
#define PIN_BCD1                        A2

//power settings

#define TIME_TO_SLEEP                   2000 // milliseconds

#define BATTERY_FULL                    4100 // millivolts
#define BATTERY_MINIMUM             	3200
#define BATTERY_CRITICAL            	3000

#define TIME_INTERVAL_CONVERSION_FACTOR	10	// Multiply time interval by this to get milliseconds

/**
 * Data storage parameters
 **/
// Data formatting specs
#define CAL_OFFSET                       1025
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
