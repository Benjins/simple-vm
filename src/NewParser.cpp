#include "../header/Parser.h"
#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/AST.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>


using std::cout;

vector<string> JustShuntingYard(vector<string>& tokens, map<string, Type>& variables){
	const string operators = "+*-/=><|&:";
	const string numbers = "0123456789.";

	vector<string> shuntedTokens;

	bool funcDef = false;

	Stack<string> operatorStack;

	for(int i = 0; i < tokens.size(); i++){
		string token = tokens[i];
		if(numbers.find(token[0]) != string::npos){
			shuntedTokens.push_back(token);
		}
		else if(variables.find(token) != variables.end()){
			shuntedTokens.push_back(token);
		}
		else if(token == "("){
			operatorStack.Push(token);
		}
		else if(token == ")"){
			while(operatorStack.Peek() != "("){
				shuntedTokens.push_back(operatorStack.Pop());
			}

			operatorStack.Pop();

			if(operatorStack.stackSize > 0 && operators.find(operatorStack.Peek()) == string::npos && operatorStack.Peek() != "("){
				shuntedTokens.push_back(operatorStack.Pop());
			}
		}
		else if(token == ","){
			while(operatorStack.Peek() != "("){
				shuntedTokens.push_back(operatorStack.Pop());
			}
		}
		else if(token == ";"){
			while(operatorStack.stackSize > 0){
				shuntedTokens.push_back(operatorStack.Pop());
			}

			shuntedTokens.push_back(";");
		}
		else if(operators.find(token) != string::npos){
			while(operatorStack.stackSize > 0
				&& operators.find(operatorStack.Peek()) != string::npos
				&& OperatorPrecedence(operatorStack.Peek()) >= OperatorPrecedence(token)){
					shuntedTokens.push_back(operatorStack.Pop());
			}
			operatorStack.Push(token);
		}
		else{
			operatorStack.Push(token);
		}
	}

	while(operatorStack.stackSize > 0){
		shuntedTokens.push_back(operatorStack.Pop());
	}

	return shuntedTokens;
}

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

