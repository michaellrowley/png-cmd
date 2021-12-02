#include "png-chunks.h"

void free_chunk( pchunk chnk ) {
	if ( chnk->data != nullptr ) {
		free( chnk->data );
		chnk->data = nullptr;
	}
}

BOOL is_string_number( const char* string, size_t len ) {
	for ( size_t i = 0; i < len; i++ ) {
		if ( !isdigit( string[ i ] ) ) {
			return FALSE;
		}
	}
	return TRUE;
}

BOOL read_backwards( FILE* src_handle, BYTE* buf, unsigned char len ) {
	// Assuming that 'buf' as already been allocated to an appropriate size.
	for ( unsigned char index = 0; index < len; index++ ) {
		buf[ ( len - 1 ) - index ] = fgetc( src_handle );
		if ( feof( src_handle ) ) {
			return FALSE;
		}
	}
	return TRUE;
}

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