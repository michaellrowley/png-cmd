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
	if ( !read_backwards( handle, (int*)&current_chunk.size, 4 ) ) {
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
		if ( fseek( handle, current_chunk.size, SEEK_CUR ) != 0x0 ) {
			return FALSE;
		}
	}
	else if ( current_chunk.size != 0 ) {
		current_chunk.data = calloc( current_chunk.size, sizeof(BYTE) );
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

	// Real CRC32
	chunk_crc( &current_chunk );

	*output_buffer = current_chunk;
	return TRUE;
}

void chunk_crc( chunk* chunk_ptr ) {
	// Initial algorithm taken from: https://stackoverflow.com/a/21001712
	// with minor adjustments made to fit the current context.
	uint32_t byte, crc, mask;

	if ( chunk_ptr->data == nullptr ) {
		chunk_ptr->real_checksum = 0x0;
		return;
	}

	crc = 0xFFFFFFFF;
	// Name
	for ( unsigned char i = 0; i < 4; i++ ) {
		byte = chunk_ptr->name[i];
		crc = crc ^ byte;
		for (char j = 7; j >= 0; j--) {
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
	}
	// Data
	for ( uint32_t i = 0; i < chunk_ptr->size; i++ ) {
		byte = chunk_ptr->data[i];
		crc = crc ^ byte;
		for (char j = 7; j >= 0; j--) {
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
	}
	chunk_ptr->real_checksum = ~crc;
	return;
}