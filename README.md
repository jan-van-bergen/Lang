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
// Link against Windows API
extern GetStdHandle(std_handle: int) -> void *;
extern WriteFile(handle: void *, buffer: void *, bytes_to_write: int, bytes_written: int *, overlapped: u64) -> bool;

// Write to stdout
let STD_OUTPUT_HANDLE := -11;

func print(str: char *, str_len: int) -> void {
	let std_handle := GetStdHandle(STD_OUTPUT_HANDLE);

	let bytes_written: int;
	WriteFile(std_handle, str, str_len, &bytes_written, 0);
}

// Entry point
func main() -> int {
	print("Hello world!", 12);
	
	return 0;
}
```

## Dependencies

- <b>NASM</b> (https://www.nasm.us/): used as assembler and should be added to the system `Path` variable.
- <b>Microsoft Linker</b> (included with Visual Studio) is used for linking.
