JSON
==
Streaming reader and writer.

[![standwithukraine](docs/StandWithUkraine.svg)](https://ukrainewar.carrd.co/)

Features:
- `FILE`-based streaming parser
- No internal heap allocations

Usage:
```c
#define PAIV_JSON_IMPLEMENTATION // import implementation
#include "paiv_json.h"
```

- Parser interface: `json_reader_*` functions
- Writer interface: `json_writer_*` functions

Refer to examples for a sample code.

Basic parser structure:
```
json_parser_init
json_reader_open_object
  json_reader_read_object until JsonError_not_found
  json_reader_read_ [string, number*, bool, null]
```
