#include "../header/AST.h"
#include "../header/Instruction.h"
#include "../header/VM.h"

Statement* CompileTokenToAST(const string& token){
	const string operators = "+-*><|&=/";
	if(operators.find(token) != string::npos){
		return (Statement*)(new Operator(token));
	}
	else{
		return (Statement*)(new Builtin(token));
	}
}

void AST::GenerateFromShuntedTokens(const vector<string>& tokens, VM& vm){
	unordered_map<string, int> varRegs;
	unordered_map<string, int> funcArity;
	int varCount = 0;

	const string operators = "+-*><|&=/";
	const string numbers = "0123456789";

	Stack<Scope*> scopes;
	Stack<Value*> values;

	for(int i = 0; i < tokens.size(); i++){
		string token = tokens[i];

		if(numbers.find(token[0]) != string::npos){
			values.Push(new Literal(atoi(token.c_str())));
		}
		else if(operators.find(token) != string::npos){
			Operator* op = new Operator(token);
			op->right = values.Pop();
			op->left = values.Pop();
			values.Push(op);
		}
		else if(token == "def"){
			string funcName = tokens[i+1];
			i += 3;

			FuncDef* currDef = new FuncDef();
			currDef->name = funcName;

			while(tokens[i] != ")"){
				if(token != ","){
					currDef->paramNames.push_back(token);
				}
			}

			i++; // Skip '{'

			funcArity.insert(std::pair<string, int>(funcName, currDef->paramNames.size()));
			scopes.Push(currDef);
		}
		else if(token == "var"){
			varRegs.insert(std::pair<string, int>(tokens[i+1], varCount));
			varCount++;
		}
		else if(token == "PRINT" || token == "READ" || token == "return" || token == "RETURN"){
			Builtin* builtin = new Builtin(token);
			for(int i = 0; i < builtin->NumParams(); i++){
				builtin->AddParameter(values.Pop());
			}

			values.Push(builtin);
		}
		else if(funcArity.find(token) != funcArity.end()){
			FuncCall* func = new FuncCall();
			for(int i = 0; i < funcArity[token]; i++){
				func->AddParameter(values.Pop());
			}

			values.Push(func);
		}
		else if(varRegs.find(token) != varRegs.end()){
			Variable* var = new Variable();
			var->reg = varRegs[token];
			values.Push(var);
		}
		else if(token == ":"){
			Assignment* assmt = new Assignment();
			assmt->val = values.Pop();
			Variable* reg = (Variable*)values.Pop(); 
			assmt->reg = reg->reg;

			values.Push(assmt);
		}
		else if(token == ";"){
			scopes.Peek()->AddStatement(values.Pop());
		}
		else if(token == "{"){
			scopes.Push(new Scope());
		}
		else if(token == "if"){
			IfStatement* ifStmt = new IfStatement();
			ifStmt->test = values.Pop();
			scopes.Push(ifStmt);
			i++; //capture the {
		}
		else if(token == "while"){
			WhileStatement* whileStmt = new WhileStatement();
			whileStmt->test = values.Pop();
			scopes.Push(whileStmt);
			i++; //capture the {
		}
		else if(token == "}"){
			Scope* scp = scopes.Pop();
			if(scopes.stackSize > 0){
				scopes.Peek()->AddStatement(scp);
			}
			else{
				FuncDef* def = dynamic_cast<FuncDef*>(scp);
				if(def == NULL){
					cout << "\nError: Top-level scope is not a function.\n";
				}
				else{
					defs.push_back(def);
					varRegs.clear();
					varCount = 0;
				}
			}
		}
	}
}

void AST::GenerateByteCode(VM& vm){
	vm.funcPointers.clear();
	vm.byteCodeLoaded.clear();
	for(int i = 0; i < defs.size(); i++){
		vm.funcPointers.insert(std::pair<string, int>(defs[i]->name, vm.byteCodeLoaded.size()));
		FuncDef* def = defs[i];
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