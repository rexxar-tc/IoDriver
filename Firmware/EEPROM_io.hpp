#ifndef _IODRIVER_EEPROM_IO_HPP
#define _IODRIVER_EEPROM_IO_HPP

void read_data( int, int );
void write_data( int, const char*, int );

bool profile_exists( int );
bool create_profile( int );

void write_name( int, char* );
void write_color( int, unsigned int, signed int, signed int, signed int, signed int, signed int, signed int, signed int );
signed int delete_profile( int );
void reorder_profiles( int [][2], int );
void mark_empty( int );

void print_profile( int );

#endif // _IODRIVER_EEPROM_IO_HPP
