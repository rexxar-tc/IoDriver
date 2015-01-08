#include <EEPROMHandler.hpp>

#include <SPI_io.hpp>

IoDriver::EEPROMHandler EEPROM_H;

namespace IoDriver {

uint8_t EEPROMHandler::read( int address ) {

#ifdef EEPROM_H_VIRTUAL_SIZE
    if ( address >= EEPROM_H_VIRTUAL_START && address <= EEPROM_H_VIRTUAL_END ) {
        int a = address_to_virtual_index( address );
        return virtual_eeprom[a];
    } else
#endif
    {
#ifdef HAS_SECONDARY_EEPROM
        if ( address >= EEPROM1_SIZE ) {
            int e2_address = address - EEPROM1_SIZE;
            read_EE2( e2_address );
        } else
#endif
        {
            return EEPROM.read( address );
        }
    }
}

void EEPROMHandler::write( int address, uint8_t value ) {
#ifdef EEPROM_H_VIRTUAL_SIZE
    if ( address >= EEPROM_H_VIRTUAL_START && address <= EEPROM_H_VIRTUAL_END ) {
        int a = address_to_virtual_index( address );
        virtual_eeprom[a] = value;
    } else
#endif
    {
#ifdef HAS_SECONDARY_EEPROM
        if ( address >= EEPROM1_SIZE ) {
            int e2_address = address - EEPROM1_SIZE;
            write_EE2( e2_address, value );
        } else
#endif
        {
            EEPROM.write( address, value );
        }
    }
}

#ifdef EEPROM_H_VIRTUAL_SIZE
int EEPROMHandler::address_to_virtual_index( int address ) {
    int adj_address = address - EEPROM_H_VIRTUAL_START;
    return adj_address;
}
#endif

}
