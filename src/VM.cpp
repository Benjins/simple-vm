#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include "../header/AST.h"
#include "../header/DLLFile.h"
#include <iostream>
#include <cstring>
#include <fstream>

using std::cout; using std::cin; using std::endl; using std::ifstream; using std::ofstream; using std::getline;

VM::VM(){
	stackSize = 0;
}

bool VM::CompileAndLoadCode(const string& fileName, vector<string>* dllsToLoad /* = nullptr*/){
	ifstream fileIn;
	fileIn.open(fileName);

	if(!fileIn.good()){
		cout << "\nError opening file: " << fileName << endl;
		return false;
	}

	byteCodeLoaded.clear();
	funcPointers.clear();

	string code = "";
	while(!fileIn.eof()){
		string line;
		getline(fileIn, line, '\n');
		code = code + line + "\n";
	}

	fileIn.close();

	if(dllsToLoad != nullptr){
		for(auto& dllName : *dllsToLoad){
			LoadDLL(dllName);
		}
	}
	
	AST b;
	
	vector<string> tokens = NewTokenize(code);
	vector<string> shuntedTokens = JustShuntingYard(tokens);

	b.GenerateFromShuntedTokens(shuntedTokens, *this);
	b.GenerateByteCode(*this);

	for(char instr : byteCodeLoaded){
		cout << "|" << (int)instr << "|\n";
	}
	
	return true;
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

	int numDllsUsed = dllFilesLoaded.size();
	fileOut.write((char*)&numDllsUsed, 4);

	for(auto iter = dllFilesLoaded.begin(); iter != dllFilesLoaded.end(); iter++){
		int strLength = iter->first.size() + 1;
		fileOut.write((char*)&strLength, 4);
		fileOut.write(iter->first.c_str(), strLength);
	}

	int numExternFuncs = externFuncNames.size();
	fileOut.write((char*)&numExternFuncs, 4);

	for(auto iter = externFuncNames.begin(); iter != externFuncNames.end(); iter++){
		int strLength = iter->size() + 1;
		fileOut.write((char*)&strLength, 4);
		fileOut.write(iter->c_str(), strLength);
	}

	fileOut.close();
}

