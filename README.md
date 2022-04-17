![Issues](https://img.shields.io/github/issues/michaellrowley/png-chunks) ![Forks](https://img.shields.io/github/forks/michaellrowley/png-chunks) ![Stars](https://img.shields.io/github/stars/michaellrowley/png-chunks) ![License](https://img.shields.io/github/license/michaellrowley/png-chunks) ![Twitter](https://img.shields.io/twitter/url?url=https%3A%2F%2Fgithub.com%2Fmichaellrowley%2Fpng-chunks)

# PNG-Chunks
This is a small ( ~400 lines ) tool I have written to help me learn about [the file structure of PNG files](https://en.wikipedia.org/wiki/Portable_Network_Graphics) by using sample files and enumerating their chunks.

In ``/samples/`` there are a few PNG files that you can use for testing chunk extraction!

## Usage
This program has two primary uses:
### Enumerating/listing chunks within a PNG file
To enumerate the chunks of a PNG file, simply execute ``./png-chunks.out samples/wikipedia.png`` and you should receive an output similar to this:
```None
user$ ./png-chunks.out samples/wikipedia.png

Validated PNG magic bytes.
IHDR
|0|
 |--- Location: 0x8
 |--- Size: 0xD
 |--- CRC32: 0x373EFFB4

... * omitted to save space *

IEND
|4|
 |--- Location: 0x5B082
 |--- Size: 0x0
 |--- CRC32: 0xAE426082


File summary:
	Resolution: 2400 x 2189
	Bit-depth: 8
	Colour-type: 0
```

Where ``IEND``, ``IDAT``, and ``IHDR`` are critical chunks of the PNG file.

### Deleting/erasing chunks from a PNG file
After enumerating the chunks within a PNG file, it becomes possible to delete them using the following command:
```None
./png-chunks sample.png -s eXIF
```
Where ``eXIF`` refers to an ancillary chunk enumerated previously using the chunk-listing command syntax.
If used correctly, removing/wiping chunks should not degrade the quality/integrity of a PNG image as the erasing procedure works by leaving the chunk's structure in place but by overwriting the chunk's name/identifier, contents, and CRC32/checksum with null bytes - meaning that image parsers should be able to identify the erased chunk as a null one that needs to be skipped.
Erasing critical (fully-capitalized) chunks will result in parsing errors when trying to load the image into an image viewer due to the nature of PNG.

### Dumping the bytes of a chunk via its index
Sometimes the bytes of a chunk are important but using a hex editor or terminal command to extract a byte range can be [tedious](https://stackoverflow.com/a/40792605/) at times - using the ``--dump``/``-d`` parameters, it is possible to write the contents of a PNG chunk to a file by simply passing its index as an argument.
```None
./png-chunks sample.png -d 0
```
Where ``0`` refers to the 0ᵗʰ chunk within the PNG file - the output can be found at ``CHNK-00000012`` where ``00000012`` is the hex-formatted offset of the chunk (as can be observed in the 'Location' section of an enumeration).

## Building
Compiling PNG-chunks should be pretty simple, I compiled it on Windows using WSL with [GCC](https://gcc.gnu.org/) and all debugging was done using [GDB](https://www.gnu.org/software/gdb/) so if you have any errors while trying to work with other alternatives, it might be worth trying to use GCC/GDB to resolve your issue. This project is also compatible with [clang](https://clang.llvm.org/) and [lldb](https://lldb.llvm.org/) alongside a variety of operating systems (tested via WSL running Ubuntu, a seperate Ubuntu virtual machine, and a MacOS virtual machine).

If you'd like to compile/build this project for fuzzing, see the [SECURITY.MD](https://github.com/michaellrowley/png-chunks/blob/main/SECURITY.md) document which details how to compile it with ``afl-gcc`` and launch an AFL session using ``afl-fuzz``.

#### Debugging

```bash
gcc src/main.c src/utilities.c src/png_chunk.c -o png-chunks-dbg.out -ggdb -v
```

#### General usage

```bash
gcc src/main.c src/utilities.c src/png_chunk.c -o png-chunks.out -w -Ofast
# OR (same command is executed)
./build.sh
```
