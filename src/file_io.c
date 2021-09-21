#include "png-chunks.h"

BOOL read_bytes( FILE* file_handle, size_t len, BYTE* buffer ) {
	if ( buffer == nullptr || len == 0 ) {
		return FALSE;
	}
	for ( size_t i = 0; i < len; i++ ) {
		BYTE iterative_byte = fgetc( file_handle );
		if ( feof( file_handle ) ) {
			return FALSE;
		}
		buffer[ i ] = iterative_byte;
	}
	return TRUE;
}