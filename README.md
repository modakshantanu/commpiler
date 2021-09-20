# commpiler

A compiler for a C-like language written in C.

## The language

Generally basic C syntax works. Only while loops are supported, and only int and bool datatypes are supported. Unsupported features include short operators like += and ++, floating point datatypes, pointers/references and arrays.  Some slightly hard to parse syntax is also not implemented (like multiple declaration in 1 line, conditional operator). Recursion works. To read in input, use the function `int read(int address)` that returns the integer stored in the memory address reserved for input. `void write(int address, int value)` writes the result to the output memory location, which is later shown to the user after program execution completes.

## Compiling

Compile main.cpp for the compiler portion, runtime/runtime.cpp for the runtime.

`g++ main.cpp -o main.exe`
`g++ runtime/runtime.cpp -o runtime.exe`

## Running

`./main.exe <input file> <output file>`

Will take in the program file from the input path and output an assembly file to the output path. 

`./runtime.exe <assembly file>`

to run the code in the 'emulator'. The emulator will let you write to the input address space before strating the program and will show you the output memory space after program execution completes.

## The instruction set

The instruction set is based off of ARM, but with some simplifications. Importantly, division and modulo are separate instructions to simplify the code generation. Also, all memory addresses store 32-bit integers, so there is no notion of word alignment vs byte alignment. PC increments by +1 instead of +4.

The flags C and V are not fully implemented as they were not needed in code generation. The syntax for conditional instructions is also a bit different.

## Example

There is no proper example folder, but abc.txt shows a program that finds the first prime above n. out.txt shows its assembly. 