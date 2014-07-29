#include <profile.hpp>

#include <Arduino.h>

#include <iodriver.hpp>
#include <EEPROMHandler.hpp>

namespace IoDriver {

ColorData Profile::colors[PROFILE_MAX_DATACOUNT];

Profile::Profile()
:    iaddress( -1 )
{

}

void Profile::set_address( signed int address ) {
    iaddress = address;
}

int Profile::init() {
    // Get index
    signed int address = iaddress;

    // Get profile colors
    color_count = 0;
    bool last_color = 0;
    for ( int i = PROFILE_FIRST_DATA_OFFSET; i < PROFILE_MAX_SIZE; i+=PROFILE_DATA_UNIT_SIZE ) {
        // Check what kind of data this is
        uint8_t dtype = EEPROM_H.read( address+i );
        // Ref to color data
        switch ( dtype ) {

            case PROFILE_DATA_TYPE_COLOR:
                colors[color_count].read_color( address + i );
                ++color_count;
                break;
            // No data
            case 0:
            // Who knows
            default:
                last_color = 1;
        }

        if ( last_color ) break;
    }

    // Start up the first color
    first_color();

    return 0;
}

rgbw Profile::get_color_value() {
    if ( !color_count ) return rgbw( 0, 0, 0, 0 );

    // Call for a color update and get finished status
    bool finished = active_color->update();

    // Get the present color state for rendering
    rgbw color = active_color->get_immediate();

    // If at end of color effect, go to next color
    if ( finished ) next_color();

    return color;
}

void Profile::first_color() {
    cindex = 0;
    ColorData* ca = ( color_count ? &colors[cindex] : NULL );
    set_color( ca );
}

void Profile::next_color() {
    cindex = ( cindex+1 < color_count ? cindex+1 : 0 );
    ColorData* ca = ( color_count ? &colors[cindex] : NULL );
    set_color( ca );
}

ColorData* Profile::whatsnext() {
    if ( !color_count ) return NULL;

    return ( cindex+1 < color_count ? &colors[cindex+1] : &colors[0] );
}

void Profile::set_color( ColorData* color_to_set ) {

    // Go to the next color
    active_color = color_to_set;

    if ( color_to_set ) {
        // Initialize gradient if appropriate
        if ( active_color->effects & COLOR_EFFECT_GRADIENT ) {
            ColorData* next = whatsnext();
            active_color->init_gradient( *next );
        }

        // Start color effect
        active_color->start();
    }
}

}
