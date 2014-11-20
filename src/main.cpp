#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include <assert.h>
#include <iostream>

using std::cout; using std::endl;

int main(){

	/*
	unsigned char code[6] = {INT_DLIT, 1, 14, READ, INT_MUL, PRINT};

	VM vm;
	vm.Execute(code, 6);
	*/

	string code = "";
	vector<string> tokens = Tokenize(code);
	assert(tokens.size() == 0);

	code = "PRINT";
	tokens = Tokenize(code);
	assert(tokens.size() == 1);
	assert(tokens[0] == "PRINT");

	code = "PRINT()";
	tokens = Tokenize(code);
	assert(tokens.size() == 3);
	assert(tokens[0] == "PRINT");
	assert(tokens[1] == "(");
	assert(tokens[2] == ")");

	code = "PRINT(2+3)";
	tokens = Tokenize(code);
	assert(tokens.size() == 6);
	assert(tokens[0] == "PRINT");
	assert(tokens[1] == "(");
	assert(tokens[2] == "2");
	assert(tokens[3] == "+");
	assert(tokens[4] == "3");
	assert(tokens[5] == ")");

	code = "PRINT(242 + 3  *(413+5224) + READ())";
	tokens = Tokenize(code);
	for(int i = 0; i < tokens.size(); i++){
		cout << tokens[i] << "%" << endl;
	}

    return 0;
}
