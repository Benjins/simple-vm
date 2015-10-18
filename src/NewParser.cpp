#include "../header/Parser.h"
#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/AST.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>


using std::cout;

vector<string> JustShuntingYard(vector<string>& tokens, map<string, FuncDef*>& defs, map<string, FuncDef*>& builtinDefs){
	const string operators = "+*-/=><|&.";
	const string numbers = "0123456789";

	vector<string> shuntedTokens;

	bool funcDef = false;

	Stack<string> operatorStack;

	for(int i = 0; i < tokens.size(); i++){
		string token = tokens[i];
		if(numbers.find(token[0]) != string::npos){
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
		else if(operators.find(token[0]) != string::npos){
			while(operatorStack.stackSize > 0
				&& operators.find(operatorStack.Peek()) != string::npos
				&& OperatorPrecedence(operatorStack.Peek()) >= OperatorPrecedence(token)){
					shuntedTokens.push_back(operatorStack.Pop());
			}
			operatorStack.Push(token);
		}
		else if(defs.find(token) == defs.end() 
			&& builtinDefs.find(token) == builtinDefs.end()){
			shuntedTokens.push_back(token);
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

enum struct TokenizeState{
	WORD,
	OPERATOR,
	NUMBER,
	QUOTES,
	EMPTY
};

vector<string> NewTokenize(const string& code){
	static const int operatorCount = 17;
	const string operators[operatorCount] = {"(", ")", "{", "}", ",", ".", "+", "*", "-", "/", "=", "==", "<", ">", "&", "|", ";"};
	const string numbers = "0123456789";
	string memoryString = "";
	char currChar = ' ';

	vector<string> tokens;
	TokenizeState state = TokenizeState::EMPTY;

	for(int i = 0; i < code.size(); i++){
		currChar = code[i];

		bool charIsOperator = false;
		for(int idx = 0; idx < operatorCount; idx++){
			if(operators[idx].find(currChar) != string::npos){
				charIsOperator = true;
				break;
			}
		}

		bool charIsNumber = numbers.find(currChar) != string::npos;

		bool charIsWhitespace = (currChar == ' ' || currChar == '\n' || currChar == '\t' || currChar == '\r');

		if(charIsWhitespace){
			if(state != TokenizeState::EMPTY){
				tokens.push_back(memoryString);
				memoryString = "";
			}
			state = TokenizeState::EMPTY;
		}
		
		switch (state)
		{
		case TokenizeState::WORD:
			{
				if(charIsOperator){
					state = TokenizeState::OPERATOR;
					tokens.push_back(memoryString);
					memoryString = "";
					memoryString += currChar;
				}
				else{
					memoryString += currChar;
				}
			}break;
		case TokenizeState::OPERATOR:
			{
				if(charIsOperator){
					string memoryAndThis = memoryString;
					memoryAndThis += currChar;

					bool continuesOperator = false;
					for(int idx = 0; idx < operatorCount; idx++){
						if(operators[idx].find(memoryAndThis) != string::npos){
							continuesOperator = true;
							break;
						}
					}

					if(continuesOperator){
						memoryString += currChar;
						//If I were a leet haxxor, I'd do this:
						//memoryString.swap(memoryAndThis);
					}
					else{
						tokens.push_back(memoryString);
						memoryString = "";
						memoryString += currChar;
					}
				}
				else if(charIsNumber){
					tokens.push_back(memoryString);
					memoryString = "";
					memoryString += currChar;

					state = TokenizeState::NUMBER;
				}
				else{
					tokens.push_back(memoryString);
					memoryString = "";
					memoryString =+ currChar;
					state = TokenizeState::WORD;
				}
			}break;
		case TokenizeState::NUMBER:
			{
				if(charIsNumber || currChar == '.'){
					memoryString += currChar;
				}
				else{
					tokens.push_back(memoryString);
					memoryString = "";
					memoryString += currChar;
					
					state = (charIsOperator ? TokenizeState::OPERATOR : TokenizeState::WORD);
				}
			}break;
		case TokenizeState::QUOTES:
			{
			}break;
		case TokenizeState::EMPTY:
			{
				if(charIsWhitespace){
					
				}
				else{
					memoryString = "";
					memoryString += currChar;

					if(charIsOperator){
						state = TokenizeState::OPERATOR;
					}
					else if(charIsNumber){
						state = TokenizeState::NUMBER;
					}
					else{
						state = TokenizeState::WORD;
					}
				}
			}break;
		default:
			break;
		}
	}

	if(memoryString.size() > 0){
		tokens.push_back(memoryString);
		memoryString = "";
	}

	return tokens;
}
