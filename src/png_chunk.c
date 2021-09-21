#include "png-chunks.h"

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