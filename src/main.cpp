#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include "../header/AST.h"
#include <assert.h>
#include <iostream>

using std::cout; using std::endl;

int main(int argc, char** argv){

	
	string code1 = "def Factorial(num){\
					   if(num < 2){\
							return(1);\
					   }\
					   return(num * (Factorial(num-1)));\
				   }\
				   def FactorialLoop(num){\
					   var total : 1;\
					   var counter : 1;\
					   while(counter < num+1){\
							total : total * counter;\
							counter : counter + 1;\
					   }\
					   return(total);\
				   }\
				   def FactorialLoopTest(){\
					   var num : 5;\
					   var total : 1;\
					   var counter : 1;\
					   while(counter < num+1){\
							total : total * counter;\
							counter : counter + 1;\
					   }\
					   RETURN(total);\
				   }\
				   def main(){\
						var input : 1;\
						while(input > 0){\
							input : READ();\
							PRINT(FactorialLoop(input));\
						}\
					}";

	
	/*
	string code1 = "def MultAdd(a,b,c){\
						return(a*b+a*c);\
				   }\
				   def main(){\
						PRINT(MultAdd(READ(), READ(), READ()));\
				    }";*/
					
	
	VM x;

	/*
	vector<string> tokens1 = NewTokenize(code1);
	vector<unsigned char> byteCode = NewShuntingYard(tokens1, x);
	x.byteCodeLoaded= byteCode;
	*/

	vector<string> tokens = NewTokenize(code1);
	vector<string> shuntedTokens = JustShuntingYard(tokens);

	for(int i = 0; i < shuntedTokens.size(); i++){
		//cout << shuntedTokens[i] << "\n";
	}

#if 1 //TESTING
	bool allPass = true;



	x.CompileAndLoadCode("test2.svm");
	x.SaveByteCode("test2.svb");
	x.LoadByteCode("test2.svb");

	//allPass &= (x.Execute("main") == 5);
	allPass &= (x.Execute("testOne") == 1);
	allPass &= (x.Execute("testTwo") == 1);
	allPass &= (x.Execute("testThree") == 0);

	VM b;
	AST y;
	y.GenerateFromShuntedTokens(shuntedTokens, b);

	y.GenerateByteCode(b);
	allPass &= (b.Execute("FactorialLoopTest") == 120);

	for(int i = 0; i < b.byteCodeLoaded.size(); i++){
		//cout << "Instr " << i << ": " << (int)b.byteCodeLoaded[i] << endl;
	}

	//cout << "\nMain entry:" << b.funcPointers["main"] << endl;

	cout << (allPass ? "allPass" : "some failed.") << endl;
	return allPass ? 0 : 1;
#else

#endif

	
    return 0;
}
