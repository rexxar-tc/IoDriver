#include <color.hpp>

#include <vector>

#include <EEPROM.h>

namespace Sabre {

rgbw::rgbw()
:   r(0),
    g(0),
    b(0),
    w(0)
{

}

ColorData::ColorData()
:   tval( 0 ),
    xval( NULL ),
    end_flag( false )
{

}

ColorData::ColorData( const ColorData& rval )
:   cval( rval.cval ),
    tval( rval.tval ),
    xval( NULL ),
    end_flag( rval.end_flag )
{

}

ColorData::~ColorData() {
    delete xval;
}

uint8_t ColorData::get_r() {
    return cval.r;
}

uint8_t ColorData::get_g() {
    return cval.g;
}

uint8_t ColorData::get_b() {
    return cval.b;
}

uint8_t ColorData::get_w() {
    return cval.w;
}

uint16_t ColorData::get_t() {
    return tval;
}

int ColorData::read_color( unsigned int address ) {
    uint8_t lb;
    uint8_t bb;
    uint8_t rval;
    uint8_t gval;
    uint8_t bval;
    uint8_t wval;

    rval     = EEPROM.read( address+0 );
    gval     = EEPROM.read( address+1 );
    bval     = EEPROM.read( address+2 );
    wval     = EEPROM.read( address+3 );
    lb       = EEPROM.read( address+4 );
    bb       = EEPROM.read( address+5 );
    end_flag = EEPROM.read( address+6 );

    uint8_t* t = (uint8_t*)&this->tval;
    t[0] = lb;
    t[1] = bb;

    rgbw newcval = rgbw( rval, gval, bval, wval );
    cval = newcval;

    return 0;
}

bool ColorData::is_last() {
    return end_flag;
}

void ColorData::generate_gradient( const Sabre::ColorData& next_color ) {
    int ms = tval * 10;

    delete xval;
    xval = new Gradient( cval, next_color.cval, ms );
}

const rgbw& ColorData::get_rgbw() {
    return cval;
}

const rgbw& ColorData::get_grad() {
    return (*xval);
}

uint8_t ColorData::get_xr() {
    return xval->r;
}

uint8_t ColorData::get_xg() {
    return xval->g;
}

uint8_t ColorData::get_xb() {
    return xval->b;
}

int ColorData::update_gradient() {
    return xval->step();
}

void ColorData::reset_gradient() {
    xval->reset();
}

}
