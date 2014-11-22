#include "../header/Parser.h"
#include "../header/Instruction.h"
#include <fstream>
#include <iostream>
#include <sstream>

using std::ifstream; using std::cout; using std::endl; using std::cin;
using std::stringstream;

string ParseByteCode(string fileName){
	string fileContents;
	ReadFromFile(fileContents, fileName);

	return ShuntingYard(Tokenize(fileContents));

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

string ShuntingYard(vector<string> tokens){
	stringstream byteCode;
	StringStack operatorStack;

	for(int i = 0; i < tokens.size(); i++){

		string token = tokens[i];
		if(__digits.find(token[0]) != string::npos){
			byteCode << MakeIntLiteral(atoi(token.c_str()));
		}
		else if(token == ","){
			while(operatorStack.Peek() != ")"){
				string op = operatorStack.Pop();
				byteCode << Compile(op);
				if(operatorStack.stackSize == 0){
					cout << "\nError: Mismatched parentheses.\n";
					break;
				}
			}
		}
		else if(token == "("){
			operatorStack.Push(token);
		}
		else if(token == ")"){
			while(operatorStack.Peek() != "("){
				string op = operatorStack.Pop();
				byteCode << Compile(op);
				if(operatorStack.stackSize == 0){
					cout << "\nError: Mismatched parentheses.\n";
					break;
				}
			}

			operatorStack.Pop();
			if(operatorStack.stackSize > 0 && IsAFunctionToken(operatorStack.Peek())){
				byteCode << Compile(operatorStack.Pop());
			}
			
		}
		else if(__operators.find(token) != string::npos){
			while(operatorStack.stackSize > 0 
				&& __operators.find(operatorStack.Peek()) != string::npos 
				&& OperatorPrecedence(operatorStack.Peek()) > OperatorPrecedence(token)){
					byteCode << Compile(operatorStack.Pop());
			}
			operatorStack.Push(token);
		}
		else{
			operatorStack.Push(token);
		}
	}

	while(operatorStack.stackSize > 0){
		string op = operatorStack.Pop();

		if(op == ")" || op == "("){
			cout << "\nError: mismatched parentheses.\n";
		}

		byteCode << Compile(op);
	}

	string code;
	byteCode >> code;

	return code;
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
	if(token == "+"){
		return 1;
	}
	if(token == "*"){
		return 2;
	}

	cout << "\nWarning: Called OperatorPrecedence on non-operator.\n";
	return -1;
}

StringStack::StringStack(){
	stackSize = 0;
}

void StringStack::Push(string str){
	if(stackSize < MAX_STACK_SIZE){
		stackSize++;
		values[stackSize] = str;
	}
	else{
		cout << "\nError: String Stack overflow.\n";
	}
}
string StringStack::Pop(){
	if(stackSize > 0){
		string ret = values[stackSize];
		stackSize--;
		return ret;
	}
	else{
		cout << "\nError: String Stack overflow.\n";
	}
}

string StringStack::Peek() const{
	return values[stackSize];
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

string Compile(string token){
	if(token == "PRINT"){
		char code[2];
		code[0] = PRINT;
		code[1] = 0;
		return string(code);
	}
	if(token == "READ"){
		char code[2];
		code[0] = READ;
		code[1] = 0;
		return string(code);
	}
	if(token == "*"){
		char code[2];
		code[0] = INT_MUL;
		code[1] = 0;
		return string(code);
	}
	if(token == "-"){
		char code[2];
		code[0] = INT_INV;
		code[1] = 0;
		return string(code);
	}
	if(token == "+"){
		char code[2];
		code[0] = INT_ADD;
		code[1] = 0;
		return string(code);
	}

	cout << "\nError: tried to compile unfamiliar token.\n";
	return "";
}