#include <string>
#include <sstream>

#include <Arduino.h>
#include <EEPROM.h>

#include <profile.hpp>

void serialRead( std::string );
void serialWrite( std::string );
void serialProfile( std::string );
void serialHandshake( std::string );
unsigned int parse_int( const std::string& );

void calReference();
long checkVoltage( bool );
int getSOC();

Sabre::Profile* seekProfile( unsigned int );

extern std::vector<Sabre::Profile*>::iterator activeProfile;

void checkSerial(){
    while (Serial.available() > 0){
    int bufsize = Serial.available();
    char* buf = new char[bufsize];

    Serial.readBytes ( buf, bufsize );
    Serial.flush();

    std::string bufcopy( buf, bufsize );

    delete buf;

    size_t pos = bufcopy.find( '.', 0 );
    std::string cmd = bufcopy.substr( 0, pos );
    std::string arg = bufcopy.substr( pos, bufcopy.size() - pos );

    if          ( cmd == "read"  ) {
        serialRead( arg );
    } else if   ( cmd == "write" ) {
        serialWrite( arg );
    } else if   ( cmd == "profile" ) {
        serialProfile( arg );
    } else if   ( cmd == "handshake" ) {
        serialHandshake( arg );
    } else if   ( cmd == "voltage" ) {
        Serial.println(checkVoltage(true));
        //do not call this twice in a row or it will kill the program
        //I have no idea why.
    } else if     (cmd == "calibrate" ) {
        calReference();
        Serial.println("done");
        Serial.print("cal offset: ");
        Serial.println(checkVoltage( false ) * 4200);
    } else if     (cmd == "SOC" ) {
        //estimates State of Charge. Returns 0-100.
        Serial.print(getSOC());
    } else {
        Serial.println("Unknown command.");
    }

  }
}

void serialRead( std::string buf ) {
    size_t pos = 0;
    while( pos != std::string::npos ) {
        size_t apos = std::string::npos;
        size_t lpos = std::string::npos;

        pos = buf.find( '.', pos );
        if ( pos != std::string::npos ) {
            pos  = pos+1;
            apos = pos;
        }

        pos = buf.find( '.', pos );
        if ( pos!= std::string::npos ) {
            pos  = pos+1;
            lpos = pos;
        }

        // If both an address and a length were found
        if ( apos != std::string::npos && lpos != std::string::npos ) {
            unsigned int address  = parse_int( buf.substr( apos, buf.size() - apos ) );
            unsigned int readsize = parse_int( buf.substr( lpos, buf.size() - lpos ) );

            // Read bytes and concatenate into a string to output
            std::stringstream output;
            for ( int i = 0; i < readsize; ++i ) {
                output << EEPROM.read( address + i );
            }
            Serial.println( output.str().c_str() );
        }
    }
}

void serialWrite( std::string buf ){
    size_t pos = 0;
    while( pos != std::string::npos ) {
        size_t apos = std::string::npos;
        size_t dpos = std::string::npos;

        pos = buf.find( '.', pos );
        if ( pos != std::string::npos ) {
            pos  = pos+1;
            apos = pos;
        }

        pos = buf.find( '.', pos );
        if ( pos != std::string::npos ) {
            pos  = pos+1;
            dpos = pos;
        }

        // If both an address and a byte to write were found
        if ( apos != std::string::npos && dpos != std::string::npos ) {
            unsigned int address = parse_int( buf.substr( apos, buf.size() - apos ) );
            unsigned int byteval =            buf[dpos];

            // Write bytes and continue
            EEPROM.write( address, byteval );
        }
    }
}

void serialProfile( std::string buf ) {
    if ( buf[0] != '.' && buf[0] != ',' ) {
        return;
    }
    unsigned int index = parse_int( buf.substr( 1, buf.size() - 1 ) );
    // Try to find the requested profile
    Sabre::Profile* prof = seekProfile( index );
    if ( NULL != prof ) {
        // If found, print stuff
        Serial.println( prof->get_name().c_str() );

        std::vector<Sabre::ColorData> colors = prof->get_colors();
        for ( std::vector<Sabre::ColorData>::iterator iter = colors.begin(); iter < colors.end(); ++iter ) {

            std::stringstream col_str;
//            col_str << "RGBW:\t";
            col_str << (int)iter->get_r() << ' ';
            col_str << (int)iter->get_g() << ' ';
            col_str << (int)iter->get_b() << ' ';
            col_str << (int)iter->get_w() << ' ';

//            Serial.println( col_str.str().c_str() );
//            col_str.str( "" );


col_str << iter->get_t() << ' ';
if ( iter+1 < colors.end() ) {
    col_str << '0';
} else {
    col_str << '1';
}
Serial.println( col_str.str().c_str() );
//            col_str << "Time:\t" << iter->get_t() << " ms";
//            Serial.println( col_str.str().c_str() );

        }
    } else {
        std::stringstream err_msg;
        err_msg << "Profile " << index << " not found.";
        Serial.println( err_msg.str().c_str() );
    }
}

void serialHandshake( std::string buf ) {
    if ( ',' == buf[0] || '.' == buf[0] ) {
        if ( "ARDUINO_SABRE" == buf.substr( 1, 13 ) ) {
            Serial.println( "hello" );
        }
    }
}

unsigned int parse_int( const std::string& buf ) {
    int counter = 0;

    size_t pos = buf.find( '.', 0 );
    size_t len = ( pos == std::string::npos ? buf.size() : pos );

    std::string digits = buf.substr( 0, len );
    unsigned int convert = atoi( digits.c_str() );

    return convert;
}
