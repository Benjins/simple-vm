#ifndef VM_H
#define VM_H

#include <string>

using std::string;

#define MAX_STACK 1024

struct VM{

	VM();

	//Execute bytecode, given the start of instruction, and the count
	void Execute(unsigned char* code, int instructionCount);
	void Execute(string code);

protected:
	short stack[MAX_STACK];
	short stackSize;

	void Push(short value);
	short Pop();
};


#endif