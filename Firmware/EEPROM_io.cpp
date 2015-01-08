#include <Arduino.h>

#include <iodriver.hpp>
#include <EEPROMHandler.hpp>

/**
 * Raw data IO
 **/

void read_data( int start_address, int end_address ) {
    signed int inc = 1;
    if ( end_address < start_address ) {
        inc = -1;
    }

    for ( int i = start_address; i != end_address+inc; i += inc ) {
        unsigned char data = EEPROM_H.read( i );
        Serial.print( data, DEC );
        Serial.print( ' ' );
    }
    Serial.println( "" );

}

void write_data( int start_address, const char* data, int n_bytes ) {
    // Iterate through bytes and write them in position
    for ( int i = 0; i < n_bytes; ++i ) {
        EEPROM_H.write( start_address+i, data[i] );
    }
}

/**
 * Utility functions
 **/

signed int find_address_of( int pindex ) {
    // Address of preview profile in virtual eeprom
    if ( pindex == -2 ) return EEPROM_H_VIRTUAL_START;

    // Do bounds checking
    if ( pindex < 1 || pindex > EEPROM1_MAX_PROFILES ) return -1;

    for ( int i = 0; i < EEPROM1_MAX_PROFILES; ++i ) {
        // Get address of a profile in this physical position
        signed int address = EEPROM1_FIRST_PROFILE_OFFSET
                           + i*PROFILE_MAX_SIZE;
        // Read the index of the profile
        unsigned char j = EEPROM_H.read( address + PROFILE_INDEX_OFFSET );
        // If this is the profile we're looking for, return its address
        if ( j == pindex ) return address;
    }

    return -1;
}

bool profile_exists( int index ) {
    signed int address = find_address_of( index );
    return ( -1 != address );
}

signed int find_free_address() {
    for ( int i = 0; i < EEPROM1_MAX_PROFILES; ++i ) {
        // Get address of a profile in this physical position
        signed int address = EEPROM1_FIRST_PROFILE_OFFSET
                           + i*PROFILE_MAX_SIZE;
        // Read the index of the profile
        unsigned char j = EEPROM_H.read( address + PROFILE_INDEX_OFFSET );
        // Return if the position is free
        if ( j < 1 || j > EEPROM1_MAX_PROFILES ) return address;
    }

    return -1;
}

/**
 * High-level write methods
 **/

bool create_profile( int index ) {
    // Do bounds checking
    if ( index < 1 || index > EEPROM1_MAX_PROFILES ) return 0;

    // Make sure the profile with this index doesn't already exist
    signed int address = find_address_of( index );
    if ( -1 != address ) return 0;

    // Try to find an address without a profile in it
    address = find_free_address();
    if ( -1 != address ) {
        // Write the desired index in that position and truncate name
        EEPROM_H.write( address+PROFILE_INDEX_OFFSET, index );
        EEPROM_H.write( address+PROFILE_NAME_OFFSET,  '\0'  );
        return 1;
    }
    return 0;
}

void write_name( int index, char* name ) {
    // Find profile address and abort if not found
    signed int address = find_address_of( index );
    if ( -1 == address ) return;

    // Get address of name
    address = address
            + PROFILE_NAME_OFFSET;

    // If the profile was not found, abort
    if ( -1 == address ) return;

    // Write name characters until null byte or end of space
    for ( int i = 0; i < PROFILE_NAME_MAX_CHARS; ++i ) {
        EEPROM_H.write( address+i, name[i] );
        if ( '\0' == name[i] ) return;
    }
}

void write_color( int pindex, unsigned int cindex, signed int r, signed int g, signed int b, signed int w, signed int t, signed int effects, signed int last_color ) {
    // Find profile address and abort if not found
    signed int address = find_address_of( pindex );
    if ( -1 == address ) return;

    // Get address of color
    address = address
            + PROFILE_FIRST_DATA_OFFSET
            + ((cindex-1)*PROFILE_DATA_UNIT_SIZE );

    // Abort if color index is bad
    if ( cindex < 1 || cindex > PROFILE_MAX_DATACOUNT ) return;

    // Mark data as being a color
    EEPROM_H.write( address+PROFILE_DATA_TYPE_OFFSET, PROFILE_DATA_TYPE_COLOR );

    // Write color properties as given
    if ( r != -1 ) {
        EEPROM_H.write( address+PROFILE_COLOR_RGBW_OFFSET+0, r );
    }
    if ( g != -1 ) {
        EEPROM_H.write( address+PROFILE_COLOR_RGBW_OFFSET+1, g );
    }
    if ( b != -1 ) {
        EEPROM_H.write( address+PROFILE_COLOR_RGBW_OFFSET+2, b );
    }
    if ( w != -1 ) {
        EEPROM_H.write( address+PROFILE_COLOR_RGBW_OFFSET+3, w );
    }
    if ( t != -1 ) {
        // Convert time value into two bytes and write separately
        unsigned int tconvert = t / TIME_INTERVAL_CONVERSION_FACTOR;
        if ( tconvert > 65000 ) tconvert = 65000; // bounds checking for 16-bit int
        uint16_t tval = tconvert;
        char* byteptr = (char*)&tval;
        char lb = byteptr[0];
        char bb = byteptr[1];

        EEPROM_H.write( address+PROFILE_COLOR_TIME_INTERVAL_OFFSET+0, lb );
        EEPROM_H.write( address+PROFILE_COLOR_TIME_INTERVAL_OFFSET+1, bb );
    }
    if ( effects != -1 ) {
        EEPROM_H.write( address+PROFILE_COLOR_EFFECT_FLAGS_OFFSET, effects );
    }
    if ( last_color != -1 ) {
        // If this is NOT the end of a static-size profile,
        // ( accept only 1 as argument)
        if ( last_color == 1 && cindex < PROFILE_MAX_DATACOUNT ) {
            // Go to the next row and mark as dataless
            EEPROM_H.write( address+PROFILE_DATA_UNIT_SIZE+PROFILE_DATA_TYPE_OFFSET, 0 );
        }
    }
}

