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
				   def main(){\
						var input : 1;\
						while(input > 0){\
							input : READ();\
							PRINT(Factorial(input));\
						}\
					}";

	/*
	string code1 = "def MultAdd(a,b,c){\
						return(a*b+a*c);\
				   }\
				   def main(){\
						PRINT(MultAdd(READ(), READ(), READ()));\
				    }";
					*/

	//For some reason, input is being set back to un-initialised, but recursion does work.
	
	VM x;

	/*
	vector<string> tokens1 = NewTokenize(code1);
	vector<unsigned char> byteCode = NewShuntingYard(tokens1, x);
	x.byteCodeLoaded= byteCode;
	*/

	//x.CompileAndLoadCode("test1.svm");
	//x.SaveByteCode("test1.svb");
	//x.LoadByteCode("test1.svb");
	//x.Execute("main");

	vector<string> tokens = NewTokenize(code1);
	vector<string> shuntedTokens = JustShuntingYard(tokens);

	for(int i = 0; i < shuntedTokens.size(); i++){
		//cout << shuntedTokens[i] << "\n";
	}

#if 1 //TESTING
	bool allPass = true;



	//x.CompileAndLoadCode("test2.svm");
	//x.SaveByteCode("test2.svb");
	//x.LoadByteCode("test2.svb");

	//allPass &= (x.Execute("main") == 5);
	//allPass &= (x.Execute("testOne") == 1);
	//allPass &= (x.Execute("testTwo") == 1);
	//allPass &= (x.Execute("testThree") == 0);

	VM b;
	AST y;
	y.GenerateFromShuntedTokens(shuntedTokens, b);

	y.GenerateByteCode(b);
	b.Execute("main");

	for(int i = 0; i < b.byteCodeLoaded.size(); i++){
		//cout << "Instr " << i << ": " << (int)b.byteCodeLoaded[i] << endl;
	}

	//cout << "\nMain entry:" << b.funcPointers["main"] << endl;

	return allPass ? 0 : 1;
#else

#endif

	
    return 0;
}
