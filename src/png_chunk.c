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

	// 00 00 00 0D | 49 48 44 52 | FA .. .. 9B | 12 A0 05 5F
	//     SIZE    |     NAME    |     DATA    |    CRC32
	// SIZE
	if ( !read_bytes( handle, 4, (uint32_t*)&current_chunk.size ) ) {
		return FALSE;
	}
	INT32_FLIP(current_chunk.size);

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
	if ( !read_bytes( handle, 4, (BYTE*)&current_chunk.checksum ) ) {
		free( current_chunk.data );
		return FALSE;
	}
	INT32_FLIP(current_chunk.checksum);

	*output_buffer = current_chunk;
	return TRUE;
}

BOOL strip_chunk( FILE* png_handle, const char* chunk_name, const int chunk_index ) {
	chunk iterative_chunk;
	unsigned int chunk_iterative_index = 0;
	if (chunk_name == nullptr && chunk_index == -1) {
		return false;
	}

	while ( read_chunk( png_handle, 0, &iterative_chunk ) ) {
		chunk_iterative_index++;
		if ( chunk_iterative_index == UINT_MAX ) {
			free( iterative_chunk.data );
			return FALSE;
		}
		if ( ( chunk_name != nullptr && strncmp( iterative_chunk.name,
				chunk_name, 4 ) != 0 ) ||
			 ( chunk_index != -1 && chunk_iterative_index != chunk_index + 1 ) ) {
			free_chunk( &iterative_chunk );
			continue;
		}

		// Found the chunk!
		if ( 0 != fseek( png_handle, FPOS_GETVAL( iterative_chunk.location ) + 4, SEEK_SET ) ) {
			printf( "Unable to perform an IO operation while wiping chunk '%.4s'.\n",
				iterative_chunk.name );
			free_chunk( &iterative_chunk );
			return FALSE;
		}

		// Adding eight to iterative_chunk.size allows us to
		// overwrite the CRC32 which could provide information
		// to anyone that needs to narrow down the possible
		// value(s) of the chunk we are wiping.
		// That only accounts for four bytes though (CRC32 =
		// four byte integer in PNG) - the other four bytes
		// are the chunk's identifier (four 1-byte characters)
		// so that anyone analyzing the PNG also can't tell
		// what chunk was previously there.
		for ( uint32_t byte_index = 0; byte_index < iterative_chunk.size + 8; byte_index++ ) {
			if ( 0 != fputc( (char)0x0, png_handle ) ) {
				printf( "Unable to write to chunk '%.4s'.\n", iterative_chunk.name );
			}
			if ( feof( png_handle ) ) {
				printf( "Unable to write at 0x%08X (chunk '%.4s').",
					(unsigned int)( FPOS_GETVAL( iterative_chunk.location ) + byte_index ),
					iterative_chunk.name );
				free_chunk( &iterative_chunk );
				return FALSE;
			}
			// TODO: Logging so that we know the program isn't
			// frozen on larger chunks.
		}

		printf( "Filled '%.4s' with null bytes.\n", iterative_chunk.name );
		free_chunk( &iterative_chunk );
		return TRUE;
	}

	printf( "Unable to locate chunk within the provided file.\n" );
	return FALSE; // We couldn't find that chunk.
}

BOOL dump_chunk( FILE* file_handle, unsigned long target_chunk_index ) {
	chunk iterative_chunk;
	unsigned long iterative_chunk_index = 0;
	while ( read_chunk( file_handle, ( size_t ) 65535, &iterative_chunk ) ) {
		if ( target_chunk_index != iterative_chunk_index ) {
			iterative_chunk_index++;
			free_chunk( &iterative_chunk );
			continue;
		}
		
		// We've found the chunk, now we can
		// create an output file for it and
		// write its bytes.
		char file_path[ 14 ];
		
		snprintf( file_path, 14, "%.4s-%08X\x00", iterative_chunk.name, iterative_chunk.location );
		const FILE* const output_handle = fopen( file_path, "w" );
		if ( output_handle == nullptr ) {
			free_chunk( &iterative_chunk );
			return FALSE;
		}
		for (uint32_t i = 0; i < iterative_chunk.size; i++) {
			fputc( iterative_chunk.data[ i ], output_handle );
			if ( ferror( output_handle ) ) {
				free_chunk( &iterative_chunk );
				fclose( output_handle );
				return FALSE;
			}
		}
		free_chunk( &iterative_chunk );
		fclose( output_handle );
		return TRUE;
	}
	free_chunk( &iterative_chunk );
	return FALSE;
}