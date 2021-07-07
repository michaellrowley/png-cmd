https://img.shields.io/github/issues/michaellrowley/png-chunks
https://img.shields.io/github/forks/michaellrowley/png-chunks
https://img.shields.io/github/stars/michaellrowley/png-chunks
https://img.shields.io/github/license/michaellrowley/png-chunks
https://img.shields.io/twitter/url?url=https%3A%2F%2Fgithub.com%2Fmichaellrowley%2Fpng-chunks

# PNG-Chunks:
This is a small ( < 300 lines ) tool I have written to help me learn about [the file structure of PNG files](https://en.wikipedia.org/wiki/Portable_Network_Graphics) by using sample files and enumerating their chunks.

In ``/samples/`` there are a few PNG files that you can use for testing chunk extraction!

## Usage:
This program has two primary uses:
### Enumerating/listing chunks within a PNG file:
To enumerate the chunks of a PNG file, simply execute ``./PNG_chunks sample.png`` and you should receive an output similar to this:
```None
root$ ./PNG_chunks samples/firefox.png

Validated PNG magic bytes.
IHDR
|_|
 |
 |--- Location: 0x00000008
 |--- Size: 0x0000000D
 |--- CRC32: 0x000001E2

IDAT
|_|
 |
 |--- Location: 0x00000021
 |--- Size: 0x000631E5
 |--- CRC32: 0x00000135

IEND
|_|
 |
 |--- Location: 0x00063212
 |--- Size: 0x00000000
 |--- CRC32: 0x000001D2


File summary:
  Resolution: 2001 x 2066
  Bit-depth: 8
  Colour-type: 0
```

Where ``IEND``, ``IDAT``, and ``IHDR`` are critical chunks of the PNG file.

### Deleting/erasing chunks from a PNG file:
After enumerating the chunks within a PNG file, it becomes possible to delete them using the following command:
```None
./PNG_chunks sample.png eXIF
```
Where ``eXIF`` refers to an ancillary chunk enumerated previously using the chunk-listing command syntax.
If used correctly, removing/wiping chunks should not degrade the quality/integrity of a PNG image as the erasing procedure works by leaving the chunk's structure in place but by overwriting the chunk's name/identifier, contents, and CRC32/checksum with null bytes - meaning that image parsers should be able to identify the erased chunk as a null one that needs to be skipped.
Erasing critical (fully-capitalized) chunks will result in parsing errors when trying to load the image into an image viewer due to the nature of PNG.


## Building:
Compiling PNG-chunks should be pretty simple, I compiled it on Windows 11 using the Windows Ubuntu subsystem with [GCC](https://gcc.gnu.org/), while I was developing the program I did all of the debugging in [GDB](https://www.gnu.org/software/gdb/) so if you have any errors while trying to work with other alternatives, it might be worth trying to use GCC/GDB to resolve your issue.
#### Debugging:
```bash
gcc main.c -o png-chunks-dbg -ggdb -v
```
#### General usage:
```bash
gcc main.c -o png-chunks -w
```