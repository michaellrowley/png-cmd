#include "png-cmd.h"

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
	ihdr_output.compression_type = data[ 10 ];

	// 1b + [11b] = [12b]
	// Filter-type:
	ihdr_output.filter_type = data[ 11 ];

	// 1b + [12b] = [13b] -> TOTAL
	// Interlace-type:
	ihdr_output.interlace_type = data[ 12 ];

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

		printf( "%s\n|%u|\n |--- Location:\t0x%X\n |--- Size:\t0x%X\n |--- CRC32:\t0x%X\n\n",
			iterative_chunk.name, iterative_chunk_index,
			(unsigned int)FPOS_GETVAL( iterative_chunk.location ),
			iterative_chunk.size, iterative_chunk.checksum );

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
	printf( "\nFile summary:\n\tResolution:\t%d x %d\n\tBit-depth:\t%d\n\tColour-type:\t%d - %s\n\n",
		ihdr.width, ihdr.height, ihdr.bit_depth, ihdr.colour_type, (ihdr.colour_type > 6 ) ? "Invalid"
		: colour_type_descriptions[ihdr.colour_type] );
	return TRUE;
}

int main( int argc, char** argv ) {

	if ( argc <= 1 ) {
		printf( "Invalid amount of arguments provided.\n" );

		printf( "Usage:\n\t%s [file.png] | List the chunks in 'file.png'.\n\t%s [file.png] -s [index] | Erases chunk at index 'index' in 'file.png'.\n",
			argv[ 0 ], argv[ 0 ] );

		return -1;
	}

	// Check that the provided file is a 'regular' file
	// (not a symbolic link, directory, device, or something
	// else).
	struct stat png_stat;
	if ( &png_stat == nullptr || stat( argv[ 1 ], &png_stat ) == -1 ||
		!S_ISREG( png_stat.st_mode ) ) {
		printf( "Unable to validate the filetype of file '%s'.\n", argv[1] );
		return 1;
	}

	const FILE* png_handle = fopen( argv[ 1 ], "r+" );
	if ( !png_handle ) {
		printf( "Unable to open a handle to file '%s'.\n", argv[ 1 ] );
		return 1;
	}

	// The first bytes of a PNG should be 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x1A, 0x0A
	BYTE magic_bytes_buffer[ 8 ];
	if ( !read_bytes( png_handle, 8, magic_bytes_buffer ) ) {
		fclose( png_handle );
		printf( "Unable to read the first seven bytes of '%s'.\n", argv[ 1 ] );
		return 1;
	}
	
	// I'm not sure why but the last two bytes need to be reversed in order (0x0A <-> 0x1A)
	// but it's probably something to do with endianess
	if ( strncmp( magic_bytes_buffer, "\x89\x50\x4E\x47\x0D\x0A\x1A",
		(long unsigned int)7 ) != 0 ) {
		printf( "Unable to verify that the provided file ('%s') is a PNG.\n", argv[1] );
		fclose( png_handle );
		return 1;
	}

	if ( argc == 2 ) {
		// ./program image.png
		list_ancillary_full( png_handle );
	}
	else {
		if ( argc == 4 ) {
			const char* operation = argv[ 2 ]; // type of operation
			const char* argument = argv[ 3 ];

			if ( strcmp( operation, "--strip" ) == 0 ||
				 strcmp( operation, "-s" ) == 0 ) {
				if ( is_string_number( argument, strnlen( argument,
					 get_number_length( LONG_MAX ) ) ) ) {
					// ./program image.png --strip 0
					char* const parse_end = nullptr;
					long target_chunk_index = strtol( argument, &parse_end, 10 );
					if ( parse_end == argument ||
						target_chunk_index > INT_MAX || target_chunk_index < INT_MIN ) {
						printf( "Chunk index unable to be parsed.\n" );
						return -1;
					}
					strip_chunk( png_handle, nullptr, (int)target_chunk_index );
				}
				else {
					// ./program image.png --strip IHDR
					strip_chunk( png_handle, argv[ 3 ], -1 );
				}
			}
			else if ( strcmp( operation, "--dump" ) == 0 ||
					  strcmp( operation, "-d" ) == 0 ) {
				// ./program image.png --dump 0
				char* const parse_end = nullptr;
				unsigned long chunk_index = strtoll( argument, &parse_end, 10 );
				if ( parse_end == argument ) {
					printf("Chunk index unable to be parsed.");
					fclose( png_handle );
					return 1;
				}
				if ( !dump_chunk( png_handle, chunk_index ) ) {
					printf( "Unable to dump chunk." );
					fclose( png_handle );
					return 1;
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
