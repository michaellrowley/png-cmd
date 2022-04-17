#!/bin/bash

gcc src/main.c src/utilities.c src/png_chunk.c -o png-chunks.out -w -Ofast