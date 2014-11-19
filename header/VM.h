#ifndef VM_H
#define VM_H

#define MAX_STACK 1024

struct VM{

	VM();

	//Execute bytecode, given the start of instruction, and the count
	void Execute(unsigned char* code, int instructionCount);

protected:
	short stack[MAX_STACK];
	short stackSize;

	void Push(short value);
	short Pop();
};


#endif