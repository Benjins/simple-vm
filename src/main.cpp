#include "../header/VM.h"
#include "../header/Instruction.h"
#include "../header/Parser.h"
#include "../header/AST.h"
#include <assert.h>
#include <iostream>

using std::cout; using std::endl;

int main(int argc, char** argv){

	string code1 = " def add(nix){\
				   var v : nix*2;\
				   if(v = 0){\
					  return(0);\
				   }\
				   return(v+4*READ());\
				   }\
				   def main(){\
						var input : READ();\
						var output : add(2)*3+1+4*4/5;\
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
	//allPass &= (x.Execute("testOne") == 1);
	//allPass &= (x.Execute("testTwo") == 1);
	//allPass &= (x.Execute("testThree") == 0);

	AST y;

	FuncDef* defAdd = new FuncDef();
	defAdd->name = "add";
	defAdd->paramNames.push_back("num");
	defAdd->AddStatement(new Builtin("return"));
	Variable* numVar = new Variable();
	numVar->reg = 0;
	Operator* op = new Operator("+");
	op->left = numVar;
	op->right = new Literal(7);
	((FuncCall*)(defAdd->statements[0]))->AddParameter(op);
	y.defs.push_back(defAdd);

	FuncDef* def = new FuncDef();
	def->name = "main";
	y.defs.push_back(def);
	def->AddStatement(new Builtin("PRINT"));
	FuncCall* call = new FuncCall();
	call->funcName = "add";
	call->AddParameter(new Builtin("READ"));
	((FuncCall*)(def->statements[0]))->AddParameter(call);

	y.GenerateByteCode(x);
	//x.Execute("main");

	for(int i = 0; i < x.byteCodeLoaded.size(); i++){
		//cout << "Instr: " << (int)x.byteCodeLoaded[i] << endl;
	}

	return allPass ? 0 : 1;
#else

#endif

	
    return 0;
}