vector<unsigned char> NewShuntingYard(vector<string> tokens, VM& vm){
	vector<unsigned char> byteCode;
	Stack<string> operatorStack;
	Stack<string> branchingStack;
	Stack<int> branchIndexStack;
	Stack<int> whileIndexStack;
	int assignment = -1;

	map<string, int> varReg;
	map<string, int> paramNames;

	bool funcDef = false;
	int braceCount = 0;
	Stack<int> funcCallInfoIdxs;

	const string numbers = "0123456789.";
	const string operators = "+-*><|&=/";
	int branchIndex = 0;

	int varCount = 0;
	int prevVarCount = 0;

	for(int i = 0; i < tokens.size(); i++){
		string token = tokens[i];
		//cout << "token: |" << token << "|\n";
		if(token.size() == 0){
			cout << "\nEmpty token, compiler error.\n";
			continue;
		}
		else if(token == "def"){
			if(funcDef){
				cout << "\nError: Trying to define a function inside a function body.\n";
			}
			else{
				funcDef = true;
				paramNames.clear();
				braceCount = 0;
				i++;
				string funcName = tokens[i];
				if(funcName == "main"){
					//cout << "Main entry point: " << byteCode.size() << std::endl;
				}
				std::pair<string, int> funcPtr(funcName, byteCode.size());
				//cout << "Function " << funcPtr.first << " at instruction " << funcPtr.second << std::endl;
				vm.funcPointers.insert(funcPtr);

				int paramCount = 0;
				i++;
				if(tokens[i] != "("){
					cout << "\nError: Expected a '(' after 'def " << funcName << "'.\n";
				}

				i++;
				while(tokens[i] != ")"){
					std::pair<string, int> param(tokens[i],paramCount);
					paramNames.insert(param);
					paramCount++;
					varCount++;

					i++;
					if(tokens[i] == ","){
						i++;
					}
				}

				for(int paramIdx = paramCount - 1; paramIdx >= 0; paramIdx--){
					byteCode.push_back(INT_LIT);
					byteCode.push_back(paramIdx);
					byteCode.push_back(PARAM);
				}

			}
		}
		else if(token == "var" || token == "int" || token == "float"){
			i++;
			string varName = tokens[i];
			auto regIdx = varReg.find(varName);
			if(regIdx != varReg.end()){
				cout << "\nVariable " << varName << " already defined.\n";
			}
			else{
				std::pair<string, int> regVal(varName, varCount);
				varReg.insert(regVal);
				assignment = varCount;
				varCount++;
			}
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
			if(operatorStack.stackSize > 0 && vm.funcPointers.find(operatorStack.Peek()) != vm.funcPointers.end()){
				funcCallInfoIdxs.Push(byteCode.size());
				byteCode.push_back(STK_FRAME); //Stack frame?
				byteCode.push_back(INT_LIT);
				byteCode.push_back(253); //Return Addr?
			}
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
			if(operatorStack.stackSize > 0 && vm.funcPointers.find(operatorStack.Peek()) != vm.funcPointers.end()){
				int infoIdx = funcCallInfoIdxs.Pop();
				string funcName = operatorStack.Pop();
				int funcAddr = vm.funcPointers.find(funcName)->second;

				byteCode.push_back(INT_LIT);
				byteCode.push_back(varCount);
				byteCode.push_back(INT_LIT);
				byteCode.push_back(funcAddr);
				byteCode.push_back(CALL);

				byteCode[infoIdx + 2] = byteCode.size(); //Return addr?
				//cout << "ByteCode size: " << byteCode.size() << std::endl;
			}
			else if(operatorStack.stackSize > 0 && operatorStack.Peek() != "(" && operators.find(operatorStack.Peek()) == string::npos){
				//byteCode.erase(byteCode.begin() + infoIdx, byteCode.begin() + infoIdx + 3);
				string op = operatorStack.Pop();
				byteCode.push_back(Compile(op));
			}
			else{
				//byteCode.erase(byteCode.begin() + infoIdx, byteCode.begin() + infoIdx + 3);
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
			byteCode.push_back(INT_LIT);
			byteCode.push_back(assignment);
			assignment = -2;
		}
		else if(token == "if" || token == "while"){
			branchingStack.Push(token);

			if(token == "while"){
				whileIndexStack.Push(byteCode.size() - 1);
			}
		}
		else if(token == "{"){
			if(funcDef){
				braceCount++;
			}
			if(!funcDef || braceCount != 1){
				byteCode.push_back(INT_LIT);
				byteCode.push_back(0);
				branchIndexStack.Push(byteCode.size() - 1);
				byteCode.push_back(BRANCH);
			}
		}
		else if(token == "}"){
			if(!funcDef || braceCount > 1){
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

			if(funcDef){
				braceCount--;
				if(braceCount == 0){
					funcDef = false;
					varCount = prevVarCount;
				}
			}

		}
		else if(token == ";"){
			while(operatorStack.stackSize > 0){
				string op = operatorStack.Pop();

				if(op == ")" || op == "("){
					cout << "\nError: mismatched parentheses.\n";
				}

				byteCode.push_back(Compile(op));
			}
			if(assignment == -2){
				byteCode.push_back(SAVE_REG);
			}
			assignment = -1;
		}
		else if(varReg.find(token) != varReg.end()){
			if(tokens[i + 1] == ":"){
				assignment = varReg.find(token)->second;
			}
			else{
				byteCode.push_back(INT_LIT);
				byteCode.push_back(varReg.find(token)->second);
				byteCode.push_back(LOAD_REG);
			}
		}
		else if(paramNames.find(token) != paramNames.end()){
			byteCode.push_back(INT_LIT);
			byteCode.push_back(paramNames.find(token)->second);
			byteCode.push_back(LOAD_REG);
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
