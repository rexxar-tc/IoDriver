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

//CS address definitions
#define     PIN_CS_E        A3
#define     PIN_CS_1        8
#define     PIN_CS_2        7

#define     PIN_Vexp        3

char read_EE2( int );
void write_EE2( int, char );
void erase_EE2();
void bulkWrite_EE2( int address, char* data, int length );

#endif // _IODRIVER_SPI_HPP_
