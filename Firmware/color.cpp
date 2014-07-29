#include <color.hpp>

#include <Arduino.h>

#include <iodriver.hpp>
#include <EEPROMHandler.hpp>

namespace IoDriver {

ColorData::StateInfo ColorData::state;

// Constructor from integral component values
rgbw::rgbw( uint8_t valr, uint8_t valg, uint8_t valb, uint8_t valw )
:   r( valr ),
    g( valg ),
    b( valb ),
    w( valw )
{

}

// Assignment operator overload
rgbw& rgbw::operator=( const rgbw& rval )
{
    r = rval.r;
    g = rval.g;
    b = rval.b;
    w = rval.w;

    return *this;
}

void ColorData::start()
{
    // Get start time
    state.start_time = millis();
    // Startup gradient
    if ( effects & COLOR_EFFECT_GRADIENT ) {
        state.xval.reset();
    }

    // Get strobe start time and turn strobe state on
    if ( effects & COLOR_EFFECT_STROBE ) {
        state.strobe_last  = state.start_time;
        state.strobe_state = true;
    }
}

int ColorData::read_color( unsigned int address ) {
//    uint8_t dtype    = EEPROM_H.read( address+PROFILE_DATA_TYPE_OFFSET );
    uint8_t rval     = EEPROM_H.read( address+PROFILE_COLOR_RGBW_OFFSET+0          );
    uint8_t gval     = EEPROM_H.read( address+PROFILE_COLOR_RGBW_OFFSET+1          );
    uint8_t bval     = EEPROM_H.read( address+PROFILE_COLOR_RGBW_OFFSET+2          );
    uint8_t wval     = EEPROM_H.read( address+PROFILE_COLOR_RGBW_OFFSET+3          );
    uint8_t lb       = EEPROM_H.read( address+PROFILE_COLOR_TIME_INTERVAL_OFFSET+0 );
    uint8_t bb       = EEPROM_H.read( address+PROFILE_COLOR_TIME_INTERVAL_OFFSET+1 );
            effects  = EEPROM_H.read( address+PROFILE_COLOR_EFFECT_FLAGS_OFFSET    );

    uint8_t* t = (uint8_t*)&this->tval;
    t[0] = lb;
    t[1] = bb;

    // Construct color value object
    cval = rgbw( rval, gval, bval, wval );

    return 0;
}

void ColorData::init_gradient( const IoDriver::ColorData& next_color ) {
    int ms = tval * TIME_INTERVAL_CONVERSION_FACTOR;

    state.xval.regen( &cval, &next_color.cval, ms );
}

rgbw ColorData::get_immediate() const {
    // For strobe effects,
    if ( COLOR_EFFECT_STROBE & effects ) {
        // Return no color if strobe effect is in off state
        if ( !state.strobe_state ) {
            return rgbw( 0, 0, 0, 0 );
        }
    }

    // Return gradient value if gradient effect
    if ( COLOR_EFFECT_GRADIENT & effects ) {
        return state.xval;

    // Return plain color if no gradient effect
    } else {
        return cval;
    }
}

bool ColorData::timeup() const {
    return ( millis() >= ( state.start_time + tval * TIME_INTERVAL_CONVERSION_FACTOR ) );
}

bool ColorData::update() {
    // For strobe effect only
    if ( effects & COLOR_EFFECT_STROBE ) {
        // Check whether it's time to change the strobe state
        unsigned long time_now = millis();
        if ( time_now >= ( state.strobe_last + COLOR_STROBE_INTERVAL ) ) {
            // If so, flip the state and update the strobe time
            state.strobe_state = !state.strobe_state;
            state.strobe_last = time_now;
        }
    }

    bool finished = timeup();
    // For gradient effect only
    if ( effects & COLOR_EFFECT_GRADIENT ) {
        // Update gradient and get finished status
        bool grad_fin = state.xval.step();
        // Make sure its time is also up
        finished = finished && grad_fin;
    }
    return finished;
}

/**
 *
 * GRADIENT METHODS
 *
 **/

void Gradient::reset() {
    unsigned long ms_now = millis();
    for ( int i = 0; i < GRADIENT_COMPONENT_COUNT; ++i ) {
        *(c[i].vx) = c[i].v1;
        c[i].next_update = ms_now + c[i].freq_ms;
    }
}

void Gradient::regen( const rgbw* color_begin, const rgbw* color_end, unsigned long transition_time )
{
    color1 = color_begin;
    color2 = color_end;
    ttime_ms = transition_time;

    r = color1->r;
    g = color1->g;
    b = color1->b;
    w = color1->w;

    c[0].v1 = color1->r;
    c[0].v2 = color2->r;
    c[0].vx = &r;
    c[1].v1 = color1->g;
    c[1].v2 = color2->g;
    c[1].vx = &g;
    c[2].v1 = color1->b;
    c[2].v2 = color2->b;
    c[2].vx = &b;
    c[3].v1 = color1->w;
    c[3].v2 = color2->w;
    c[3].vx = &w;

    for ( int i = 0; i < GRADIENT_COMPONENT_COUNT; ++i ) {
        int diff = c[i].v2 - c[i].v1;
        if ( diff < 0 ) {
            c[i].inc = -1;
            diff = 0 - diff;
        } else {
            c[i].inc = 1;
        }
        if ( diff != 0 ) {
            c[i].freq_ms = ttime_ms / diff;
        } else {
            c[i].freq_ms = 0;
        }
    }
}

bool Gradient::step() {
    bool finished = 1;
    unsigned long ms_now = millis();

    for ( int i = 0; i < GRADIENT_COMPONENT_COUNT; ++i ) {
//        uint8_t v1 =   c[i].v1;  // Gradient start color
        uint8_t v2 =   c[i].v2;  // Gradient end color
        uint8_t vx = *(c[i].vx); // Current gradient color state

        // Check that the color hasn't finished transitioning
        if ( vx != v2 ) {
            // Check whether it is time to increment color
            if ( c[i].next_update < ms_now ) {
                *(c[i].vx) += c[i].inc;
                c[i].next_update = ms_now + c[i].freq_ms;

                // Check again whether the transition is finished
                if ( vx != v2 ) {
                    finished = 0;
                }
            } else {
                finished = 0;
            }
        }
    }

    return finished;
}

}
