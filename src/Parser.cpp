#include "../header/Parser.h"
#include "../header/Instruction.h"
#include <fstream>
#include <iostream>
#include <sstream>

using std::ifstream; using std::cout; using std::endl; using std::cin;
using std::stringstream;

int OperatorPrecedence(string token){
	static const int operatorCount = 11;
	int precedence[operatorCount] = {3,3,4,4,2,2,2,1,1,0,5};
	
	//cout << "OpPrec\n";
	
	string ops[operatorCount] = {"+", "-", "/", "*", "==", ">", "<", "|", "&", "=", "."};
	

	int index = 0;
	for(int i = 0; i < operatorCount; i++){
		if(ops[i] == token){
			index = i;
			break;
		}
	}

	if(index != string::npos){
		//cout << "Operator: " << token << ", precedence: " << precedence[index] << endl;
		return precedence[index];
	}

	cout << "\nWarning: Called OperatorPrecedence on non-operator.\n";
	return -1;
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
	if(token == "=="){
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

	cout << "\nError: tried to compile unfamiliar token: '" << token << "'\n";
	return '\0';
}
