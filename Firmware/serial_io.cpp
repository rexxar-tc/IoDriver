#include <cstdlib>

#include <Arduino.h>

#include <iodriver.hpp>
#include <EEPROM_io.hpp>
#include <parser.hpp>
#include <EEPROMHandler.hpp>
#include <profilehandler.hpp>
#include <I2C.hpp>

extern int freeRam();
unsigned long serialTimer = 0;
int lastBuf = 0;
//extern bool chg_preview;

void cmd_handshake( arg_t args, int n_args ) {
    Serial.println( F("hello") );
}

void cmd_version( arg_t args, int n_args ) {
    Serial.println( F("Firmware: " FIRMWARE_VERSION_NUMBER) );
    Serial.println( F("Hardware: " HARDWARE_VERSION_NUMBER) );
}

void cmd_params( arg_t args, int n_args ) {
#define PRINT_SPEC( param )            \
    Serial.print  ( F( #param "=" ) ); \
    Serial.println(     param       );
    PRINT_SPEC( FIRMWARE_VERSION_NUMBER             );
    PRINT_SPEC( HARDWARE_VERSION_NUMBER             );
    PRINT_SPEC( EEPROM1_MAX_PROFILES                );
    PRINT_SPEC( PROFILE_NAME_MAX_CHARS              );
    PRINT_SPEC( PROFILE_MAX_DATACOUNT               );
    PRINT_SPEC( EEPROM1_SIZE                        );
#undef PRINT_SPEC
}

void cmd_read( arg_t args, int n_args ) {
    char* arg_start = find_arg( "a", args, n_args );
    char* arg_end   = find_arg( "e", args, n_args );
    char* arg_len   = find_arg( "l", args, n_args );
    if ( !arg_start ) return;

    int start_address = atoi( arg_start );
    int end_address   = start_address;
    if ( arg_end ) {
        end_address = atoi( arg_end );
    } else {
        if ( arg_len ) {
            int length = atoi( arg_len );
            if ( !length ) return;

            if ( length > 0 ) {
                end_address = start_address+length-1;
            } else {
                end_address = start_address+length+1;
            }
        }
    }

    read_data( start_address, end_address );
}

void cmd_write( arg_t args, int n_args ){
    // Get address to write to and abort if not given
    char* arg_address = find_arg( "a", args, n_args );
    char* arg_data    = find_arg( "data", args, n_args );
    if ( !arg_address || !arg_data ) return;

    // Get value string and separate by delimiter
    param_t values[ARG_LIST_MAX_ARGS]; // arg_t
    int n_values = subdivide_args( arg_data, values );

    // Copy data into a buffer
    char data[ARG_LIST_MAX_ARGS];
    for ( int i = 0; i < n_values; ++i ) {
        data[i] = atoi( values[i][0] );
    }

    // Send it away to be written
    write_data( atoi( arg_address ), data, n_values );
}

void cmd_erase( arg_t args, int n_args )
{
    char* arg_start = find_arg( "a", args, n_args );
    char* arg_end   = find_arg( "e", args, n_args );
    int start_address = atoi( arg_start );
    int end_address = atoi( arg_end );
    for( int i = start_address ; i <= end_address; i++ )
        EEPROM_H.write( i, 0 );
    Serial.println("Done");
}

void cmd_register( arg_t args, int n_args )
{
    char* arg_address = find_arg( "a", args, n_args );
    char* arg_bytes   = find_arg( "b", args, n_args );
    int address = atoi( arg_address );
    int bytes = atoi( arg_bytes );
    getRegister( address, bytes );
}

void cmd_profw( arg_t args, int n_args ) {
    char* arg_name  = find_arg( "name", args, n_args );
    char* arg_index = find_arg( "i",    args, n_args );
    char* arg_empty = find_arg( "empty",args, n_args );

    // Abort if no index
    if ( !arg_index ) return;
    // Index to int
    int index = atoi( arg_index );

    // Check to make sure the profile exists,
    // and create a new profile if it doesn't
    bool exists = profile_exists( index );
    if ( !exists ) {
        exists = create_profile( index );
        // If profile creation fails, abort
        if ( !exists ) return;
    }

    // Write the profile name if indicated
    if ( arg_name ) {
        write_name( index, arg_name );
    }
    // Mark empty if indicated
    if ( arg_empty && atoi( arg_empty ) == 1 ) {
        mark_empty( index );
    }
    // Cancel preview if active
    chg_preview = false;
}

void cmd_profwc( arg_t args, int n_args ) {
    char* arg_index   = find_arg( "i", args, n_args );
    char* arg_cindex  = find_arg( "j", args, n_args );
    char* arg_r       = find_arg( "r", args, n_args );
    char* arg_g       = find_arg( "g", args, n_args );
    char* arg_b       = find_arg( "b", args, n_args );
    char* arg_w       = find_arg( "w", args, n_args );
    char* arg_t       = find_arg( "t", args, n_args );
    char* arg_u       = find_arg( "u", args, n_args );
    char* arg_last    = find_arg( "last", args, n_args );

    // Abort if no indices
    if ( !arg_index || !arg_cindex ) return;
    // Indices to ints
    int pindex = atoi( arg_index  );
    int cindex = atoi( arg_cindex );

    // Write color info
    int r = ( arg_r ? atoi( arg_r ) : -1 );
    int g = ( arg_g ? atoi( arg_g ) : -1 );
    int b = ( arg_b ? atoi( arg_b ) : -1 );
    int w = ( arg_w ? atoi( arg_w ) : -1 );
    int t = ( arg_t ? atoi( arg_t ) : -1 );
    int u = ( arg_u ? atoi( arg_u ) : -1 );
    int last = ( arg_last ? atoi( arg_last ) : -1 );
    write_color( pindex, cindex, r, g, b, w, t, u, last );
}

void cmd_profmv( arg_t args, int n_args ) {
    char* arg_indices = find_arg( "i", args, n_args );
    if ( !arg_indices ) return;

    // Further parse indices for detailed data
    param_t values[ARG_LIST_MAX_ARGS]; // arg_t
    int n_values = subdivide_args( arg_indices, values );

    // Convert arg_t to integral array
    int indices[ARG_LIST_MAX_ARGS][2];
    int n_indices = 0;
    for ( int i = 0; i < n_values; ++i ) {
        int index1 = atoi( values[i][0] );
        int index2 = atoi( values[i][1] );

        if ( index1 == 0 || index2 > EEPROM1_MAX_PROFILES ) continue;

        indices[i][0] = index1;
        indices[i][1] = index2;
        ++n_indices;
    }

    reorder_profiles( indices, n_values );
}

void cmd_profdel( arg_t args, int n_args ) {
    char* arg_index = find_arg( "i", args, n_args );
    if ( !arg_index ) return;

    int index = atoi( arg_index );
    delete_profile( index );
}

void cmd_profprint( arg_t args, int n_args ) {
    char* arg_index = find_arg( "i", args, n_args );
    if ( !arg_index ) return;

    // Convert profile index argument to integer
    unsigned int index = atoi( arg_index );

    print_profile( index );
}

void cmd_voltage( arg_t args, int n_args ) {
    //checkVoltDebug();
    Serial.println( checkVoltage() * ADC_CONVERSION );
}

void cmd_memory( arg_t args, int n_args ) {
    Serial.println( freeRam() );
}

void cmd_preview( arg_t args, int n_args ) {
    extern IoDriver::ProfileHandler ph;
    ph.preview_profile();
    chg_preview = true;
}

void cmd_reload( arg_t args, int n_args ) {
    extern IoDriver::ProfileHandler ph;
    (void)ph.enum_profiles();
    (void)ph.first_profile();
}

void checkSerial() {

    if ( Serial.available() > 0 )                       //is there data in the serial buffer?
    {

        if( lastBuf == Serial.available() && millis() < (serialTimer + (SERIAL_DELAY)))
        {
            //Serial.println("DEBUG 1");
            return;
        }
                                                 //if the buffer has not increased do nothing
        else if ( lastBuf < Serial.available() )        //if buffer has increased reset the timer
        {
            //Serial.println("DEBUG 2");
            lastBuf = Serial.available();
            serialTimer = millis();
            return;
        }
        else                                            //buffer has not changed for more than 3 byte periods
        {
            //Serial.println("DEBUG 3");
            lastBuf = 0;
        }
                                            //reset the if block and continue with the program

    }

    if ( Serial.available() == 0 )
        return;
    // Get the number of bytes to read and abort if there is no data
    int bufsize = Serial.available();
    // Skip lone characters.. this happens sometimes on PuTTY,
    // when a lone \r lags behind the rest of the data being sent
    // and shows up in an iteration of this function all on its lonesome.
    if ( bufsize == 1 ) {
        (void) Serial.read();
        //Serial.println("DEBUG 4");
        return;
    }

    // Create a buffer and read serial data into it
    char buffer[SERIAL_BUFFER_SIZE+1];
    Serial.readBytes ( buffer, bufsize );
    //Serial.println("DEBUG 5");
    /*for (int i = 0; i<= bufsize; ++i)
    {
        Serial.print(buffer[i]);
    }*/
    // Make sure the buffer terminates,
    // and get rid of any carriage returns
    buffer[bufsize] = '\0';
    if ( buffer[bufsize-1] == '\r' ) {
        buffer[bufsize-1] = '\0';
    }

    // Parse arguments and return if none were given
    param_t args[CMD_MAX_ARGS]; // arg_t
    int n_args = parse_args( buffer, args );
    if ( !n_args ) return;

    // Get command from args array
    char* cmd = args[0][0];

    bool routed = 0;

#define ROUTE_CMD( x, y )                    \
if ( 0 == strcmp_P( cmd, (char*)F( x ) ) ) { \
    y ( args, n_args );                      \
    routed = 1;                              \
}
    ROUTE_CMD( "handshake-iodriver",    cmd_handshake );
    ROUTE_CMD( "version",               cmd_version );
    ROUTE_CMD( "params",                cmd_params );
    ROUTE_CMD( "read",                  cmd_read );
    ROUTE_CMD( "write",                 cmd_write );
    ROUTE_CMD( "prof-print",            cmd_profprint );
    ROUTE_CMD( "prof-w",                cmd_profw );
    ROUTE_CMD( "prof-wc",               cmd_profwc );
    ROUTE_CMD( "prof-mv",               cmd_profmv );
    ROUTE_CMD( "prof-del",              cmd_profdel );
    ROUTE_CMD( "voltage",               cmd_voltage );
    ROUTE_CMD( "memory",                cmd_memory );
    ROUTE_CMD( "preview",               cmd_preview );
    ROUTE_CMD( "reload",                cmd_reload );
    ROUTE_CMD( "erase",                 cmd_erase );
    ROUTE_CMD( "register",              cmd_register );
#undef ROUTE_CMD
    if ( !routed ) {
        Serial.print( F("Unknown command: ") );
        for (int i = 0; i<= bufsize; ++i)
        {
            Serial.print(buffer[i]);
        }
        Serial.print("\n");
    }
}
