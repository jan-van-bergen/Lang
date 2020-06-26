#pragma once
#include <string.h>
#include <stdlib.h>

inline char const * read_file(char const * filename) {
	FILE * f;
	fopen_s(&f, filename, "rb");

	if (f == NULL) {
		printf("Unable to open file %s!\n", filename);
		abort();
	}

	fseek(f, 0, SEEK_END);
	int file_length = ftell(f);
	fseek(f, 0, SEEK_SET);

	char * string = malloc(file_length + 1);
	fread(string, 1, file_length, f);
	string[file_length] = '\0';

	fclose(f);

	return string;
}


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
