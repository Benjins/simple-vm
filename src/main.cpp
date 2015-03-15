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
	def IsPrime(number){\
		var factor : 1;                                   \
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
			return(0);                                       \
		}                                                 \
														\
		if(factorCount = 1){                              \
			return(1);                                     \
		}                                                \
	}\
	def Summation(n){\
		if(n > 0){\
			return(n+Summation(n-1));\
		}\
		return(0);\
	}\
	def MultAdd(a,b,c){\
		var prodOne : a * b;\
		var prodTwo : a * c;\
		return(prodOne + prodTwo);\
	}\
	\
	def SumMultAddSum(x,y,z){\
	    return(Summation(MultAdd(Summation(x),y,z)));\
	}\
	def main(){\
		var input : 1;\
		while(input > 0){\
			input  : READ();\
			PRINT(IsPrime(input));\
			PRINT(Summation(input));\
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
