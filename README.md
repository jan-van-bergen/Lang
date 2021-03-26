# Lang
Compiler in C for a simple programming language that compiles to x64 assembly (NASM syntax).

## Features

- [x] Functions
- [x] Scopes
- [x] Pointers
- [x] Arrays
- [x] Structs
- [x] Type inference 

## Hello World Example
```C
extern puts(str: char *);
extern strlen(str: char *) -> int;

func main() -> int {
	str := "Hello world!";	
	puts(str);

	return strlen(str);
}

```

## Dependencies

- <b>NASM</b> (https://www.nasm.us/): used as assembler and should be added to the system `Path` variable.
- <b>Microsoft Linker</b> (included with Visual Studio) is used for linking.
