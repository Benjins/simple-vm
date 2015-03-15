#ifndef VM_H
#define VM_H

#include <string>
#include <unordered_map>
#include <vector>

#define VERSION_INT 100

/*
File formats and specs for svm and svb files.

SVM: Standard text file, only ASCII encoding for now.

SVB: 
 - 4 bytes: "SVB\0"
 - 4 bytes: Version in integer format (100 for 1.0.0, 110 for 1.1.0, 111 for 1.1.1, etc.)
 - 4 bytes: length of code to follow
 - Code in bytecode format, length specified by header

 - 4 bytes: number of functions defined
 - [Number of functions] of FuncPair struct
    - FuncPair struct: 
		- 4 bytes entry point value in code
		- 4 bytes length of string (including null terminator)
		- [Length of string and null terminator] bytes containing the value of the string


*/

using std::string; using std::unordered_map; using std::vector;

#define MAX_STACK 512

#define REGISTER_COUNT 1024

struct VM{

	VM();

	void CompileAndLoadCode(const string& fileName);
	void SaveByteCode(const string& fileName);
	void LoadByteCode(const string& fileName);

	//Execute bytecode, given the start of instruction, and the count
	int Execute(unsigned char* code, int instructionCount, int entryPoint = 0);
	int Execute(unsigned char* code, int instructionCount, const string& entry);
	int Execute(string funcName);

	unordered_map<string, int> funcPointers;
	vector<unsigned char> byteCodeLoaded;

protected:
	short stack[MAX_STACK];
	short stackSize;

	short stackFrame;
	
	short registers[REGISTER_COUNT];

	void Push(short value);
	short Pop();
};


#endif
