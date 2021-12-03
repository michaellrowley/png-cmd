#include "png-chunks.h"

BOOL parse_ihdr( BYTE* data, ihdr_data* output_buffer ) {
	if ( data == nullptr || output_buffer == nullptr ) {
		return FALSE;
	}

	ihdr_data ihdr_output = { .width = 0, .height = 0, .bit_depth = 1,
		.colour_type = 1, .compression_type = 1, .filter_type = 1,
		.interlace_type = 1 };

	//  4b    +  4b  = [8b]/64B
	// Height & width:
	for ( unsigned char i = 0; i < 4; i++ ) {
		// Using one loop instead of calling read_backwards twice
		// should save some time.
		( (BYTE*)(&ihdr_output.width)) [ 3 - i ] = data[ i ];
		( (BYTE*)(&ihdr_output.height)) [ 3 - i ] = data[ i + 4 ];
		// w0 w0 w0 w0 | h0 h0 h0 h0
	}

	// 1b + [8b] = [9b]
	// Bit-depth:
	ihdr_output.bit_depth = data[ 8 ];

	// 1b + [9b] = [10b]
	// Colour-type:
	ihdr_output.colour_type = data[ 9 ];

	// 1b + [10b] = [11b]
	// Compression-type:
	ihdr_output.colour_type = data[ 10 ];

	// 1b + [11b] = [12b]
	// Filter-type:
	ihdr_output.colour_type = data[ 11 ];

	// 1b + [12b] = [13b] -> TOTAL
	// Interlace-type:
	ihdr_output.colour_type = data[ 12 ];

	*output_buffer = ihdr_output;
	return TRUE;
}

BOOL list_ancillary_full( FILE* png_handle ) {
	chunk iterative_chunk;
	ihdr_data ihdr;
	unsigned int iterative_chunk_index = 0;
	while ( read_chunk( png_handle, 1000, &iterative_chunk ) ) {
		if ( iterative_chunk_index == UINT_MAX - 1 ) {
			free_chunk( &iterative_chunk );
			return FALSE;
		}

		printf( "%s\n|%u|\n |\n |--- Location: 0x%X\n |--- Size: 0x%X\n |--- CRC32: 0x%X\n |--- Real CRC32: 0x%X\n\n",
			iterative_chunk.name, iterative_chunk_index,
			(unsigned int)iterative_chunk.location.__pos,
			iterative_chunk.size, iterative_chunk.checksum, iterative_chunk.real_checksum );
		
		// IHDR handling (we don't assume that IHDR is the first chunk present).
		if ( strncmp( iterative_chunk.name, "IHDR", 4 ) != 0 ) {
			iterative_chunk_index++;
			free_chunk( &iterative_chunk );
			continue;
		}
		if ( !parse_ihdr( iterative_chunk.data, &ihdr ) ) {
			printf( "Unable to parse IHDR\n" );
			free_chunk( &iterative_chunk );
			return FALSE;
		}
		free_chunk( &iterative_chunk );
		iterative_chunk_index++;
	}
	printf( "\nFile summary:\n\tResolution: %d x %d\n\tBit-depth: %d\n\tColour-type: %d\n\n",
		ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.colour_type );
	return TRUE;
}

