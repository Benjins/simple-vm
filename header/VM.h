#ifndef VM_H
#define VM_H

#include <string>
#include <map>
#include <map>
#include <vector>

#define VERSION_INT 120

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

## - 4 bytes: number of DLL files to load
## - [Number of DLL's] of:
##     - 4 bytes length of string
##     - [Length of string and null terminator] bytes containing string

 - 4 bytes: number of extern functions used
 - [Number of functions] of extern reference
    - FuncPair struct: 
		- 4 bytes length of string (including null terminator)
		- [Length of string and null terminator] bytes containing the value of the string

*/

enum struct ValueType{
	INT,
	FLOAT,
	VOID
};

struct VMValue{
	union{
		int intValue;
		float floatValue;
	};

	ValueType type;

};

using std::string; using std::map; using std::vector;

struct DLLFile;

#define MAX_STACK 4096

#define REGISTER_COUNT 1024

struct VM{

	VM();

	bool CompileAndLoadCode(const string& fileName);
	void SaveByteCode(const string& fileName);
	bool LoadByteCode(const string& fileName);

	//Execute bytecode, given the start of instruction, and the count
	int Execute(unsigned char* code, int instructionCount, int entryPoint = 0);
	int Execute(unsigned char* code, int instructionCount, const string& entry);
	int Execute(string funcName);

	vector<unsigned char> byteCodeLoaded;
	map<string, int> funcPointers;
	map<string, void*> externFuncPointers;
	map<string, DLLFile> dllFilesLoaded;

protected:
	VMValue stack[MAX_STACK];
	short stackSize;

	short stackFrame;
	
	VMValue registers[REGISTER_COUNT];

	void LoadDLL(const string& fileName);
	void LoadExternFunction(const string& funcName);

	void Push(VMValue value);
	VMValue Pop();
};


#endif
