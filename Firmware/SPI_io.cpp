#include <SPI_io.hpp>

int byteCount = 0;
bool readFirst = true;
bool writeFirst = true;

void select_EE2()
{
    char RDY = 255;
    SPI.setBitOrder( MSBFIRST );        //SPI config stuff
    SPI.setDataMode( SPI_MODE0 );
    digitalWrite( PIN_CS_E, LOW );      //pull CS low to activate EEPROM
    while ( bitRead( RDY, 0 ) == 1 )    //check if device is busy
    {
        digitalWrite( PIN_CS_E, LOW );
        SPI.transfer( RDSR );
        RDY = SPI.transfer( 0 );
        digitalWrite( PIN_CS_E, HIGH );
        delay( 1 );                     //wait for it to finish
    }
    SPI.transfer( WRSR );               //select status register write
    SPI.transfer( 2 );                  //unprotect all EEPROM2 memory space
    digitalWrite( PIN_CS_E, HIGH );
    delay(1);
    digitalWrite( PIN_CS_E, LOW );
    SPI.transfer( WREN );               //enable writing
    digitalWrite( PIN_CS_E, HIGH );
    delay(1);
    digitalWrite( PIN_CS_E, LOW );
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
    digitalWrite( PIN_CS_E, HIGH );     //pull CS high to deselect EEPROM
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
    digitalWrite( PIN_CS_E, HIGH );     //pull CS high to disconnect and begin write process
    //delay(5);
}

void setup_EE2()
{
    SPI.transfer( WRSR );                   //unprotect all EEPROM2 memory space
    SPI.transfer( 2 );
    digitalWrite( PIN_CS_E, HIGH );
    delay(10);
    digitalWrite( PIN_CS_E, LOW );
    SPI.transfer( WREN );               //enable writing
    digitalWrite( PIN_CS_E, HIGH );
    delay(10);
}

void erase_EE2()
{
    for( int p = 0; p < 8; ++p )        //loop through all 8 pages
    {
        select_EE2();
        SPI.transfer( WRITE );          //send write command
        SPI.transfer( p );              //send page address
        SPI.transfer( 0 );              //send data address 0
        for( int d = 0; d < 256; ++d )  //send 255 bytes of 0
            SPI.transfer( 0 );
        digitalWrite( PIN_CS_E, HIGH ); //pull CS line high to begin write
        delayMicroseconds( 100 );
    }
}

void bulkWrite_EE2( int address, char* data, int length )
{
    char* w_bytes;
    select_EE2();
    SPI.transfer( WRITE );
    w_bytes = (char*)&address;              //put the bytes of address into an array
    SPI.transfer( w_bytes [1] );            //send page address (first byte)
    SPI.transfer( w_bytes [0] );            //send starting address
    int addrBuf = w_bytes [0];
    char pageBreak = 0;
    char bufBreak = 0;

    for( int i = 0; i < length; ++i )
    {
        if(( addrBuf + i ) % 255 == 0 )         //check if we've reached the end of the current page
        {
            ++pageBreak;                   //increment page break counter
            digitalWrite( PIN_CS_E, HIGH );     //pull CS line high to begin write
            bufBreak = 0;
            select_EE2();
            SPI.transfer( w_bytes [1] + pageBreak ); //send address of next page
            SPI.transfer( 0 );              //send address 0 to start writing new page
        }

        if( i % 32 == 0 )
        {
            ++bufBreak;
            digitalWrite( PIN_CS_E, HIGH );     //pull CS line high to begin write
            if( addrBuf + i <= 223 )
            {
                select_EE2();
                SPI.transfer( WRITE );
                SPI.transfer( addrBuf + ( 32 * bufBreak ));
            }
        }

        SPI.transfer( data [i] );           //load bytes into EEPROM buffer
    }
    digitalWrite( PIN_CS_E, HIGH );             //begin write
}
