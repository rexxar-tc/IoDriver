#ifndef _IODRIVER_PROFILE_HANDLER_HPP
#define _IODRIVER_PROFILE_HANDLER_HPP

#include <iodriver.hpp>
#include <profile.hpp>

#define PH_PROFILE_COUNT EEPROM1_MAX_PROFILES

namespace IoDriver {

class ProfileHandler {
    public:
        ProfileHandler();
        int enum_profiles();
        Profile* first_profile();
        Profile* next_profile();
        Profile* preview_profile();

        Profile* active() { return active_profile; };
    protected:
        Profile profiles[PH_PROFILE_COUNT];
        Profile preview;
        Profile* active_profile;
        int pindex;
        int profile_count;
        Profile* activate_profile( Profile* );
};

}

#endif // _IODRIVER_PROFILE_HANDLER_HPP
