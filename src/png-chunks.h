#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <sys/stat.h>

#define BYTE char
#define BOOL bool
#define TRUE true
#define FALSE false
#define nullptr 0x0
#define NULL 0

typedef struct png_chunk {
	fpos_t location; // Offset in file.
	int32_t size; // Not including name or checksum.
	unsigned char name [ 5 ]; // Don't print without '%.4s' to avoid over-reads/ID.
	BYTE* data; // Allocated using calloc so free this when you're finished.
	uint32_t checksum; // CRC32.
	uint32_t real_checksum; // A CRC32 calculated by png-chunks.
} chunk, *pchunk;

typedef struct ihdr_data {
// | <--4--> | <--4--> | <---1---> | <----1----> | <----1----> | <---1---> | <---1---> |
// | width   | height  | bit-depth | colour-type | compression |   filter  | interlace |
//      4    +    4    +     1     +      1      +      1      +     1     +     1     = 13 bytes.
	int32_t width;
	int32_t height;
	BYTE bit_depth;
	BYTE colour_type;
	BYTE compression_type;
	BYTE filter_type;
	BYTE interlace_type;
} ihdr_data, *pihdr_data;

BOOL read_bytes( FILE* file_handle, size_t len, BYTE* buffer );