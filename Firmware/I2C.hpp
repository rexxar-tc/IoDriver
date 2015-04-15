#ifndef _IODRIVER_I2C_HPP_
#define _IODRIVER_I2C_HPP_

#include <Wire.h>

// Forward declarations
void setupCharge();
void setCurrent( int );
void chargeState();
void configCharger( int, int );
void disableCharge();
void writeLED( int, int, int, int );
uint32_t calReference( bool );
bool chargeTimeout();
bool chargingTimeout();
void getRegister( int, int );

#endif // _IODRIVER_I2C_HPP_
