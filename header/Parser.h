#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

#define MAX_STACK_SIZE 1024

using std::string; using std::vector;

struct StringStack{
	string values[MAX_STACK_SIZE];
	int stackSize;

	StringStack();

	void Push(string str);
	string Pop();
	string Peek() const;
};

string ParseByteCode(string fileName);

static void ReadFromFile(string& readInto, string fileName);

vector<string> Tokenize(const string& code);

string ShuntingYard(vector<string> tokens);

string MakeIntLiteral(int number);

string Compile(string token);

bool IsAFunctionToken(string token);

int OperatorPrecedence(string token);

static string __tokensArr[] = {"PRINT","READ", "*", "-", "+", "(", ",", ")"};

static string __functionsArr[] = {"PRINT","READ"};

static vector<string> __tokens(&__tokensArr[0], &__tokensArr[8]);

static string __operators = "*+";

static string __digits = "0123456789";

#endif