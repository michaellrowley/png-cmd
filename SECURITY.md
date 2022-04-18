# Security Policy

## Reporting a Vulnerability

Security-related vulnerabilities can be reported via an
[email](mailto:michaellrowley@protonmail.com) or an issue can be opened
so that I can get the issue patched as soon as possible.

Whilst I think that the number of attack vectors for this project is
limited, I'd like to have any potential vulnerabilities fixed as soon
as possible regardless of how likely they are to be feasible from a
practical perspective however some issues are acknowledged and their
impact accepted as a cost of using the program, therefore the following
assumptions are made:
- The files being submitted are secure from malicious content.
- The path being supplied (and all accompanying arguments) are benign in nature.
- This program is not run as sudo/root/administrator or with any unnecessary privileges.

## Fuzzing this project

During development, AFL (Americal Fuzzy Lop) was used; a tool that was
developed by Google and is now open-source - if you'd like
to fuzz this project using AFL; there is a directory (``/AFL/``) that contains
samples that can be used as a base for fuzzing (these can be found
under ``/AFL/samples``) and an empty output directory (``/AFL/results``).

```bash
# Compile the project
afl-gcc src/main.c src/utilities.c src/png_chunk.c -o png-chunks-afl.out -ggdb

# Start fuzzing
afl-fuzz -i AFL/samples/ -o AFL/results ./png-chunks-afl.out @@ -s eXIF
```

Fuzzing has not been thoroughly tested on MacOS but it does work on Ubuntu (stable).