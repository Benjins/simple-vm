#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include <assert.h>
#include <iostream>

using std::cout; using std::endl;

int main(){

	
	//string code = "PRINT(READ() + 1);";
	string code = "SAVE(0,0);while(LOAD(0) < 10){PRINT(LOAD(0));SAVE(0, LOAD(0) + READ());}PRINT(LOAD(0));";
	vector<string> tokens = NewTokenize(code);
	
	vector<unsigned char> byteCode = NewShuntingYard(tokens);
	
	for(int i = 0; i < byteCode.size(); i++){
		int val = byteCode[i];
		//cout << val << endl;
	}

	VM x;
	x.Execute(&byteCode[0], byteCode.size());
	
	/*
	for(int i = 0; i < byteCode.size(); i++){
		cout << "|" << (int)byteCode[i] << "|\n";
	}
	*/
	

	
    return 0;
}
