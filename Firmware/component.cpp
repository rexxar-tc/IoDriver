#include <color.hpp>

#include <vector>

#include <Arduino.h>

namespace Sabre {

// Constructor from integral component values
rgbw::rgbw( uint8_t valr, uint8_t valg, uint8_t valb, uint8_t valw )
:   r( valr ),
    g( valg ),
    b( valb ),
    w( valw )
{

}

// Assignment operator overload
rgbw rgbw::operator=( const rgbw& rval )
{
    r = rval.r;
    g = rval.g;
    b = rval.b;
    w = rval.w;
}

Gradient::Gradient( const rgbw& color_begin, const rgbw& color_end, unsigned long transition_time )
:   rgbw( color_begin ),
    color1( color_begin ),
    color2( color_end   ),
    ttime_ms( transition_time )
{
    c[0].v1 = color1.r;
    c[0].v2 = color2.r;
    c[0].vx = &r;
    c[1].v1 = color1.g;
    c[1].v2 = color2.g;
    c[1].vx = &g;
    c[2].v1 = color1.b;
    c[2].v2 = color2.b;
    c[2].vx = &b;

    for ( int i = 0; i < 3; ++i ) {
        int diff = c[i].v2 - c[i].v1;
        if ( diff < 0 ) {
            c[i].inc = -1;
            diff = 0 - diff;
        } else {
            c[i].inc = 1;
        }
        c[i].freq_ms = transition_time / diff;
    }
}

int Gradient::step() {
    int finished = 1;
    unsigned long ms_now = millis();

char fstr[32];

    for ( int i = 0; i < 3; ++i ) {
        uint8_t v1 = c[i].v1;
        uint8_t v2 = c[i].v2;
        uint8_t vx = *(c[i].vx);

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

void Gradient::reset() {
    unsigned long ms_now = millis();
    for ( int i = 0; i < 3; ++i ) {
        *(c[i].vx) = c[i].v1;
        c[i].next_update = ms_now + c[i].freq_ms;
    }
}

}
