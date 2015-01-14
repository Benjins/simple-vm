#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include <assert.h>
#include <iostream>

using std::cout; using std::endl;

int main(){

	string code = "PRINT(READ()+READ());PRINT(READ());";
	vector<string> tokens = NewTokenize(code);
	
	vector<unsigned char> byteCode = NewShuntingYard(tokens);
	
	VM x;
	x.Execute(&byteCode[0], byteCode.size());
	
	/*
	for(auto iter = tokens.begin(); iter != tokens.end(); iter++){
		cout << "|" << *iter << "|\n";
	}
	*/
	
    return 0;
}
