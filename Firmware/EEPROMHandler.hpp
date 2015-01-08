#ifndef _IODRIVER_EEPROMHANDLER
#define _IODRIVER_EEPROMHANDLER

#include <EEPROM.h>

#include <iodriver.hpp>

#ifndef EEPROM1_SIZE
#   error IoDriver::EEPROMHandler needs to know the size of the primary EEPROM!
#endif

#ifdef HAS_SECONDARY_EEPROM
#   ifndef EEPROM2_SIZE
#       error IoDriver::EEPROMHandler needs to know the size of the secondary EEPROM!
#   endif
#endif

// Note: VIRTUAL_START must be less than VIRTUAL_END.
//       Example sizes assume a virtual size of 200.
#ifdef EEPROM_H_VIRTUAL_SIZE
#   ifndef NEGATIVE_VIRTUAL_INDICES
#       ifndef HAS_SECONDARY_EEPROM
#           define EEPROM_H_VIRTUAL_START EEPROM1_SIZE // e.g. 1024
#       else
#           define EEPROM_H_VIRTUAL_START EEPROM1_SIZE+EEPROM2_SIZE // e.g. 2048
#       endif
#   else
#       define EEPROM_H_VIRTUAL_START -100-EEPROM_H_VIRTUAL_SIZE // e.g. -300
/* Note: address -1 is reserved for "404 not found" return values,
 * so we can't have a virtual array with indices, say, -200 to -1.
 * However, values such as -201 to -2 are inconvenient. So, we offset
 * by 100 instead of 1 to have something cleaner like -300 to -99.
 */
#   endif
#   define EEPROM_H_VIRTUAL_END EEPROM_H_VIRTUAL_START+EEPROM_H_VIRTUAL_SIZE-1 // e.g. 1223, 2247, -101
#endif

namespace IoDriver {

class EEPROMHandler {
    public:
        uint8_t read( int );
        void write( int, uint8_t );

    private:

#ifdef EEPROM_H_VIRTUAL_SIZE
        int address_to_virtual_index( int );
        uint8_t virtual_eeprom[EEPROM_H_VIRTUAL_SIZE];
#endif
};

}

extern IoDriver::EEPROMHandler EEPROM_H;

#endif // _IODRIVER_EEPROMHANDLER
