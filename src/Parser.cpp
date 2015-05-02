#include "../header/Parser.h"
#include "../header/Instruction.h"
#include <fstream>
#include <iostream>
#include <sstream>

using std::ifstream; using std::cout; using std::endl; using std::cin;
using std::stringstream;

static void ReadFromFile(string& readInto, string fileName){
	ifstream fileIn;
	fileIn.open(fileName.c_str());

	if(!fileIn.good()){
		fileIn.close();
		return;
	}

	while(!fileIn.eof()){
		fileIn >> readInto;
	}

	fileIn.close();
}

bool IsAFunctionToken(string token){
	for(int i = 0; i < 2; i++){
		if(token == __functionsArr[i]){
			return true;
		}
	}

	return false;

}

int OperatorPrecedence(string token){
	int precedence[] = {3,3,4,4,2,2,2,1,1,0};
	
	//cout << "OpPrec\n";
	
	string ops = "+-*/=><|&:";
	
	int index = ops.find(token);
	if(index != string::npos){
		//cout << "Operator: " << token << ", precedence: " << precedence[index] << endl;
		return precedence[index];
	}

	cout << "\nWarning: Called OperatorPrecedence on non-operator.\n";
	return -1;
}

string MakeIntLiteral(int number){
	if(number < 255){
		char code[3];
		code[0] = INT_LIT;
		code[1] = number;
		code[2] = 0;
		return string(code);
	}
	else{
		char code[4];
		code[0] = INT_DLIT;
		code[1] = (number >> 8) & 0x000000ff ;
		code[2] = number & 0x000000ff;
		code[3] = 0;
		return string(code);
	}
}

//TODO: Make this return vector<char> for more complicated operations
unsigned char Compile(string token){
	if(token == "*"){
		return INT_MUL;
	}
	if(token == "/"){
		return INT_DIV;
	}
	if(token == "-"){
		return INT_SUB;
	}
	if(token == "+"){
		return INT_ADD;
	}
	if(token == "="){
		return COMPARE;
	}
	if(token == "<"){
		return L_THAN;
	}
	if(token == ">"){
		return G_THAN;
	}
	if(token == "|"){
		return BOOL_OR;
	}
	if(token == "&"){
		return BOOL_AND;
	}
	if(token == "PRINT"){
		return PRINT;
	}
	if(token == "READ"){
		return READ;
	}
	if(token == "LOAD"){
		return LOAD_REG;
	}
	if(token == "SAVE"){
		return SAVE_REG;
	}
	if(token == "return"){
		return RETURN;
	}
	if(token == "RETURN"){
		return RETURN_FINAL;
	}

	cout << "\nError: tried to compile unfamiliar token: '" << token << "'\n";
	return '\0';
}
