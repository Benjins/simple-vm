#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

using std::string; using std::vector;

string ParseByteCode(string fileName);

static void ReadFromFile(string& readInto, string fileName);

vector<string> Tokenize(const string& code);

static string __tokensArr[] = {"PRINT","READ", "*", "-", "+", "(", ",", ")"};

static vector<string> __tokens(&__tokensArr[0], &__tokensArr[8]);

static string __operators = "*+";

static string __digits = "0123456789";

#endif