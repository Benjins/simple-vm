#include "../header/Parser.h"
#include "../header/Instruction.h"
#include <fstream>
#include <iostream>
#include <sstream>

using std::ifstream; using std::cout; using std::endl; using std::cin;
using std::stringstream;

string ParseByteCode(string fileName){

	return "";
}

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

vector<string> Tokenize(const string& code){
	vector<string> tokenizedCode;
	string currentToken = "";
	bool number=false;
	for(int i = 0; i < code.size(); i++){
		char currentChar = code[i];
		
		//Don't bother parsing spaces
		if(currentChar == ' '){
			//Unless we're dealing with a number
			if(number){
				number=false;
				tokenizedCode.push_back(currentToken);
				currentToken = "";
			}
			if(currentToken != ""){
				cout << "\nError in forming lexical tokens, token not found:" << currentToken << endl;
				currentToken = "";
				return tokenizedCode;
			}

			continue;
		}

		//We've hit the end of a number
		if(number && __digits.find(currentChar) == string::npos){
			number = false;
			tokenizedCode.push_back(currentToken);
			currentToken = "";
		}
		//We've got 
		else if(!number && __digits.find(currentChar) != string::npos){
			number = true;
			string charCast = "1";
			charCast[0] = currentChar;
			currentToken += charCast;
			continue;
		}
		else if(number){
			string charCast = "1";
			charCast[0] = currentChar;
			currentToken += charCast;
			continue;
		}

		string charCast = "1";
		charCast[0] = currentChar;
		currentToken += charCast;

		bool foundToken = false;
		for(int i = 0; i < __tokens.size(); i++){
			if(__tokens[i].find(currentToken) != string::npos){
				if(__tokens[i].size() == currentToken.size()){
					tokenizedCode.push_back(currentToken);
					currentToken = "";
				}
				foundToken = true;
				break;
			}
		}

		if(!foundToken){
			cout << "\nError in forming lexical tokens, token not found:" << currentToken << endl;
			return tokenizedCode;
		}
	}

	return tokenizedCode;
}

//Deprecated
string ShuntingYard(vector<string> tokens){
	return "";
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
