#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include <assert.h>
#include <iostream>

using std::cout; using std::endl;

int main(int argc, char** argv){

	string code1 = " def main(){\
						var input : READ();\
						RETURN(input * 2 + 3);\
					}";

	//For some reason, input is being set back to un-initialised, but recursion does work.
	
	VM x;

	/*
	vector<string> tokens1 = NewTokenize(code1);
	vector<unsigned char> byteCode = NewShuntingYard(tokens1, x);
	x.byteCodeLoaded= byteCode;
	*/

	x.CompileAndLoadCode("test1.svm");
	x.SaveByteCode("test1.svb");
	x.LoadByteCode("test1.svb");
	//x.Execute("main");


#if 1 //TESTING
	bool allPass = true;

	x.CompileAndLoadCode("test2.svm");
	x.SaveByteCode("test2.svb");
	x.LoadByteCode("test2.svb");

	allPass &= (x.Execute("main") == 5);
	allPass &= (x.Execute("testOne") == 1);
	allPass &= (x.Execute("testTwo") == 1);
	allPass &= (x.Execute("testThree") == 0);

	return allPass ? 0 : 1;
#else

#endif

	
    return 0;
}
