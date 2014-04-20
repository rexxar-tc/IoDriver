#include <profile.hpp>

#include <EEPROM.h>

#include <sabre.hpp>

#include <Arduino.h>

namespace Sabre {

Profile::Profile()
:   index( 0 )
{

}

Profile::Profile( const Profile& rval ) {

}

Profile::~Profile() {

}

uint8_t Profile::get_index() {
    return index;
}

std::string Profile::get_name() {
    return pname;
}

const std::vector<Sabre::ColorData>& Profile::get_colors() {
    return colors;
}

int Profile::init( signed int address ) {
    bool end_profile = 0;
    pname = "";

    // Get index
    index = EEPROM.read( address + PROFILE_INDEX_OFFSET );

    // Get profile name
    char letter = 0;
    for ( int i = 0; i < PROFILE_NAME_MAX_CHARS; ++i ) {
        // Read character and exit loop if null
        letter = EEPROM.read( address + PROFILE_NAME_OFFSET + i );
        if ( 0 == letter ) break;
        // Otherwise append character to name string
        pname += letter;
    }

    // Get profile colors
    for ( int i = PROFILE_FIRST_COLOR_OFFSET; i < PROFILE_SIZE; i+=PROFILE_COLOR_UNIT_SIZE ) {
        // Read color data and add to color container
        Sabre::ColorData col;
        col.read_color( address + i );
        colors.push_back( col );

        // Stop reading colors if last color flag is set
        if ( col.is_last() ) break;
    }

    // Initialize color gradients
    for ( std::vector<ColorData>::iterator iter = colors.begin(); iter < colors.end(); ++iter )
    {
        std::vector<ColorData>::iterator next_iter = iter + 1;
        if ( next_iter < colors.end() )
        {
            iter->generate_gradient( *next_iter );
        } else {
            iter->generate_gradient( *colors.begin() );
        }
    }

    activeGradient = colors.begin();

    return 0;
}

rgbw Profile::get_gradient_value() {
    int finished = activeGradient->update_gradient();
    const rgbw& grad = activeGradient->get_grad();

    // If at end of gradient, go to next main color
    if ( finished == 1 ) {
        next_gradient();
    }

    return grad;
}

void Profile::next_gradient() {
    if ( activeGradient+1 < colors.end() ) {
        set_gradient( activeGradient+1 );
    } else {
        set_gradient( colors.begin() );
    }
}

void Profile::set_gradient( const std::vector<ColorData>::iterator& setgradient ) {
    activeGradient = setgradient;
    activeGradient->reset_gradient();
}

void Profile::first_gradient() {
    set_gradient( colors.begin() );
}

}
