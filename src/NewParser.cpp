#include "../header/Parser.h"
#include "../header/Instruction.h"
#include <fstream>
#include <iostream>
#include <sstream>

using std::cout;

vector<string> NewTokenize(const string& code){
	const string operators = "(),+*-/=><|&{};:";
	const string numbers = "0123456789.";
	string memoryString = "";
	char currChar = ' ';
	bool inQuotes = false;
	bool inNumber = false;
	
	vector<string> tokens;
	
	for(int i = 0; i < code.size(); i++){
		currChar = code[i];
		if(currChar == '"'){
			if(inQuotes || memoryString.size() > 0){
				tokens.push_back(memoryString);
				memoryString = "";
			}
			
			tokens.push_back("\"");
			inQuotes = !inQuotes;
			inNumber = false;
		}
		else if(inQuotes){
			memoryString += currChar;
		}
		else if(currChar == ' ' || currChar == '\r' || currChar == '\t' || currChar == '\n'){
			if(memoryString.size() > 0){
				tokens.push_back(memoryString);
				memoryString = "";
			}
		}
		else if(numbers.find(currChar) != string::npos){
			inNumber = true;
			memoryString += currChar;
		}
		else if(operators.find(currChar) != string::npos){
			inNumber = false;
			if(memoryString.size() > 0){
				tokens.push_back(memoryString);
				memoryString = "";
			}
			
			string op = "_";
			op[0] = currChar;
			tokens.push_back(op);
		}
		else{
			memoryString += currChar;
			inNumber = false;
		}
	}
	
	if(memoryString.size() > 0){
		tokens.push_back(memoryString);
		memoryString = "";
	}
	
	return tokens;
}

vector<unsigned char> NewShuntingYard(vector<string> tokens){
	vector<unsigned char> byteCode;
	Stack<string> operatorStack;
	Stack<string> branchingStack;
	Stack<int> branchIndexStack;
	Stack<int> whileIndexStack;

	const string numbers = "0123456789.";
	const string operators = "+-*><|&=";
	int branchIndex = 0;

	for(int i = 0; i < tokens.size(); i++){
		string token = tokens[i];
		if(token.size() == 0){
			continue;
		}
		else if(numbers.find(token[0]) != string::npos){
			int num = atoi(token.c_str());
			if(num < 255){
				byteCode.push_back(INT_LIT);
				byteCode.push_back((unsigned char)num);
			}
			else{
				byteCode.push_back(INT_DLIT);
				byteCode.push_back(num >> 8);
				byteCode.push_back((unsigned char)num);
			}
		}
		else if(token == ","){
			while(operatorStack.Peek() != "("){
				string op = operatorStack.Pop();
				byteCode.push_back(Compile(op));
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
				byteCode.push_back(Compile(op));
				if(operatorStack.stackSize == 0){
					cout << "\nError: Mismatched parentheses.\n";
					break;
				}
			}

			operatorStack.Pop();
			if(operatorStack.stackSize > 0 && operators.find(operatorStack.Peek()) == string::npos){
				string op = operatorStack.Pop();
				byteCode.push_back(Compile(op));
			}	
		}
		else if(operators.find(token) != string::npos){
			while(operatorStack.stackSize > 0 
				&& operators.find(operatorStack.Peek()) != string::npos 
				&& OperatorPrecedence(operatorStack.Peek()) > OperatorPrecedence(token)){
					byteCode.push_back(Compile(operatorStack.Pop()));
			}
			operatorStack.Push(token);
		}
		else if(token == ":"){
			//Some kind of assignment
		}
		else if(token == "if" || token == "while"){
			branchingStack.Push(token);

			if(token == "while"){
				whileIndexStack.Push(byteCode.size() - 1);
			}
		}
		else if(token == "{"){
			byteCode.push_back(INT_LIT);
			byteCode.push_back(0);
			branchIndexStack.Push(byteCode.size() - 1);
			byteCode.push_back(BRANCH);
		}
		else if(token == "}"){
			string branchingType = branchingStack.Pop();
			int index = branchIndexStack.Pop();
			if(branchingType == "while"){
				byteCode.push_back(INT_LIT);
				byteCode.push_back(0);
				byteCode.push_back(INT_LIT);
				int idx = whileIndexStack.Pop();
				byteCode.push_back(idx+1);
				byteCode.push_back(BRANCH);
			}
			byteCode[index] = byteCode.size();
		}
		else if(token == ";"){
			while(operatorStack.stackSize > 0){
				string op = operatorStack.Pop();

				if(op == ")" || op == "("){
					cout << "\nError: mismatched parentheses.\n";
				}

				byteCode.push_back(Compile(op));
			}
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

		byteCode.push_back(Compile(op));
	}

	return byteCode;
}
