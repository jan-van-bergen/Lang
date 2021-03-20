#pragma once

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

char const * read_file(char const * filename);

char const * replace_file_extension(char const * filename, char const * file_extension);
