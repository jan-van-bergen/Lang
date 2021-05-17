#pragma once
#include <stdbool.h>

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof(arr[0]))

#define NO_RETURN __declspec(noreturn)

void * mem_alloc(size_t size);
void * mem_realloc(void * old_ptr, size_t new_size);
void   mem_free(void * ptr);

void flag_set  (unsigned * flags, unsigned flag);
void flag_unset(unsigned * flags, unsigned flag);
bool flag_is_set(unsigned flags, unsigned flag);

bool is_power_of_two(int x);
int log2(int x);

bool is_escape_char(char c);
char remove_escape(char c);

void swap(void * a, void * b, size_t size_in_bytes);

char const * read_file(char const * filename);

char const * replace_file_extension(char const * filename, char const * file_extension);
