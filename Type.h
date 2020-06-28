#pragma once

typedef enum Type_Type {
	TYPE_VOID,

	TYPE_INT,
	TYPE_BOOL,
	TYPE_CHAR,

	TYPE_POINTER
} Type_Type;

typedef struct Type {
	Type_Type type;

	struct Type * ptr; // If type is a TYPE_POINTER, this contains the Type being pointed to
} Type;
