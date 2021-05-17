#include "Util.h"

#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include "Error.h"

void * mem_alloc(size_t size) {
	void * ptr = malloc(size);

	if (!ptr) {
		puts("ERROR: malloc failed!");
		error(ERROR_INTERNAL);
	}

	return ptr;
}

void * mem_realloc(void * old_ptr, size_t new_size) {
	void * ptr = realloc(old_ptr, new_size);

	if (!ptr) {
		puts("ERROR: realloc failed!");
		error(ERROR_INTERNAL);
	}

	return ptr;
}

void mem_free(void * ptr) {
	free(ptr);
}


void flag_set(unsigned * flags, unsigned flag) {
	*flags |= flag;
}

void flag_unset(unsigned * flags, unsigned flag) {
	*flags &= ~flag;
}

bool flag_is_set(unsigned flags, unsigned flag) {
	return flags & flag;
}


bool is_power_of_two(int x) {
	return x > 0 && ((x & (x - 1)) == 0);
}

int log2(int x) {
	int result = 0;
	while (x > 1) {
		x /= 2;
		result++;
	}
	return result;
}


bool is_escape_char(char c) {
	switch (c) {
		case '\a':
		case '\b':
		case '\f':
		case '\n':
		case '\r':
		case '\t':
		case '\'':
		case '\"':
		case '\0':
		case '\\': return true;

		default: return false;
	}
}

char remove_escape(char c) {
	switch (c) {
		case '\a': return 'a';
		case '\b': return 'b';
		case '\f': return 'f';
		case '\n': return 'n';
		case '\r': return 'r';
		case '\t': return 't';
		case '\0': return '0';

		default: return c;
	}
}


void swap(void * a, void * b, size_t size_in_bytes) {
	if (a == b) return;

	char tmp[1024];
	if (size_in_bytes > sizeof(tmp)) {
		error(ERROR_INTERNAL);
	}

	memcpy(tmp, a, size_in_bytes);
	memcpy(a, b,   size_in_bytes);
	memcpy(b, tmp, size_in_bytes);
}


char const * read_file(char const * filename) {
	FILE * f;
	fopen_s(&f, filename, "rb");

	if (f == NULL) {
		printf("ERROR: Unable to open file %s!\n", filename);
		error(ERROR_INTERNAL);
	}

	fseek(f, 0, SEEK_END);
	int file_length = ftell(f);
	fseek(f, 0, SEEK_SET);

	char * string = mem_alloc(file_length + 1);
	fread(string, 1, file_length, f);
	string[file_length] = '\0';

	fclose(f);

	return string;
}

char const * replace_file_extension(char const * filename, char const * file_extension) {
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
	char * str = mem_alloc(str_size);

	memcpy_s(str,                       str_size,                       filename,       extension_index);
	memcpy_s(str + extension_index + 1, str_size - extension_index - 1, file_extension, file_extension_len);
	str[extension_index] = '.';
	str[str_size - 1]    = '\0';

	return str;
}
