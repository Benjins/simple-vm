#include "../header/VM.h"
#include "../header/Instruction.h"
#include <iostream>

using std::cout; using std::cin; using std::endl;

VM::VM(){
	stackSize = 0;
}

void VM::Execute(string code){
	int count = code.size();

	unsigned char* byteCode = new unsigned char[count];

	for(int i = 0; i < count; i++){
		byteCode[i] = (unsigned char)code[i];
	}

	Execute(byteCode, count);

	delete byteCode;
}

void VM::Execute(unsigned char* code, int instructionCount){

	for(int i = 0; i < instructionCount; i++){
		unsigned char instruction = code[i];

		switch (instruction){
			case INT_ADD:{
				short a = Pop();
				short b = Pop();
				Push(a + b);
			}break;

			case INT_MUL:{
				short a = Pop();
				short b = Pop();
				Push(a * b);
			}break;

			case INT_INV:{
				short a = Pop();
				Push(a * -1);
			}break;

			case INT_LIT:{
				i++; //Get the next symbol in the byte code
				short a = code[i];
				Push(a);
			}break;

			case INT_DLIT:{
				i++; //Get the next symbol in the byte code
				short a = code[i];
				i++;
				short b = code[i];
				short c = (a << 8) + b;
				Push(c);
			}break;

			case PRINT:{
				short a = Pop();
				cout << a << endl;
			}break;

			case READ:{
				short a;
				cin >> a;
				Push(a);
			}break;
			
			case L_THAN:{
				short a = Pop();
				short b = Pop();
				short res = a > b? 1 : 0;
				Push(res);
			}break;
			
			case G_THAN:{
				short a = Pop();
				short b = Pop();
				short res = a < b? 1 : 0;
				Push(res);
			}break;
			
			case BOOL_AND:{
				short a = Pop();
				short b = Pop();
				short res = (a > 0 && b > 0)? 1 : 0;
				Push(res);
			}break;
			
			case BOOL_OR:{
				short a = Pop();
				short b = Pop();
				short res = (a > 0 || b > 0)? 1 : 0;
				Push(res);
			}break;
			
			case COMPARE:{
				short a = Pop();
				short b = Pop();
				short res = (a == b)? 1 : 0;
				Push(res);
			}break;
			
			case BRANCH:{
				short a = Pop();
				short b = Pop();
				if(b == 0){
					i = a-1;
				}
			}break;

			default:
				cout << "\nInvalid instruction at instruction " << i << endl;
				break;
		}
	}
}

/*,
	ASSIGN = 12,
	COMPARE = 13,
	BRANCH = 14
*/

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
