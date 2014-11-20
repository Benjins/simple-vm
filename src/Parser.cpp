#include "../header/Parser.h"
#include <fstream>
#include <iostream>

using std::ifstream; using std::cout; using std::endl; using std::cin;

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

vector<string> Tokenize(const string& code){
	vector<string> tokenizedCode;
	string currentToken = "";
	bool number=false;
	for(int i = 0; i < code.size(); i++){
		char currentChar = code[i];
		
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

		//Don't bother parsing spaces
		if(currentChar == ' '){
			if(currentToken != ""){
				cout << "\nError in forming lexical tokens, token not found:" << currentToken << endl;
				currentToken = "";
				return tokenizedCode;
			}

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