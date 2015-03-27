#include "../header/AST.h"
#include "../header/Instruction.h"
#include "../header/VM.h"

Value* CompileTokenToAST(const string& token){
	const string operators = "+-*><|&=/";
	if(operators.find(token) != string::npos){
		return new Operator(token);
	}
	else{
		return new Builtin(token);
	}
}

void AST::GenerateFromTokens(const vector<string>& tokens){

}

void AST::GenerateByteCode(VM& vm){
	vm.funcPointers.clear();
	vm.byteCodeLoaded.clear();
	for(int i = 0; i < defs.size(); i++){
		vm.funcPointers.insert(std::pair<string, int>(defs[i]->name, vm.byteCodeLoaded.size()));
		defs[i]->AddByteCode(vm);
	}
}

int FuncCall::Evaluate(){
	return -1;
}

void FuncCall::AddByteCode(VM& vm){
	vm.byteCodeLoaded.push_back(STK_FRAME); //Stack frame?
	vm.byteCodeLoaded.push_back(INT_LIT);
	int retAddrIdx = vm.byteCodeLoaded.size();
	vm.byteCodeLoaded.push_back(253); //Return Addr?

	for(int i = 0; i < numParams; i++){
		parameterVals[i]->AddByteCode(vm);
	}

	int funcAddr = vm.funcPointers.find(funcName)->second;

	vm.byteCodeLoaded.push_back(INT_LIT);
	vm.byteCodeLoaded.push_back(varCount);
	vm.byteCodeLoaded.push_back(INT_LIT);
	vm.byteCodeLoaded.push_back(funcAddr);
	vm.byteCodeLoaded.push_back(CALL);

	vm.byteCodeLoaded[retAddrIdx] = vm.byteCodeLoaded.size();
}


int Builtin::Evaluate(){
	return -1;
}

void Builtin::AddByteCode(VM& vm){
	for(int i = 0; i < numParams; i++){
		parameterVals[i]->AddByteCode(vm);
	}

	if(funcName == "PRINT"){
		vm.byteCodeLoaded.push_back(PRINT);
	}
	else if(funcName == "READ"){
		vm.byteCodeLoaded.push_back(READ);
	}
	else if(funcName == "return"){
		vm.byteCodeLoaded.push_back(RETURN);
	}
}

int Assignment::Evaluate(){
	return -1;
}

void Assignment::AddByteCode(VM& vm){
	Literal(reg).AddByteCode(vm);
	val->AddByteCode(vm);
	vm.byteCodeLoaded.push_back(LOAD_REG);
}

int Literal::Evaluate(){
	return value;
}

void Literal::AddByteCode(VM& vm){
	if(value < 256){
		vm.byteCodeLoaded.push_back(INT_LIT);
		vm.byteCodeLoaded.push_back(value);
	}
	else if(value < (1 << 16)){
		vm.byteCodeLoaded.push_back(INT_DLIT);
		vm.byteCodeLoaded.push_back(value / 256);
		vm.byteCodeLoaded.push_back(value % 256);
	}
	else{
		const int thirdByte  = 1 << 16;
		const int fourthByte = 1 << 24;

		vm.byteCodeLoaded.push_back(INT_QLIT);
		vm.byteCodeLoaded.push_back(value / fourthByte);
		vm.byteCodeLoaded.push_back((value % fourthByte) / thirdByte);
		vm.byteCodeLoaded.push_back((value % thirdByte) / 256);
		vm.byteCodeLoaded.push_back(value % 256);
	}
}

int Variable::Evaluate(){
	return -1;
}

void Variable::AddByteCode(VM& vm){
	Literal(reg).AddByteCode(vm);
	vm.byteCodeLoaded.push_back(LOAD_REG);
}

int Operator::Evaluate(){
	return -1;
}

void Operator::AddByteCode(VM& vm){
	left->AddByteCode(vm);
	right->AddByteCode(vm);
	vm.byteCodeLoaded.push_back(instr);
}

int Scope::Evaluate(){
	return -1;
}

void Scope::AddByteCode(VM& vm){
	for(int i = 0; i < numStatements; i++){
		statements[i]->AddByteCode(vm);
	}
}

int FuncDef::Evaluate(){
	return -1;
}

void FuncDef::AddByteCode(VM& vm){
	for(int paramIdx = paramNames.size() - 1; paramIdx >= 0; paramIdx--){
		vm.byteCodeLoaded.push_back(INT_LIT);
		vm.byteCodeLoaded.push_back(paramIdx);
		vm.byteCodeLoaded.push_back(PARAM);
	}
	for(int i = 0; i < numStatements; i++){
		statements[i]->AddByteCode(vm);
	}
}

int IfStatement::Evaluate(){
	return -1;
}

void IfStatement::AddByteCode(VM& vm){
	int jumpAddrIdx = vm.byteCodeLoaded.size();

	vm.byteCodeLoaded.push_back(INT_LIT);
	vm.byteCodeLoaded.push_back(253);
	test->AddByteCode(vm);
	vm.byteCodeLoaded.push_back(BRANCH);

	for(int i = 0; i < numStatements; i++){
		statements[i]->AddByteCode(vm);
	}

	vm.byteCodeLoaded[jumpAddrIdx] = vm.byteCodeLoaded.size();
}

int WhileStatement::Evaluate(){
	return -1;
}

void WhileStatement::AddByteCode(VM& vm){
	int jumpAddrIdx = vm.byteCodeLoaded.size();

	vm.byteCodeLoaded.push_back(INT_LIT);
	vm.byteCodeLoaded.push_back(253);
	test->AddByteCode(vm);
	vm.byteCodeLoaded.push_back(BRANCH);

	for(int i = 0; i < numStatements; i++){
		statements[i]->AddByteCode(vm);
	}

	vm.byteCodeLoaded.push_back(INT_LIT);
	vm.byteCodeLoaded.push_back(jumpAddrIdx);
	vm.byteCodeLoaded.push_back(INT_LIT);
	vm.byteCodeLoaded.push_back(0);
	vm.byteCodeLoaded.push_back(BRANCH);

	vm.byteCodeLoaded[jumpAddrIdx] = vm.byteCodeLoaded.size();
}