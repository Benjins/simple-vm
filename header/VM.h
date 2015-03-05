#ifndef VM_H
#define VM_H

#include <string>
#include <unordered_map>

using std::string; using std::unordered_map;

#define MAX_STACK 1024

#define REGISTER_COUNT 16

struct VM{

	VM();

	//Execute bytecode, given the start of instruction, and the count
	void Execute(unsigned char* code, int instructionCount, int entryPoint = 0);
	void Execute(unsigned char* code, int instructionCount, const string& entry);
	void Execute(string code);

	unordered_map<string, int> funcPointers;
protected:
	short stack[MAX_STACK];
	short stackSize;

	

	short stackFrame;
	
	short registers[REGISTER_COUNT];

	void Push(short value);
	short Pop();
};


#endif
