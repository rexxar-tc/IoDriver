#include <profilehandler.hpp>
#include <EEPROMHandler.hpp>

namespace IoDriver {

ProfileHandler::ProfileHandler()
:   active_profile( NULL )
{

}

Profile* ProfileHandler::first_profile() {
    pindex = 0;
    Profile* pa = ( profile_count ? &profiles[pindex] : NULL );
    return activate_profile( pa );
}

Profile* ProfileHandler::next_profile() {
    pindex = ( pindex+1 < profile_count ? pindex+1 : 0 );
    Profile* pa = ( profile_count || pindex ? &profiles[pindex] : NULL );
    return activate_profile( pa );
}

Profile* ProfileHandler::activate_profile( Profile* profile_to_set ) {
    // Initialize the newly activated profile and mark it active
    if ( profile_to_set ) {
            profile_to_set->init();
    }
    active_profile = profile_to_set;
    return active_profile;
}

Profile* ProfileHandler::preview_profile() {
    return activate_profile( &preview );
}

int ProfileHandler::enum_profiles() {
    // Initialize preview address in virtual eeprom
    preview.set_address( EEPROM_H_VIRTUAL_START );

    signed int p_address[PH_PROFILE_COUNT];

    // Initialize to bogus addresses
    for ( int i = 0; i < PH_PROFILE_COUNT; ++i ) {
        p_address[i] = -1;
    }

    // Iterate through each profile address
    for ( int i = 0; i < PH_PROFILE_COUNT; ++i ){
        signed int address = EEPROM1_FIRST_PROFILE_OFFSET + i * PROFILE_MAX_SIZE;
        uint8_t    pindex  = EEPROM_H.read( address );

        // Store profile address if good
        if ( pindex >= 1 && pindex <= PH_PROFILE_COUNT ) {
            p_address[pindex-1] = address;
        }
    }

    profile_count = 0;
    // Iterate through profiles in their selection order
    for ( int i = 0; i < EEPROM1_MAX_PROFILES; ++i ) {
        // If the profile is valid, save its address and add it to profile container
        if ( -1 != p_address[i] ) {
            Profile& prof = profiles[profile_count];
            prof.set_address( p_address[i] );
            ++profile_count;
        }
    }

    return profile_count;
}

}
