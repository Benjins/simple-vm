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

	//string code = "var z : 1; while(z > 0){var x : READ(); z : z - x;  PRINT(z);}";

	
	string code = "\
				  def Factorial(n){\
					if(n > 1){\
					return(n*Factorial(n-1));\
					}\
					return(1);\
				  }\
	def MultAdd(a,b,c){\
		var prodOne : a * b;\
		var prodTwo : a * c;\
		return(prodOne + prodTwo);\
	}\
	\
	def main(){\
	var output: 2;\
	var input : 1;\
	var midput : 2;\
		while(input > 0){\
			input : READ();\
			PRINT(Factorial(input));\
		}\
	}";

	//For some reason, input is being set back to un-initialised, but recursion does work.
	
	VM x;

	vector<string> tokens = NewTokenize(code);
	
	vector<unsigned char> byteCode = NewShuntingYard(tokens, x);
	
	for(int i = 0; i < byteCode.size(); i++){
		int val = byteCode[i];
		//cout << val << endl;
	}

	
	x.Execute(&byteCode[0], byteCode.size(), "main");
	
	for(int i = 0; i < byteCode.size(); i++){
		//cout << i << ": |" << (int)byteCode[i] << "|\n";
	}
	
	

	
    return 0;
}
