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

void VM::Execute(unsigned char* code, int instructionCount, const string& entry){
	auto funcPair = funcPointers.find(entry);
	if(funcPair == funcPointers.end()){
		cout << "\nError: Could not find entry point: " << entry << endl;
	}
	else{
		Execute(code, instructionCount, funcPair->second);
	}
}

void VM::Execute(unsigned char* code, int instructionCount, int entryPoint){
	stackFrame = 0;

	for(int i = entryPoint; i < instructionCount; i++){
		unsigned char instruction = code[i];

		//cout << "Exceute order " << (int)instruction << " at instruction " << i << endl;

		switch (instruction){
			case INT_ADD:{
				//cout << "Add\n";
				short a = Pop();
				short b = Pop();
				Push(a + b);
			}break;

			case INT_MUL:{
				short a = Pop();
				short b = Pop();
				Push(a * b);
			}break;

			case INT_DIV:{
				short a = Pop();
				short b = Pop();
				Push(b / a);
			}break;

			case INT_SUB:{
				short a = Pop();
				short b = Pop();
				Push(b - a);
			}break;

			case INT_LIT:{
				//cout << "Literal\n";
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
				//cout << "PRINT\n";
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
				//cout << "BRANCH\n";
				short a = Pop();
				short b = Pop();
				if(b == 0){
					i = a-1;
				}
			}break;

			case LOAD_REG:{
				//cout << "LOAD\n";
				short a = Pop();
				if(a >= 0 && a < REGISTER_COUNT){
					Push(registers[a + stackFrame]);
				}
				else{
					cout << "Error: Tried to load a register of value: " << a << endl;
					Push(0);
				}
			}break;

			case SAVE_REG:{
				//cout << "SAVE\n";
				short a = Pop();
				short b = Pop();

				if(b >= 0 && b < REGISTER_COUNT){
					registers[b] = a + stackFrame;
				}
				else{
					cout << "Error: Tried to save to a register of value: " << b << endl;
				}
			}break;

			case CALL:{
				//short varCount = Pop();
				//stackFrame = stackFrame + varCount; //Need to modify stack frame passed to func?
				short addr = Pop();
				i = addr-1;
			}break;

			case RETURN:{
				short value = Pop();
				short retAddr = Pop();
				short prevStackFrame = Pop();
				Push(value);

				i = retAddr-1;
				//cout << "currentStackFrame: " << stackFrame << "  prevStackFrame: " << prevStackFrame << endl;
				stackFrame = prevStackFrame;
			}break;

			case STK_FRAME:{
				Push(stackFrame);
			}break;

			case PARAM:{
				short a = Pop();
				short b = Pop();

				if(a >= 0 && a < REGISTER_COUNT){
					registers[a] = b + stackFrame;
				}
			}break;

			default:
				cout << "\nInvalid instruction " << stack[i] << " at instruction " << i << endl;
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
