#ifndef _IODRIVER_SPI_HPP_
#define _IODRIVER_SPI_HPP_

#include <Arduino.h>
#include <iodriver.hpp>
#include <SPI.h>

//opcode definitions
#define     WREN            0b00000110      //write enable
#define     WRDI            0b00000100      //write disable
#define     RDSR            0b00000101      //read status register
#define     WRSR            0b00000001      //write status register
#define     READ            0b00000011      //read data from memory
#define     WRITE           0b00000010      //write data to memory

//pin definitions

#define     PIN_DSA         7
#define     PIN_MR          3
#define     PIN_CP          8

//CS address definitions
#define     BT_CS           1
#define     ACC_CS          2
#define     AUD_CS          4
#define     EE2_CS          8
#define     AUX_CS          128

char read_EE2( int );
void write_EE2( int, char );
void erase_EE2();
void CS_address( char, char );

#endif // _IODRIVER_SPI_HPP_