BOOL strip_chunk( FILE* png_handle, const char* chunk_name, const int chunk_index ) {
	chunk iterative_chunk;
	unsigned int chunk_iterative_index = 0;
	while ( read_chunk( png_handle, 0, &iterative_chunk ) ) {
		if ( chunk_iterative_index == UINT_MAX ) {
			free( iterative_chunk.data );
			return FALSE;
		}

		if ( ( chunk_name != nullptr && strncmp( iterative_chunk.name,
				chunk_name, 4 ) != 0 ) ||
			 ( chunk_index != -1 && chunk_iterative_index != chunk_index ) ) {

			chunk_iterative_index++;
			free_chunk( &iterative_chunk );
			continue;
		}

		// Found the chunk!		
		if ( 0 != fseek( png_handle, iterative_chunk.location.__pos + 4, SEEK_SET ) ) {
			printf( "Unable to perform an IO operation while wiping chunk '%.4s'.\n",
				iterative_chunk.name );
			chunk_iterative_index++;
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
		for ( unsigned short byte_index = 0; byte_index < iterative_chunk.size + 8; byte_index++ ) {
			if ( 0 != fputc( (char)0x0, png_handle ) ) {
				printf( "Unable to write to chunk '%.4s'.\n", iterative_chunk.name );
			}
			if ( feof( png_handle ) ) {
				printf( "Unable to write at 0x%08X (chunk '%.4s').",
					(unsigned int)( iterative_chunk.location.__pos + byte_index ),
					iterative_chunk.name );
				free_chunk( &iterative_chunk );
				return FALSE;
			}
		}

		printf( "Filled '%.4s' with null bytes.\n", iterative_chunk.name );
		free_chunk( &iterative_chunk );
		return TRUE;
	}

	printf( "Unable to locate chunk '%.4s' within the provided file.\n", chunk_name );
	return FALSE; // We couldn't find that chunk.
}

int main( int argc, char** argv ) {

	if ( argc <= 1 ) {
		printf( "Invalid amount of arguments provided.\n" );

		printf( "Usage:\n\t%s [file.png] | List the chunks in 'file.png'.\n\t%s [file.png] -s [index] | Erases chunk at index 'index' in 'file.png'.\n",
			argv[ 0 ], argv[ 0 ] );

		return -1;
	}

	struct stat* png_stat = (struct stat*)calloc( 1, sizeof( struct stat ) );
	if ( png_stat == nullptr || stat( argv[1], png_stat ) == -1 ||
		!S_ISREG( png_stat->st_mode ) ) {
		
		free( png_stat );
		printf( "Unable to validate the filetype of file '%s'.\n", argv[1] );
		return 1;
	}
	free( png_stat );

	const FILE* png_handle = fopen( argv[ 1 ], "r+" );
	if ( !png_handle ) {
		printf( "Unable to open a handle to file '%s'.\n", argv[ 1 ] );
		return 1;
	}

	// The first bytes of a PNG should be 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x1A, 0x0A
	BYTE* magic_bytes_buffer = (BYTE*)malloc( sizeof(BYTE) * ( 7 + 1 ) );
	if ( !read_bytes( png_handle, 8, magic_bytes_buffer ) ) {
		fclose( png_handle );
		free( magic_bytes_buffer );
		printf( "Unable to read the first seven bytes of '%s'.\n", argv[ 1 ] );
		return 1;
	}
	// I'm not sure why but the last two bytes need to be reversed in order (0x0A <-> 0x1A)
	// this shouldn't impact consistency as far as I'm aware.
	if ( strncmp( magic_bytes_buffer, "\x89\x50\x4E\x47\x0D\x0A\x1A",
		(long unsigned int)7 ) != 0 ) {
		printf( "Unable to verify that the provided file ('%s') is a PNG.\n", argv[1] );
		fclose( png_handle );
		free( magic_bytes_buffer );
		return 1;
	}
	free( magic_bytes_buffer );
	printf( "Validated PNG magic bytes.\n" );

	if ( argc == 2 ) {
		list_ancillary_full( png_handle );		
	}
	else {
		if ( argc == 4 ) {
			const char* operation = argv[ 2 ]; // type of operation
			const char* argument = argv[ 3 ];

			if ( strcmp( operation, "--strip" ) == 0 ||
				 strcmp( operation, "-s" ) == 0 ) {
				if ( is_string_number( argument, strlen( argument ) ) ) {
					// ./program image.png --strip 0
					long target_chunk_index = strtol( argument, ( char** )nullptr, 10 );
					if ( target_chunk_index > INT_MAX || target_chunk_index < INT_MIN ) {
						printf( "Chunk index unable to be casted to an int.\n" );
						return -1;
					}
					strip_chunk( png_handle, nullptr, (int)target_chunk_index );
				}
				else {
					// ./program image.png --strip IHDR
					strip_chunk( png_handle, argv[ 3 ], -1 );
				}
			}
			else {
				printf( "Invalid amount of arguments %s.\n", operation );
				return 1;
			}
		}
		else {
			printf( "Invalid amount of arguments.\n" );
			return 1;
		}
	}

	fclose( png_handle );
	return 1;
}