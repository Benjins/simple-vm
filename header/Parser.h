#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <iostream>
#include <map>

#define MAX_STACK_SIZE 512

using std::string; using std::vector; using std::cout; using std::map;

struct VM; struct Type; struct FuncDef;

vector<string> JustShuntingYard(vector<string>& tokens, map<string, FuncDef*>& defs, map<string, FuncDef*>& builtinDefs);

template<typename T>
struct Stack{
	T values[MAX_STACK_SIZE];
	int stackSize;

	Stack(){
		stackSize = 0;
	}

	void Push(T str){
		if(stackSize < MAX_STACK_SIZE){
			stackSize++;
			values[stackSize] = str;
		}
		else{
			cout << "\nError: Stack overflow.\n";
		}
	}

	T Pop(){
		if(stackSize > 0){
			T ret = values[stackSize];
			stackSize--;
			return ret;
		}
		else{
			cout << "\nError: Stack underflow.\n";
			T x;
			return x;
		}
	}

	T Peek() const{
		return values[stackSize];
	}
};

vector<string> NewTokenize(const string& code);

unsigned char Compile(string token);

int OperatorPrecedence(string token);

#endif