signed int delete_profile( int index ) {
    // Find the profile and delete it if found
    signed int address = find_address_of( index );

    if ( -1 != address ) {
        EEPROM_H.write( address, '\0' );
    }

    return address;
}

void reorder_profiles( int indices[][2], int n_indices ) {
    signed int addresses[EEPROM1_MAX_PROFILES];
    // Get addresses for all profiles on board
    // Has to be done before profiles get moved or we will have a puzzle
    for ( int i = 0; i < EEPROM1_MAX_PROFILES; ++i ) {
        addresses[i] = find_address_of( i+1 );
    }

    // Now iterate through profiles being moved
    for ( int i = 0; i < n_indices; ++i ) {
        // Get address and old/new indices of profile being moved
        int iold = indices[i][0];
        unsigned char inew = indices[i][1];
        signed int a = addresses[iold-1];
        // If the address is valid, move the profile
        if ( -1 != a ) {
            EEPROM_H.write( a, inew );
        }
    }
}

void mark_empty( int index ) {
    // Find the profile
    signed int address = find_address_of( index );
    if ( -1 == address ) return;

    address = address
            + PROFILE_FIRST_DATA_OFFSET
            + PROFILE_DATA_TYPE_OFFSET;

    EEPROM_H.write( address, 0 );
}

/**
 * High-level read methods
 **/

void print_profile( int index ) {
    // Find the address of the profile in question
    signed int address = find_address_of( index );
    // Print a 404 message if profile doesn't exist
    if ( -1 == address ) {
        Serial.print( F("Profile ") );
        Serial.print( index, DEC );
        Serial.println( F(" not found.") );
        return;
    }

    // Start xml output
    Serial.println( F("<profile>") );

    // Output profile name
    Serial.print( F("\t<name>") );
    char letter = 0;
    for ( int i = 0; i < PROFILE_NAME_MAX_CHARS; ++i ) {
        // Read character and exit loop if null
        letter = EEPROM_H.read( address + PROFILE_NAME_OFFSET + i );
        if ( 0 == letter ) break;
        // Otherwise append character to profile output
        Serial.print( letter );
    }
    Serial.println( F("</name>") );

    // Now get color data
    // Loop through 8-byte color data sequences
    for ( int i = PROFILE_FIRST_DATA_OFFSET; i < PROFILE_MAX_SIZE; i+=PROFILE_DATA_UNIT_SIZE ) {
        // Read individual bytes
        uint8_t dtype    = EEPROM_H.read( address+i+PROFILE_DATA_TYPE_OFFSET );
        if ( !dtype ) break;

        // Print the data type
        Serial.print( F("\t<data type=\'") );
        switch ( dtype ) {
            case PROFILE_DATA_TYPE_COLOR:
                Serial.print( F("color") );
                break;
            default:
                Serial.print( F("unknown") );
        }
        Serial.println( F("\'>") );

        // Get color data
        uint8_t rval     = EEPROM_H.read( address+i+PROFILE_COLOR_RGBW_OFFSET+0 );
        uint8_t gval     = EEPROM_H.read( address+i+PROFILE_COLOR_RGBW_OFFSET+1 );
        uint8_t bval     = EEPROM_H.read( address+i+PROFILE_COLOR_RGBW_OFFSET+2 );
        uint8_t wval     = EEPROM_H.read( address+i+PROFILE_COLOR_RGBW_OFFSET+3 );
        uint8_t lb       = EEPROM_H.read( address+i+PROFILE_COLOR_TIME_INTERVAL_OFFSET+0 );
        uint8_t bb       = EEPROM_H.read( address+i+PROFILE_COLOR_TIME_INTERVAL_OFFSET+1 );
        uint8_t effects  = EEPROM_H.read( address+i+PROFILE_COLOR_EFFECT_FLAGS_OFFSET );
        // Convert time value bytes to a 16-bit int
        uint16_t tval = 0;
        uint8_t* t = (uint8_t*)&tval;
        t[0] = lb;
        t[1] = bb;
        // Now convert to milliseconds
        unsigned long tms = tval * TIME_INTERVAL_CONVERSION_FACTOR;

        // Format and print the data contents
        Serial.println( F("\t\t<rgbw>") );
        Serial.print( F("\t\t\t<r>") );
        Serial.print( rval, DEC );
        Serial.println( F("</r>") );

        Serial.print( F("\t\t\t<g>") );
        Serial.print( gval, DEC );
        Serial.println( F("</g>") );

        Serial.print( F("\t\t\t<b>") );
        Serial.print( bval, DEC );
        Serial.println( F("</b>") );

        Serial.print( F("\t\t\t<w>") );
        Serial.print( wval, DEC );
        Serial.println( F("</w>") );
        Serial.println( F("\t\t</rgbw>") );

        Serial.print( F("\t\t<time>") );
        Serial.print( tms, DEC );
        Serial.println( F("</time>") );

        Serial.print( F("\t\t<effects>") );
        Serial.print( effects );
        Serial.println( F("</effects>") );
        Serial.println( F("\t</data>") );
    }
    Serial.println( F("</profile>") );
}
