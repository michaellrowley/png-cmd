#ifndef BYTE
#define BYTE char
#endif
#ifndef BOOL
#define BOOL bool
#endif
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif
#ifndef nullptr
#define nullptr 0x0
#endif
#ifndef NULL
#define NULL 0
#endif

typedef struct png_chunk {
	fpos_t location; // Offset in file.
	int32_t size; // Not including name or checksum.
	unsigned char name [ 5 ]; // Don't print without '%.4s' to avoid over-reads/ID.
	BYTE* data; // Allocated using calloc so free this when you're finished.
	uint32_t checksum; // CRC32.
	uint32_t real_checksum; // A CRC32 calculated by png-chunks.
} chunk, *pchunk;
