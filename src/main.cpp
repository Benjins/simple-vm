#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include <assert.h>
#include <iostream>

using std::cout; using std::endl;

int main(){
	
	/*
	string code =	 "var factor : 1;                                   \
					  var number : READ();                              \
					  var factorCount : 0;                              \
																		\
					  while(factor < number){                           \
						 if( factor * (number / factor) = number){      \
							 factorCount : factorCount + 1;             \
						 }                                              \
																		\
						 factor : factor + 1;                           \
					  }                                                 \
																		\
					  if(factorCount > 1 | factorCount = 0){			\
						PRINT(0);                                       \
					  }                                                 \
																		\
					  if(factorCount = 1){                              \
						PRINT(1);                                       \
					  }"; 
				   */

	string code = "var z : 1; while(z > 0){var x : READ(); z : z - x;  PRINT(z);}";

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
