#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include <iostream>
#include <fstream>

using std::cout; using std::cin; using std::endl; using std::ifstream; using std::ofstream; using std::getline;

VM::VM(){
	stackSize = 0;
}

void VM::CompileAndLoadCode(const string& fileName){
	ifstream fileIn;
	fileIn.open(fileName);

	if(!fileIn.good()){
		cout << "\nError opening file: " << fileName << endl;
		return;
	}

	byteCodeLoaded.clear();
	funcPointers.clear();

	string code = "";
	while(!fileIn.eof()){
		string line;
		getline(fileIn, line, '\n');
		code = code + line + "\n";
	}

	byteCodeLoaded = NewShuntingYard(NewTokenize(code), *this);

}

void VM::SaveByteCode(const string& fileName){
	ofstream fileOut;
	fileOut.open(fileName, std::ios::out | ofstream::binary);
	if(!fileOut.good()){
		cout << "Trouble opening file " << fileName << " for output.\n";
		return;
	}

	char buffer[4] = {'S', 'V', 'B', '\0'};
	fileOut.write(buffer, 4);

	int version = VERSION_INT;
	fileOut.write((char*)&version, 4);

	int length = byteCodeLoaded.size();
	fileOut.write((char*)&length, 4);

	fileOut.write((char*)byteCodeLoaded.data(), length);

	int numFuncs = funcPointers.size();
	fileOut.write((char*)&numFuncs, 4);

	for(auto iter = funcPointers.begin(); iter != funcPointers.end(); iter++){
		int entryPoint = iter->second;
		fileOut.write((char*)&entryPoint, 4);

		int strLength = iter->first.size() + 1;
		fileOut.write((char*)&strLength, 4);

		fileOut.write(iter->first.c_str(), strLength);
	}

	fileOut.close();
}

void VM::LoadByteCode(const string& fileName){
	ifstream fileIn;
	fileIn.open(fileName, std::ios::in | ofstream::binary);
	if(!fileIn.good()){
		cout << "Trouble opening file " << fileName << " for output.\n";
		return;
	}

	char buffer[4];
	fileIn.read(buffer, 4);

	if(strcmp(buffer, "SVB") != 0){
		cout << "\nWarning: File " << fileName << " may not be an SVB file.\n";
	}

	fileIn.read(buffer, 4);

	if(VERSION_INT < *(int*)buffer){
		cout << "\nWarning: File " << fileName << " is a higher version.  It may not work as intended\n";
	}

	fileIn.read(buffer, 4);
	int length = *(int*)buffer;

	char* codeBuffer = new char[length];
	fileIn.read(codeBuffer, length);
	unsigned char* codeBufferCast = (unsigned char*)codeBuffer;
	byteCodeLoaded.resize(length);
	for(int i = 0; i < length; i++){
		byteCodeLoaded[i] = codeBufferCast[i];
	}

	funcPointers.clear();

	fileIn.read(buffer, 4);
	int numFuncs = *(int*)buffer;

	for(int i = 0; i < numFuncs; i++){
		fileIn.read(buffer, 4);
		int entryPoint = *(int*)buffer;

		fileIn.read(buffer, 4);
		int strLength = *(int*)buffer;

		char* strData = new char[strLength];
		fileIn.read(strData, strLength);
		string funcName = string(strData);

		std::pair<string, int> funcPtr(funcName, entryPoint);
		funcPointers.insert(funcPtr);

		delete[] strData;
	}


	delete[] codeBuffer;

}

int VM::Execute(string funcName){
	return Execute(&byteCodeLoaded[0], byteCodeLoaded.size(), funcName);
}

int VM::Execute(unsigned char* code, int instructionCount, const string& entry){
	auto funcPair = funcPointers.find(entry);
	if(funcPair == funcPointers.end()){
		cout << "\nError: Could not find entry point: " << entry << endl;
		return -1;
	}
	else{
		return Execute(code, instructionCount, funcPair->second);
	}
}

int VM::Execute(unsigned char* code, int instructionCount, int entryPoint){
	stackSize = 0;
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
					registers[b + stackFrame] = a;
				}
				else{
					cout << "Error: Tried to save to a register of value: " << b << endl;
				}
			}break;

			case CALL:{
				//short varCount = Pop();
				//stackFrame = stackFrame + varCount; //Need to modify stack frame passed to func?
				short addr = Pop();
				short varCt = Pop();
				//cout << "Old stack frame before call: " << stackFrame << " after call: " << (stackFrame + varCt) << endl;
				stackFrame = stackFrame + varCt;
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
					registers[a + stackFrame] = b;
				}
			}break;

			case RETURN_FINAL:{
				short a = Pop();
				return a;
			}break;

			default:
				cout << "\nInvalid instruction " << stack[i] << " at instruction " << i << endl;
				break;
		}


	}

	return -1;
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
		return 0;
	}
}