bool VM::LoadByteCode(const string& fileName){
	ifstream fileIn;
	fileIn.open(fileName, std::ios::in | ofstream::binary);
	if(!fileIn.good()){
		cout << "Trouble opening file " << fileName << " for output.\n";
		return false;
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

	fileIn.read(buffer, 4);
	int numDlls = *(int*)buffer;

	for(int i = 0; i < numDlls; i++){
		fileIn.read(buffer, 4);
		int strLength = *(int*)buffer;

		char* strData = new char[strLength];
		fileIn.read(strData, strLength);
		string dllName = string(strData);

		auto iterPair = dllFilesLoaded.insert({dllName, DLLFile()});
		iterPair.first->second.OpenFile(dllName);

		delete[] strData;
	}

	fileIn.read(buffer, 4);
	int numExternFunctions = *(int*)buffer;

	for(int i = 0; i < numExternFunctions; i++){
		fileIn.read(buffer, 4);
		int strLength = *(int*)buffer;

		char* strData = new char[strLength];
		fileIn.read(strData, strLength);
		string externFuncName = string(strData);

		LoadExternFunction(externFuncName);

		delete[] strData;
	}


	delete[] codeBuffer;

	return true;
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

	VMValue finalStk;
	finalStk.type = ValueType::INT;
	finalStk.intValue = 0;

	VMValue instrCount;
	instrCount.intValue = instructionCount+1;
	instrCount.type = ValueType::INT;

	Push(finalStk); //final return stackframe, not really important what it's value is.
	Push(instrCount); //on the ifnla return, we jump ot the end of the program, so we terminate.

	for(int i = entryPoint; i < instructionCount; i++){
		unsigned char instruction = code[i];

		//cout << "Exceute order " << (int)instruction << " at instruction " << i << endl;

		switch (instruction){
			case INT_ADD:{
				//cout << "Add\n";
				VMValue a = Pop();
				VMValue b = Pop();
				if(a.type != b.type){
					cout << "\nError: trying to add differently-typed values.\n";
					return -1;
				}
				else if(a.type == ValueType::INT){
					VMValue result;
					result.type = ValueType::INT;
					result.intValue = a.intValue + b.intValue;
					Push(result);
				}
				else if(a.type == ValueType::FLOAT){
					VMValue result;
					result.type = ValueType::FLOAT;
					result.floatValue = a.floatValue + b.floatValue;
					Push(result);
				}
			}break;

			case INT_MUL:{
				VMValue a = Pop();
				VMValue b = Pop();
				if(a.type != b.type){
					cout << "\nError: trying to multiply differently-typed values.\n";
					return -1;
				}
				else if(a.type == ValueType::INT){
					VMValue result;
					result.type = ValueType::INT;
					result.intValue = a.intValue * b.intValue;
					Push(result);
				}
				else if(a.type == ValueType::FLOAT){
					VMValue result;
					result.type = ValueType::FLOAT;
					result.floatValue = a.floatValue * b.floatValue;
					Push(result);
				}
			}break;

			case INT_DIV:{
				VMValue a = Pop();
				VMValue b = Pop();
				if(a.type == ValueType::INT && b.type == ValueType::INT){
					VMValue result;
					result.type = ValueType::INT;
					result.intValue = b.intValue / a.intValue;
					Push(result);
				}
				else if(a.type == ValueType::FLOAT && b.type == ValueType::INT){
					VMValue result;
					result.type = ValueType::FLOAT;
					result.floatValue = b.intValue / a.floatValue;
					Push(result);
				}
				else if(a.type == ValueType::INT && b.type == ValueType::FLOAT){
					VMValue result;
					result.type = ValueType::FLOAT;
					result.floatValue = b.floatValue / a.intValue;
					Push(result);
				}
				else if(a.type == ValueType::FLOAT && b.type == ValueType::FLOAT){
					VMValue result;
					result.type = ValueType::FLOAT;
					result.floatValue = b.floatValue / a.floatValue;
					Push(result);
				}
				else{
					cout << "\nTrying to divide non-numerical types.\n";
				}
			}break;

			case INT_SUB:{
				VMValue a = Pop();
				VMValue b = Pop();
				if(a.type != b.type){
					cout << "\nError: trying to subtract differently-typed values.\n";
					return -1;
				}
				else if(a.type == ValueType::INT){
					VMValue result;
					result.type = ValueType::INT;
					result.intValue = b.intValue - a.intValue;
					Push(result);
				}
				else if(a.type == ValueType::FLOAT){
					VMValue result;
					result.type = ValueType::FLOAT;
					result.floatValue = b.floatValue - a.floatValue;
					Push(result);
				}
			}break;

			case INT_LIT:{
				//cout << "Literal\n";
				i++; //Get the next symbol in the byte code
				int a = code[i];
				VMValue lit;
				lit.type = ValueType::INT;
				lit.intValue = a;
				Push(lit);
			}break;

			case INT_DLIT:{
				i++; //Get the next symbol in the byte code
				short a = code[i];
				i++;
				short b = code[i];
				short c = (a << 8) + b;

				VMValue lit;
				lit.type = ValueType::INT;
				lit.intValue = c;
				Push(lit);
			}break;

			case PRINT:{
				//cout << "PRINT\n";
				VMValue a = Pop();
				if(a.type == ValueType::INT){
					cout << a.intValue << endl;
				}
				else if(a.type == ValueType::FLOAT){
					cout << a.floatValue << endl;
				}
			}break;

			case READ:{
				VMValue a;
				cin >> a.intValue;
				a.type = ValueType::INT;
				Push(a);
			}break;
			
			case READF:{
				VMValue a;
				cin >> a.floatValue;
				a.type = ValueType::FLOAT;
				Push(a);
			}break;
			
			case L_THAN:{
				VMValue a = Pop();
				VMValue b = Pop();
				VMValue res;
				res.type = ValueType::INT;
				if(a.type != b.type){
					cout << "\nError: trying to compare(<) differently-typed values.\n";
					return -1;
				}
				else if(a.type == ValueType::INT){
					res.intValue = b.intValue < a.intValue? 1 : 0;
					Push(res);
				}
				else if(a.type == ValueType::FLOAT){
					res.intValue = b.floatValue < a.floatValue? 1 : 0;
					Push(res);
				}
			}break;
			
			case G_THAN:{
				VMValue a = Pop();
				VMValue b = Pop();
				VMValue res;
				res.type = ValueType::INT;
				if(a.type == ValueType::INT && b.type == ValueType::INT){
					res.intValue = b.intValue > a.intValue? 1 : 0;
					Push(res);
				}
				else if(a.type == ValueType::INT && b.type == ValueType::FLOAT){
					res.intValue = b.floatValue > a.intValue? 1 : 0;
					Push(res);
				}
				else if(a.type == ValueType::FLOAT && b.type == ValueType::INT){
					res.intValue = b.intValue > a.floatValue? 1 : 0;
					Push(res);
				}
				else if(a.type == ValueType::FLOAT && b.type == ValueType::FLOAT){
					res.intValue = b.floatValue > a.floatValue? 1 : 0;
					Push(res);
				}
			}break;
			
			case BOOL_AND:{
				VMValue a = Pop();
				VMValue b = Pop();
				if(a.type != ValueType::INT || b.type != ValueType::INT){
					cout << "\nError: Trying to and non-boolean values.\n";
					return -1;
				}

				VMValue res;
				res.type = ValueType::INT;
				res.intValue = (a.intValue > 0 && b.intValue > 0)? 1 : 0;
				Push(res);
			}break;
			
			case BOOL_OR:{
				VMValue a = Pop();
				VMValue b = Pop();
				if(a.type != ValueType::INT || b.type != ValueType::INT){
					cout << "\nError: Trying to and non-boolean values.\n";
					return -1;
				}

				VMValue res;
				res.type = ValueType::INT;
				res.intValue = (a.intValue > 0 || b.intValue > 0)? 1 : 0;
				Push(res);
			}break;
			
			case COMPARE:{
				VMValue a = Pop();
				VMValue b = Pop();
				VMValue res;
				res.type = ValueType::INT;
				if(a.type != b.type){
					cout << "\nError: Trying to compare differently-typed values.\n";
					return -1;
				}
				else if(a.type == ValueType::INT){
					res.intValue = (a.intValue == b.intValue)? 1 : 0;
				}
				else if(a.type == ValueType::FLOAT){
					res.intValue = (a.floatValue == b.floatValue)? 1 : 0;
				}
				
				Push(res);
			}break;
			
			case BRANCH:{
				VMValue a = Pop();
				VMValue b = Pop();
				if(a.type != ValueType::INT || b.type != ValueType::INT){
					cout << "\nError: Triyng to branch with on-integral type.\n";
					return -1;
				}
				if(b.intValue == 0){
					i = a.intValue-1;
				}
			}break;

			case LOAD_REG:{
				//cout << "LOAD\n";
				VMValue a = Pop();
				if(a.type == ValueType::INT && a.intValue >= 0 && a.intValue < REGISTER_COUNT){
					Push(registers[a.intValue + stackFrame]);
				}
				else{
					cout << "Error: Tried to load a register of value: " << a.intValue << endl;
					return -1;
				}
			}break;

			case SAVE_REG:{
				//cout << "SAVE\n";
				VMValue a = Pop();
				VMValue b = Pop();

				if(b.type == ValueType::INT && b.intValue >= 0 && b.intValue < REGISTER_COUNT){
					registers[b.intValue + stackFrame] = a;
				}
				else{
					cout << "Error: Tried to save to a register of value: " << b.intValue << endl;
					return -1;
				}
			}break;

			case CALL:{
				//short varCount = Pop();
				//stackFrame = stackFrame + varCount; //Need to modify stack frame passed to func?
				VMValue addr = Pop();
				VMValue varCt = Pop();
				if(addr.type != ValueType::INT || varCt.type != ValueType::INT){
					cout << "\nError: Tried to execute CALL with non-integral values.\n";
					return -1;
				}
				//cout << "Old stack frame before call: " << stackFrame << " after call: " << (stackFrame + varCt) << endl;
				stackFrame = stackFrame + varCt.intValue;
				i = addr.intValue-1;
			}break;

			case RETURN:{
				VMValue value = Pop();
				VMValue retAddr = Pop();
				VMValue prevStackFrame = Pop();
				Push(value);

				if(retAddr.type != ValueType::INT || prevStackFrame.type != ValueType::INT){
					cout << "\nError: Tried to execute RETURN with non-integral values.\n";
					return -1;
				}

				i = retAddr.intValue-1;
				//cout << "currentStackFrame: " << stackFrame << "  prevStackFrame: " << prevStackFrame << endl;
				stackFrame = prevStackFrame.intValue;
			}break;

			case STK_FRAME:{
				VMValue val;
				val.type = ValueType::INT;
				val.intValue = stackFrame;
				Push(val);
			}break;

			case PARAM:{
				VMValue a = Pop();
				VMValue b = Pop();
				if(a.type != ValueType::INT){
					cout << "\nError: Tried to call PARAM with non-integral index.\n";
					return -1;
				}
				if(a.intValue >= 0 && a.intValue < REGISTER_COUNT){
					registers[a.intValue + stackFrame] = b;
				}
			}break;

			case RETURN_FINAL:{
				VMValue a = Pop();
				return a.intValue;
			}break;

			case FLT_LIT:{
				i++;
				VMValue lit;
				lit.type = ValueType::FLOAT;
				lit.floatValue = *(float*)(code + i);
				Push(lit);
				i += 3;
			}break;
			
			case INT_TO_FLT:{
				VMValue i = Pop();
				if(i.type != ValueType::INT){
					cout << "\nError: Converting non-int from int to float.\n";
				}
				
				VMValue f;
				f.type = ValueType::FLOAT;
				f.floatValue = i.intValue;
				Push(f);
			}break;

			case EXTERN_CALL:{
				VMValue addrIdx = Pop();
				if(addrIdx.type != ValueType::INT){
					cout << "\nError: Tried to execute EXTERN_CALL with non-integral value.\n";
					return -1;
				}

				void* addr = externFuncPointers[addrIdx.intValue];
				VMValue (*externFunc)(VMValue) = (VMValue (*)(VMValue))addr;
				VMValue retVal = (*externFunc)(Pop());
				Push(retVal);
			}break;

			default:{
				cout << "\nInvalid instruction " << int(code[i]) << " at instruction " << i << endl;
			}break;
		}


	}

	if(stackSize > 0){
		return Pop().intValue;
	}
	else{
		return -1;
	}
}

void VM::LoadDLL(const string& fileName){
	auto iterPair = dllFilesLoaded.insert({fileName, DLLFile()});
	iterPair.first->second.OpenFile(fileName);
}

void VM::LoadExternFunction(const string& funcName){
	for(auto& file : dllFilesLoaded){
		void* funcPtr = file.second.GetFunction(funcName);
		if(funcPtr != nullptr){
			externFuncNames.push_back(funcName);
			externFuncPointers.push_back(funcPtr);
			return;
		}
	}

	cout << "\nError: Could not find definition for function '" << funcName << "' in loaded dynamic libraries.\n";
}

void VM::Push(VMValue value){
	if(stackSize < MAX_STACK){
		stackSize++;
		stack[stackSize] = value;
	}
	else{
		cout << "\nError: Stack Overflow on VM.\n";
	}
}

VMValue VM::Pop(){
	if(stackSize > 0){
		VMValue a = stack[stackSize];
		stackSize--;
		return a;
	}
	else{
		cout << "\nError: Stack Underflow on VM.\n";
		VMValue x;
		x.intValue = 0;
		x.type = ValueType::INT;
		return x;
	}
}
