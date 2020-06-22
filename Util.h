#pragma once
#include <string.h>
#include <stdlib.h>

inline char const * replace_file_extension(char const * filename, char const * file_extension) {
	char const * extension_curr = strchr(filename, '.');
	char const * extension_last = NULL;

	while (extension_curr) {
		extension_last = extension_curr;
		extension_curr = strchr(extension_curr + 1, '.');
	}

	int extension_index;
	if (extension_last) {
		extension_index = extension_last - filename;
	} else {
		extension_index = strlen(filename);
	}

	int file_extension_len = strlen(file_extension);
	int    str_size = extension_index + file_extension_len + 2;
	char * str = malloc(str_size);

	memcpy_s(str,                       str_size,                       filename,       extension_index);
	memcpy_s(str + extension_index + 1, str_size - extension_index - 1, file_extension, file_extension_len);
	str[extension_index] = '.';
	str[str_size - 1]    = '\0';

	return str;
}
