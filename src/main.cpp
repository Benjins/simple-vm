#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include <assert.h>
#include <iostream>

using std::cout; using std::endl;

int main(int argc, char** argv){

	cout << argv[0] << endl;

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
	
	cout << "Returned: " << x.Execute("main") << endl;

#if TESTING
	bool allPass = true;

	return allPass ? 0 : 1;
#else

#endif

	
    return 0;
}
