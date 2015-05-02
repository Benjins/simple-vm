#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include <iostream>

#define MAX_STACK_SIZE 512

using std::string; using std::vector; using std::cout;

struct VM;

vector<string> JustShuntingYard(vector<string>& tokens);

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
		}
	}

	T Peek() const{
		return values[stackSize];
	}
};


static void ReadFromFile(string& readnto, string fileName);

vector<string> NewTokenize(const string& code);

vector<unsigned char> NewShuntingYard(vector<string> tokens, VM& vm);

string MakeIntLiteral(int number);

unsigned char Compile(string token);

bool IsAFunctionToken(string token);

int OperatorPrecedence(string token);

static string __tokensArr[] = {"PRINT","READ", "*", "-", "+", "(", ",", ")"};

static string __functionsArr[] = {"PRINT","READ"};

static vector<string> __tokens(&__tokensArr[0], &__tokensArr[8]);

static string __operators = "*+";

static string __digits = "0123456789";

#endif
