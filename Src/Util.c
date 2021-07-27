#include "Util.h"

#include <string.h>

#include <stdio.h>
#include <stdlib.h>

#include "Error.h"

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void * mem_alloc(size_t size) {
	void * ptr = malloc(size);

	if (!ptr) {
		error(ERROR_INTERNAL, "malloc failed!");
	}

	return ptr;
}

void * mem_realloc(void * old_ptr, size_t new_size) {
	void * ptr = realloc(old_ptr, new_size);

	if (!ptr) {
		error(ERROR_INTERNAL, "realloc failed!");
	}

	return ptr;
}

void mem_free(void * ptr) {
	free(ptr);
}


void flag_set(uint32_t * flags, uint32_t flag) {
	*flags |= flag;
}

void flag_unset(uint32_t * flags, uint32_t flag) {
	*flags &= ~flag;
}

bool flag_is_set(uint32_t flags, uint32_t flag) {
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
		error_internal();
	}

	memcpy(tmp, a, size_in_bytes);
	memcpy(a, b,   size_in_bytes);
	memcpy(b, tmp, size_in_bytes);
}


char const * read_file(char const * filename) {
	FILE * f;
	fopen_s(&f, filename, "rb");

	if (f == NULL) {
		error(ERROR_INTERNAL, "Unable to open file %s!\n", filename);
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

bool find_windows_sdk_lib_folder(char * path, int path_len) {
	const char windows_sdk_base[] = "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\*";

	WIN32_FIND_DATA find_data;
	HANDLE handle = FindFirstFile(windows_sdk_base, &find_data);
	
	if (handle == INVALID_HANDLE_VALUE) return false;

	char const * best_directory = NULL;
	uint64_t     best_number = 0;

	do {
		if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			uint64_t number = 0;

			char const * c = find_data.cFileName;
			while (*c) {
				if (isdigit(*c)) {
					number += *c - '0';
					number *= 10;
				}
				c++;
			}

			if (number > best_number) {
				if (best_directory) free(best_directory);
				best_directory = strdup(find_data.cFileName);
				best_number = number;
			}
		}
	} while (FindNextFile(handle, &find_data) != NULL);

	FindClose(handle);

	if (best_number == 0) {
		error(ERROR_INTERNAL, "Unable to locate Windows SDK!\n");
	}

	sprintf_s(path, path_len, "%.*s%s\\um\\x64", (int)(sizeof(windows_sdk_base) - 2), windows_sdk_base, best_directory);

	free(best_directory);
}
