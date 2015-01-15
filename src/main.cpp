#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include <assert.h>
#include <iostream>

using std::cout; using std::endl;

int main(){

	string code = "if(2 = 2 & 1 = 1 & 1 < 2 & 4 > 3){PRINT(2);}PRINT(0);";
	vector<string> tokens = NewTokenize(code);
	
	vector<unsigned char> byteCode = NewShuntingYard(tokens);
	
	VM x;
	x.Execute(&byteCode[0], byteCode.size());
	
	/*
	for(int i = 0; i < byteCode.size(); i++){
		cout << "|" << (int)byteCode[i] << "|\n";
	}
	*/
	

	
    return 0;
}
