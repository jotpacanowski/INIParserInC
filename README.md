# Simple INI Parser

Program for reading simplified INI file format, written in C.

## Usage:

- `./parse test.ini` to dump processed file contents.
- `./parse test.ini a.b` to print value of key `b` in section `a`.
- `./parse test.ini expression "a.b + c.d"` to evaluate expression `a.b+c.d`.

Compile command:
`gcc main.c buf_line_reader.c ini_parser.c common.c -o parse  -Wall -Wextra -pedantic`
