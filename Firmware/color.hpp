#ifndef _IODRIVER_COLOR_HPP
#define _IODRIVER_COLOR_HPP

#include <cstdlib>
#include <inttypes.h>

#define GRADIENT_COMPONENT_COUNT 4
#define COLOR_EFFECT_NONE           0b00000000
#define COLOR_EFFECT_GRADIENT       0b00000001
#define COLOR_EFFECT_STROBE         0b00000010
#define COLOR_STROBE_INTERVAL       40         // milliseconds (full on/off cycle will take twice this time)

namespace IoDriver {

class Component {
public:
    uint8_t v1;
    uint8_t v2;
    uint8_t* vx;
    int freq_ms;
    unsigned long next_update;
    signed int inc;             // Amount to increment the component on the next update (may be negative)
};

class rgbw {
public:
    rgbw() {};
    rgbw( uint8_t, uint8_t, uint8_t, uint8_t );
    rgbw& operator=( const rgbw& );

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t w;
};

class Gradient : public rgbw {
public:
    void reset();
    bool step();
    void regen( const rgbw*, const rgbw*, unsigned long );

private:
    rgbw const* color1;
    rgbw const* color2;
    unsigned long ttime_ms;
    Component c[GRADIENT_COMPONENT_COUNT];
};

class ColorData {

    struct StateInfo {
        unsigned long start_time;
        Gradient xval;
        bool strobe_state;
        unsigned long strobe_last;
    };

    friend class Profile;

    public:
        void start();
        bool update();

    protected:
        rgbw get_immediate() const;
        void init_gradient( const IoDriver::ColorData& );
        bool timeup() const;

    private:
        rgbw        cval;
        uint16_t    tval;
        uint8_t     effects;

        static StateInfo state;

        int read_color( unsigned int );
    };

}

#endif // _IODRIVER_COLOR_HPP
