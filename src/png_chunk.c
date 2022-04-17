#include "png-chunks.h"

// If max_length == 0 we assume the developer intended to specify
// 'unlimited' and disregard the buffer's size.
BOOL read_chunk( FILE* handle, size_t max_length, chunk* output_buffer ) {
	// Assuming that the handle's
	// read position is currently
	// positioned at the start of
	// the chunk.
	chunk current_chunk = { .size = 0, .data = nullptr, .checksum = 0x0,
		.location = 0 };

	if ( fgetpos( handle, &current_chunk.location ) != 0 ) {
		return FALSE;
	}

	// 00 00 00 0D | 49 48 44 52 | ?? ?? ?? ?? | 12 A0 05 5F
	//     SIZE    |     NAME    |     DATA    |    CRC32
	// SIZE
	if ( !read_backwards( handle, (int32_t*)&current_chunk.size, 4 ) ||
		 current_chunk.size < 0 ) {
		return FALSE;
	}

	// NAME
	// current_chunk.name = (char*)calloc( 4 + 1, sizeof( char ) );
	for ( unsigned char i = 0; i < 4; i++ ) {
		current_chunk.name[ i ] = fgetc( handle );
		if ( feof( handle ) ) {
			return FALSE;
		}
	}

	// DATA
	if ( current_chunk.size > max_length && max_length != 0 ) {
		current_chunk.data = (BYTE*)nullptr; // Ignore it.
		if ( fseek( handle, current_chunk.size, SEEK_CUR ) != 0x00 ||
			ferror( handle ) != 0x00 )  {
			return FALSE;
		}
	}
	else if ( current_chunk.size != 0 ) {
		current_chunk.data = calloc( current_chunk.size, sizeof(BYTE) );
		if ( current_chunk.data == nullptr ) {
			return false;
		}
		for ( unsigned int i = 0; i < current_chunk.size; i++ ) {
			current_chunk.data[ i ] = fgetc( handle );
			if ( feof( handle ) ) {
				free( current_chunk.data );
				return FALSE;
			}
		}
	}

	// CRC32
	if ( !read_backwards( handle, (BYTE*)&current_chunk.checksum, 4 ) ) {
		free( current_chunk.data );
		return FALSE;
	}

	*output_buffer = current_chunk;
	return TRUE;
}