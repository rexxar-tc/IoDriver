#ifndef _IODRIVER_PARSER_HPP
#define _IODRIVER_PARSER_HPP

#define SERIAL_BUFFER_SIZE 64
#define CMD_MAX_ARGS 16
#define ARG_LIST_MAX_ARGS 16
#define CMD_DELIM '|'
#define PARAM_DELIM '='
#define ARG_LIST_DELIM ','
#define TUPLE_DELIM '/'

typedef char*   param_t[2];
typedef param_t arg_t[];

int   parse_args( char*, arg_t );
int   subdivide_args( char*, arg_t );
char* find_arg( const char*, arg_t, int );

#endif // _IODRIVER_PARSER_HPP
