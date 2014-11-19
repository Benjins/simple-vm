#ifndef PARSER_H
#define PARSER_H

#include <string>

using std::string;

string ParseByteCode(string fileName);

static void ReadFromFile(string& readInto, string fileName);


#endif