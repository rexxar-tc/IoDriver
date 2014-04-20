#ifndef _SABRE_COLOR_HPP
#define _SABRE_COLOR_HPP

#include <inttypes.h>

#include <vector>

namespace Sabre {

class Component {
public:
    uint8_t v1;
    uint8_t v2;
    uint8_t* vx;
    int freq_ms;
    unsigned long next_update;
    signed int inc;
};

class rgbw {
public:
    rgbw();
    rgbw( uint8_t, uint8_t, uint8_t, uint8_t );
    rgbw operator=( const rgbw& );

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
};

class Gradient : public rgbw {
public:
    Gradient( const rgbw&, const rgbw&, unsigned long );
    void reset();
    int step();

private:
    rgbw color1;
    rgbw color2;
    unsigned long ttime_ms;
    Component c[3];
};

class ColorData {

    friend class Profile;

    public:
        ColorData();
        ColorData( const ColorData& );
        ~ColorData();

        uint8_t  get_r();
        uint8_t  get_b();
        uint8_t  get_g();
        uint8_t  get_w();
        const rgbw& get_rgbw();
        const rgbw& get_grad();
        uint16_t get_t();

        uint8_t get_xr();
        uint8_t get_xg();
        uint8_t get_xb();

        bool is_last();

        void generate_gradient( const Sabre::ColorData& );
        int update_gradient();
        void reset_gradient();

    protected:

    private:
        rgbw        cval;
        Gradient*   xval;
        uint16_t    tval;
        bool        end_flag;

        int read_color( unsigned int );
    };

}

#endif // _SABRE_COLOR_HPP
