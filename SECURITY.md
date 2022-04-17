# Security Policy

## Reporting a Vulnerability

There aren't many attack vectors for this project aside from malicious
images but if you find a security vulnerability with png-chunks, please
[send me an email](mailto:michaellrowley@protonmail.com) or open an issue
(both contact methods should have similar patch times) so that I can get the
issue patched/fixed as soon as possible.

## Fuzzing this project

During development/testing, I've been using AFL (Americal Fuzzy Lop) - a tool
that was originally developed by Google and is now open-source - if you'd like
to fuzz this project using AFL; there is a directory (``/AFL/``) that has a
couple of small (95B and 8.71KB) samples that can be used (these can be found
under ``/AFL/samples``) and an empty output directory (``/AFL/results``).

```bash
# Compile the project
afl-gcc src/main.c src/utilities.c src/png_chunk.c -o png-chunks-afl.out -ggdb -v
# Start fuzzing
afl-fuzz -i AFL/samples/ -o AFL/results ./png-chunks-afl.out @@ -s eXIF
```

I'd recommend adjusting the launch arguments (specifically, the ``-s eXIF``
part) appropriately.