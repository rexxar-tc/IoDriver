#include <SPI_io.hpp>

int byteCount = 0;
bool readFirst = true;
bool writeFirst = true;

void select_EE2()
{
    char RDY = 255;
    SPI.setBitOrder( MSBFIRST );        //SPI config stuff
    SPI.setDataMode( SPI_MODE0 );
    CS_address( EE2_CS, 0 );      //pull CS low to activate EEPROM
    while ( bitRead( RDY, 0 ) == 1 )    //check if device is busy
    {
        CS_address( EE2_CS, 0 );
        SPI.transfer( RDSR );
        RDY = SPI.transfer( 0 );
        CS_address( EE2_CS, 1 );
        delay( 1 );                     //wait for it to finish
    }
    SPI.transfer( WRSR );               //select status register write
    SPI.transfer( 2 );                  //unprotect all EEPROM2 memory space
    CS_address( EE2_CS, 1 );
    delay(1);
    CS_address( EE2_CS, 0 );
    SPI.transfer( WREN );               //enable writing
    CS_address( EE2_CS, 1 );
    delay(1);
    CS_address( EE2_CS, 0 );
}

char read_EE2( int address )
{
    char SI_buf = 0;
    char* r_bytes;

    select_EE2();
    SPI.transfer( READ );               //select read mode

    r_bytes = (char*)&address;            //put the bytes of address into an array
    SPI.transfer( r_bytes [1] );          //send individual address bytes
    SPI.transfer( r_bytes [0] );
    SI_buf = SPI.transfer( 0 );         //read the selected byte into a buffer
    CS_address( EE2_CS, 1 );     //pull CS high to deselect EEPROM
    return SI_buf;                      //return read byte
}

void write_EE2 ( int address, char data )
{
    select_EE2();
    char* w_bytes;
    SPI.transfer( WRITE );              //select write mode
    w_bytes = (char*)&address;            //put the bytes of address into an array
    SPI.transfer( w_bytes [1] );          //send individual address bytes
    SPI.transfer( w_bytes [0] );
    SPI.transfer( data );               //send the data
    CS_address( EE2_CS, 1 );     //pull CS high to disconnect and begin write process
    //delay(5);
}

void setup_EE2()
{
    SPI.transfer( WRSR );                   //unprotect all EEPROM2 memory space
    SPI.transfer( 2 );
    CS_address( EE2_CS, 1 );
    delay(10);
    CS_address( EE2_CS, 0 );
    SPI.transfer( WREN );               //enable writing
    CS_address( EE2_CS, 1 );
    delay(10);
}

void CS_address( char address, char state )
{
    //insert code to select CS lines on shift register
}
