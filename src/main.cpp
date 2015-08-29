#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include "../header/AST.h"
#include "../header/DLLFile.h"
#include <assert.h>
#include <iostream>

//#include <dlfcn.h>

using std::cout; using std::endl;

#define COMPILER 1

int main(int argc, char** argv){

#if TESTING
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
	
	
	vector<string> tokens = NewTokenize(code1);
	vector<string> shuntedTokens = JustShuntingYard(tokens);

	VM x;
	bool allPass = true;

	x.CompileAndLoadCode("test2.svm");
	x.SaveByteCode("test2.svb");
	x.LoadByteCode("test2.svb");

	allPass &= (x.Execute("main") == 5);
	allPass &= (x.Execute("testOne") == 1);
	allPass &= (x.Execute("testTwo") == 1);
	allPass &= (x.Execute("testThree") == 0);
	allPass &= (x.Execute("testFour") == 4);

	VM b;
	AST y;
	y.GenerateFromShuntedTokens(shuntedTokens, b);

	y.GenerateByteCode(b);
	allPass &= (b.Execute("FactorialLoopTest") == 120);

	cout << (allPass ? "allPass" : "some failed.") << endl;
	return allPass ? 0 : 1;

#elif COMPILER //VM Compiler
	if(argc < 2){
		cout << "\nError: Must supply at least one input file.\n";
		return -1;
	}

	char* fileNameC = argv[1];
	string fileName = string(fileNameC);

	vector<string> dllNames;
	for(int i = 2; i < argc; i++){
		char* dllNameC = argv[i];
		string dllName = string(dllNameC);
		dllNames.push_back(dllName);
	}

	VM x;
	if(x.CompileAndLoadCode(fileName, &dllNames)){
		x.SaveByteCode(fileName + ".svb");
		x.LoadByteCode(fileName + ".svb");
		x.Execute("main");
	}


#else //VM runner
	if(argc < 2){
		cout << "\nError: Must supply at least one input file.\n";
		return -1;
	}

	string fileName = string(argv[1]);
	string functionName = (argc > 2) ? string(argv[2]) : "main";

	VM x;
	if(x.LoadByteCode(fileName)){
		int retVal = x.Execute(functionName);
		cout << "\nReturned: " << retVal << endl;
	}
#endif

	
    return 0;
}
