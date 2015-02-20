#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include <assert.h>
#include <iostream>

using std::cout; using std::endl;

int main(){

	
	//string code = "PRINT(READ()/4);";

	/*
	string code = "SAVE(0,1);\
				  SAVE(1, READ());\
				  SAVE(2, 0);\
				  while(LOAD(0) < LOAD(1)){\
					 if( LOAD(0) * (LOAD(1) / LOAD(0)) = LOAD(1)){\
						SAVE(2, LOAD(2) + 1);\
					 }\
				     SAVE(0, LOAD(0) + 1);\
				  }\
				  if(LOAD(2) > 1){\
					PRINT(0);\
				  }\
				  if(LOAD(2) = 1){\
					PRINT(1);\
				  }"; */

	/*
	string code = "var x : READ();\
				   var y : 0;\
				   while(y < x){\
				      PRINT(y);\
					  y : y + 1;\
				   }";*/

	
	string code = "var x : 1;\
				  var y : READ();\
				  var z : 0;\
				  while(x < y){\
					 if( x * (y / x) = y){\
					     z : z + 1;\
					 }\
					 x : x + 1;\
				  }\
				  if(z > 1){\
					PRINT(0);\
				  }\
				  if(z = 1){\
					PRINT(1);\
				  }"; 
				   

	vector<string> tokens = NewTokenize(code);
	
	vector<unsigned char> byteCode = NewShuntingYard(tokens);
	
	for(int i = 0; i < byteCode.size(); i++){
		int val = byteCode[i];
		//cout << val << endl;
	}

	VM x;
	x.Execute(&byteCode[0], byteCode.size());
	
	/*
	for(int i = 0; i < byteCode.size(); i++){
		cout << "|" << (int)byteCode[i] << "|\n";
	}
	*/
	

	
    return 0;
}
