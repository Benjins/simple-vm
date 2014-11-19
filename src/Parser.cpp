#include "../header/Parser.h"
#include <fstream>

using std::ifstream;

string ParseByteCode(string fileName){
	string fileContents;
	ReadFromFile(fileContents, fileName);

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