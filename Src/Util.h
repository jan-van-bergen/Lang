#pragma once

#include <stdbool.h>

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

bool is_escape_char(char c);
char remove_escape(char c);

char const * read_file(char const * filename);

char const * replace_file_extension(char const * filename, char const * file_extension);
