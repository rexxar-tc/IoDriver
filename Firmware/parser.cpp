#include <cstddef>
#include <string.h>
#include <parser.hpp>

int get_instances( char* buffer, arg_t results, char delim ) {
    // Get arguments
	int n_args = 0;
	int startpos = 0;
	for ( int i = 0; i <= SERIAL_BUFFER_SIZE; ++i ) {
        bool terminate = ( '\0' == buffer[i] );
		if ( delim == buffer[i] || terminate ) {
            // Terminate the substring
			buffer[i] = '\0';
			// Store the substring as an argument if longer than 0
			if ( startpos != i ) {
                results[n_args][0] = &buffer[startpos];
                ++n_args;
			}
			startpos = i+1;
		}

        if ( terminate ) break;
	}

	return n_args;
}

void get_pairs( param_t arg, char delim ) {
    for ( int j = 0; j <= SERIAL_BUFFER_SIZE; ++j ) {
        // Check for an arg delimiter
        if ( arg[0][j] == delim ) {
            arg[0][j] = '\0';
            arg[1] = &arg[0][j+1];
            break;
        }

        // Stop if this is the end of the string
        if ( arg[0][j] == '\0' ) {
            arg[1] = NULL;
            break;
        }
    }
}

int process_args( char* buffer, arg_t segments, char delim1, char delim2 ) {
    // Divide the string by the delimiting character to get individual segments
    int n_segments = get_instances( buffer, segments, delim1 );
    // Dividne the substsrings by the secondary delimiter to get pairs
    for ( int i = 0; i < n_segments; ++i ) {
        get_pairs( segments[i], delim2 );
    }

    return n_segments;
}

int parse_args( char* buffer, arg_t args ) {
    return process_args( buffer, args, CMD_DELIM, PARAM_DELIM );
}

int subdivide_args( char* buffer, arg_t args ) {
    return process_args( buffer, args, ARG_LIST_DELIM, TUPLE_DELIM );
}

char* find_arg( const char* arg_to_find, arg_t args, int n_args ) {
    // Iterate through arguments until the argument is found, then return
    for ( int i = 0; i < n_args; ++i ) {
        if ( 0 == strcmp( args[i][0], arg_to_find ) ) {
            return args[i][1];
        }
    }
    return NULL;
}
