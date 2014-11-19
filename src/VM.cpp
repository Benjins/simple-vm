#include "../header/VM.h"
#include "../header/Instruction.h"
#include <iostream>

using std::cout; using std::cin; using std::endl;

VM::VM(){
	stackSize = 0;
}

void VM::Execute(unsigned char* code, int instructionCount){

	for(int i = 0; i < instructionCount; i++){
		unsigned char instruction = code[i];

		switch (instruction){
			case Instruction::INT_ADD:{
				short a = Pop();
				short b = Pop();
				Push(a + b);
			}break;

			case Instruction::INT_MUL:{
				short a = Pop();
				short b = Pop();
				Push(a * b);
			}break;

			case Instruction::INT_INV:{
				short a = Pop();
				Push(a * -1);
			}break;

			case Instruction::INT_LIT:{
				i++; //Get the next symbol in the byte code
				short a = code[i];
				Push(a);
			}break;

			case Instruction::INT_DLIT:{
				i++; //Get the next symbol in the byte code
				short a = code[i];
				i++;
				short b = code[i];
				short c = (a << 8) + b;
				Push(c);
			}break;

			case Instruction::PRINT:{
				short a = Pop();
				cout << a << endl;
			}break;

			case Instruction::READ:{
				short a;
				cin >> a;
				Push(a);
			}break;

			default:
				cout << "\nInvalid instruction at instruction " << i << endl;
				break;
		}
	}
}

void VM::Push(short value){
	if(stackSize < MAX_STACK){
		stackSize++;
		stack[stackSize] = value;
	}
	else{
		cout << "\nError: Stack Overflow on VM.\n";
	}
}

short VM::Pop(){
	if(stackSize > 0){
		short a = stack[stackSize];
		stackSize--;
		return a;
	}
	else{
		cout << "\nError: Stack Underflow on VM.\n";
	}
}