#ifndef _IODRIVER_PROFILE_HPP
#define _IODRIVER_PROFILE_HPP

#include <iodriver.hpp>
#include <color.hpp>

namespace IoDriver {

class Profile {
public:
    Profile();

    int     init();

    rgbw    get_color_value();

    void    first_color();
    void    next_color ();
    void    set_color  ( ColorData* );
    void    set_address( signed int );

protected:

    ColorData* whatsnext();

private:

    int color_count;
    int cindex;
    signed int                      iaddress;
    static ColorData colors[PROFILE_MAX_DATACOUNT];

    ColorData* active_color;
};

}

#endif // _IODRIVER_PROFILE_HPP
