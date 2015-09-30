#include "../header/AST.h"
#include "../header/Instruction.h"
#include "../header/VM.h"

#include <algorithm>

#define FIND(container, item) std::find((container).begin(), (container).end(), (item))

Statement* CompileTokenToAST(const string& token){
	const string operators = "+-*><|&=/";
	if(operators.find(token) != string::npos){
		return (Statement*)(new Operator(token));
	}
	else{
		return (Statement*)(new Builtin(token));
	}
}

typedef string Token;

struct TokenStream{
	vector<Token> tokens;
	int cursor;

	AST* ast;

	map<string, Type> definedTypes;
	map<string, FuncDef*> definedFuncs;
	map<string, FuncDef*> builtinFuncs;
	int stackSizeInWords;
	map<string, Type> variables;

	vector<string> binaryOps;
	vector<string> unaryOps;

	Stack<Scope*> scopes;

	bool ExpectAndEatToken(const string& keyWord){
		if(tokens[cursor] == keyWord){
			cursor++;
			return true;
		}

		return false;
	}

	bool ExpectAndEatUniqueName(){
		if(definedFuncs.find())
		return false;
	}

	bool ExpectAndEatVariable(){
		if(variables.find(tokens[cursor]) != variables.end()){
			cursor++;
			return true;
		}

		return false;
	}

	bool ExpectAndEatAssignment(){
		int oldCursor = cursor;
		if(!ExpectAndEatVariable()){
			cursor = oldCursor;
			return false;
		}

		const string& varName = tokens[cursor-1];

		if(!ExpectAndEatToken("=")){
			cursor = oldCursor;
			return false;
		}

		Value* val;
		if(!ExpectAndEatValue(&val)){
			cursor = oldCursor;
			return false;
		}

		if(!ExpectAndEatToken(";")){
			cursor = oldCursor;
			return false;
		}

		Assignment* assgn = new Assignment();
		assgn->var =

		return true;
	}

	void AddVariable(const Type& type, const string& name){
		variables.insert(std::make_pair(name, type));
		if(scopes.stackSize > 0){
			scopes.Peek()->variablesInScope.insert(std::make_pair(name, type));
		}

		stackSizeInWords += type.sizeInWords;
	}

	bool ExpectAndEatNumber(VMValue* out){
		if(ExpectAndConvertNumber(tokens[cursor], out)){
			cursor++;
			return true;
		}

		return false;
	}

	bool ExpectAndConvertNumber(const string& token, VMValue* out){
		static const string _digits = "0123456789";
		if(_digits.find(token[0]) != string::npos){
			if(out != nullptr){
				if(tokens[cursor].find(".") != string::npos){
					out->type = ValueType::FLOAT;
					out->floatValue = atof(token.c_str());
				}
				else{
					out->type = ValueType::INT;
					out->floatValue = atoi(token.c_str());
				}
			}
			return true;
		}

		return false;
	}

	bool EatFieldName(){
		return true;
	}

	/*
	bool ExpectAndEatValueHelp(Value** out){
		int oldCursor = cursor;
		VMValue val;
		if(ExpectAndEatNumber(&val)){
			if(val.type == ValueType::FLOAT){
				*out = new FloatLiteral(val.floatValue);
			}
			else{
				*out = new Literal(val.intValue);
			}

			return true;
		}

		if(ExpectAndEatVariable()){
			if(ExpectAndEatToken(".")){
				if(EatFieldName()){
					FieldAcces* access = new FieldAcces();
					Variable* var = new Variable();
					var->varName = tokens[cursor - 3];
					access->var = var;
					access->fieldName = tokens[cursor - 1];
					const string& structName = variables.find(var->varName)->second.name;
					StructDef* structType = nullptr;
					for(StructDef* structDef : ast->structDefs){
						if(structDef->name == structName){
							structType = structDef;
							break;
						}
					}

					if(structType == nullptr){
						//Error?
						cursor = oldCursor;
						return false;
					}
					else{
						int offset = 0;
						for(const StructMember& member : structType->members){
							if(member.name == access->fieldName){
								offset = member.offset;
								break;
							}
						}

						access->offset = offset;
						*out = offset;
						return true;
					}
				}
				else{
					//Error?
					cursor = oldCursor;
					return false;
				}
			}
			else{
				Variable* var = new Variable();
				var->varName = tokens[cursor - 1];
				*out = var;
				return true;
			}
		}
	}
	*/

	/*

	#valueHelp -> number
	#valueHelp -> variable  + . + fieldName
	#valueHelp -> variable
	valueHelp -> unaryOp + Value
	valueHelp -> ( + valueHelp + )
	valueHelp -> funcCAll + ( + valueList + )

	value -> valueHelp
	value -> valueHelp + bi_op + value
	*/

	bool ExpectAndEatValue(Value** out){
		int oldCursor = cursor;
		vector<string> valueTokens;
		int parenCount = 0;

		//Bit of a hack so i can use JustShuntingYard()
		for(int index = cursor; index < tokens.size(); index++){
			if(tokens[index] == "("){
				parenCount++;
			}
			else if(tokens[index] == ")"){
				parenCount--;
			}

			if(tokens[index] == ";" || parenCount < 0){
				break;
			}

			valueTokens.push_back(tokens[index]);
		}

		vector<string> shuntedTokens = JustShuntingYard(valueTokens);

		Stack<Value*> values;
		Stack<string> operatorStack;
		for(const string& str : shuntedTokens){
			VMValue val;
			if(ExpectAndConvertNumber(&val)){
				//push num onto stack
				if(val.type == ValueType::INT){
					values.push(new Literal(val.intValue));
				}
				else{
					values.push(new FloatLiteral(val.floatValue));
				}
			}
			else if(FIND(binaryOps, token) != binaryOps.end()){
				Operator* op = CompileTokenToAST(token);
				op->right = values.Pop();
				op->left  = values.Pop();
				values.Push(op);
			}
			else if(FIND(builtinFuncs, token) != builtinFuncs.end() || FIND(definedFuncs, token) != definedFuncs.end()){
				FuncCall* call = new FuncCall();
				call->funcName = token;
				FuncDef* def = FIND(builtinFuncs, token)->second;
				for(int i = 0; i < def->parameters.size(); i++){
					if(values.stackSize == 0){
						cout << "\n\nError, function '" << token << "' takes " << def->parameters.size() << " arguments.\n";
						return false;
					}
					call->AddParameter(values.Pop());
				}

				values.Push(call);
			}
			else{
				cout << "\nOdd egde case?\n";
			}
		}

		if(values.stackSize == ){
			*out = values.Peek();
			cursor += valueTokens.size();
			return true;
		}
		else{
			return false;
		}
	}

	bool ExpectAndEatVarDecl(){
		int currentCursor = cursor;

		if(!ExpectAndEatType()){
			cursor = currentCursor;
			return false;
		}

		const Type& declType = definedTypes.find(tokens[cursor-1])->second;

		if(!ExpectAndEatUniqueName()){
			cursor = currentCursor;
			return false;
		}

		const string& uniqueName = tokens[cursor-1];

		if(ExpectAndEatToken(";")){
			AddVariable(declType, uniqueName);
			return true;
		}
		else if(ExpectAndEatToken("=")){
			if(ExpectAndEatValue()){
				AddVariable(declType, uniqueName);
				return true;
			}
			else{
				cursor = currentCursor;
				return false;
			}
		}
		else{
			//ERROR?
			cursor = currentCursor;
			return false;
		}
	}

	bool ExpectAndEatFieldDecl(StructDef* def){
		int currentCursor = cursor;

		if(!ExpectAndEatType()){
			cursor = currentCursor;
			return false;
		}

		const Type& declType = definedTypes.find(tokens[cursor-1])->second;

		if(!ExpectAndEatUniqueName()){
			cursor = currentCursor;
			return false;
		}

		const string& uniqueName = tokens[cursor-1];

		if(ExpectAndEatToken(";")){
			StructMember member;
			member.name = uniqueName;
			member.type = declType;
			member.offset = def->size;
			def->members.push_back(member);
			def->size += members.type.size;
			return true;
		}

		return false;
	}

	bool ExpectAndEatStructDef(){
		int oldCursor = cursor;
		if(!ExpectAndEatToken("struct")){
			cursor = oldCursor;
			return false;
		}

		if(!ExpectAndEatUniqueName()){
			cursor = oldCursor;
			return false;
		}

		if(!ExpectAndEatToken("{")){
			cursor = oldCursor;
			return false;
		}

		StructDef* def = new StructDef();
		def->name = tokens[cursor - 2];

		while(tokens[cursor] != "}"){
			if(!ExpectAndEatFieldDecl(def)){
				cursor = oldCursor;
				delete def;
				return false;
			}
		}

		if(ExpectAndEatToken(";")){
			ast->structDefs.push_back(def);
			definedTypes.insert(std::make_pair(def->name,   {def->name, def->size}));

			return true;
		}

		delete def;
		cursor = oldCursor;
		return false;
	}

	void Setup(){
		FuncDef printDef = new FuncDef();
		printDef.name = "PRINT";
		printDef.retType.name = "void";
		printDef.retType.sizeInWords = 0;
		printDef.parameters.insert(std::make_pair("val", {"int", 1}));

		FuncDef printfDef = new FuncDef();
		printfDef.name = "PRINTF";
		printfDef.retType.name = "void";
		printfDef.retType.sizeInWords = 0;
		printfDef.parameters.insert(std::make_pair("val", {"float", 1}));

		FuncDef readDef = new FuncDef();
		readDef.name = "READ";
		readDef.retType.name = "int";
		readDef.retType.sizeInWords = 1;

		FuncDef readfDef = new FuncDef();
		readfDef.name = "READF";
		readfDef.retType.name = "float";
		readfDef.retType.sizeInWords = 1;

		FuncDef ftoiDef = new FuncDef();
		ftoiDef.name = "PRINTF";
		ftoiDef.retType.name = "int";
		ftoiDef.retType.sizeInWords = 1;
		ftoiDef.parameters.insert(std::make_pair("val", {"float", 1}));

		builtinFuncs.insert(std::make_pair("PRINT",  printDef));
		builtinFuncs.insert(std::make_pair("PRINTF", printfDef));
		builtinFuncs.insert(std::make_pair("READ",   readDef));
		builtinFuncs.insert(std::make_pair("READF",  readfDef));
		builtinFuncs.insert(std::make_pair("ftoi",  ftoiDef));

		definedTypes.insert(std::make_pair("int",   {"int",   1}));
		definedTypes.insert(std::make_pair("float", {"float", 1}));
		definedTypes.insert(std::make_pair("void",  {"void",  0}));

		binaryOps.push_back("+");
		binaryOps.push_back("-");
		binaryOps.push_back("/");
		binaryOps.push_back("*");
		binaryOps.push_back("&");
		binaryOps.push_back("|");
		binaryOps.push_back("==");
		binaryOps.push_back(">");
		binaryOps.push_back("<");

		//unaryOps.push_back("!");

		ast = new AST();
	}

	bool ExpectAndEatType(){
		if(definedTypes.find(tokens[cursor]) != definedTypes.end()){
			cursor++;
			return true;
		}
	}

	bool ExpectAndEatParameter(){
		int oldCursor = cursor;

		if(!ExpectAndEatType()){
			cursor = oldCursor;
			return false;
		}

		if(!ExpectAndEatUniqueName()){
			cursor = oldCursor;
			return false;
		}

		return true;
	}

	bool ExpectAndEatStatement(Statement** out){
		if(ExpectAndEatAssignment()){

		}
	}

	bool ExpectAndEatFunctionDef(){
		int oldCursor = cursor;

		if(!ExpectAndEatType()){
			cursor = oldCursor;
			return false;
		}

		const string& retTypeName = tokens[cursor - 1];

		if(!ExpectAndEatUniqueName()){
			cursor = oldCursor;
			return false;
		}

		const string& funcName = tokens[cursor - 1];

		FuncDef* def = new FuncDef();
		const Type& retType = definedTypes.find(retTypeName)->second;
		def->retType = retType;
		def->name = funcName;

		int paramCount = 0;
		while(tokens[cursor] != ")"){
			if(!ExpectAndEatParameter()){
				delete def;
				cursor = oldCursor;
				return false;
			}

			fun

			if(paramCount != 0){
				if(!ExpectAndEatToken(",")){
					delete def;
					cursor = oldCursor;
					return false;
				}
			}

			paramCount++;
		}

		if(!ExpectAndEatToken("{")){
			delete def;
			cursor = oldCursor;
			return false;
		}

		while(tokens[cursor] != "}"){
			//if(!Expec)
		}

	}

	AST* ParseAST(){
		Setup();

		while(cursor < tokens.size()){
			if(ExpectAndEatVarDecl()){

			}
			else if(ExpectAndEatStructDef()){

			}
			else if(ExpectAndEatFunctionDef()){

			}
		}

		return ast;
	}
};

AST* MakeASTFromTokens(const vector<string>& tokens){
	TokenStream stream;
	stream.tokens = tokens;
	stream.cursor = 0;

	return stream.ParseAST();
}

void AST::GenerateFromShuntedTokens(const vector<string>& tokens, VM& vm){
	map<string, int> varRegs;
	map<string, int> funcArity;
	map<string, int> externFuncArity;
	int varCount = 0;

	vector<string> variables;
	map<string, ValueType> varTypes;

	const string operators = "+-*><|&=/";
	const string numbers = "0123456789.";

	Stack<Scope*> scopes;
	Stack<Value*> values;

	for(int i = 0; i < tokens.size(); i++){
		string token = tokens[i];
		//cout << "|" << token << "|\n";

		if(numbers.find(token[0]) != string::npos){
			if(token.find(".") == string::npos){
				values.Push(new Literal(atoi(token.c_str())));
			}
			else{
				values.Push(new FloatLiteral(atof(token.c_str())));
			}
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
				if(tokens[i] == "int"){
					varRegs.insert(std::pair<string, int>(tokens[i+1], varCount));
					varTypes.insert(std::pair<string,ValueType>(tokens[i+1], ValueType::INT));
					varCount++;
				}
				else if(tokens[i] == "float"){
					varRegs.insert(std::pair<string, int>(tokens[i+1], varCount));
					varTypes.insert(std::pair<string,ValueType>(tokens[i+1], ValueType::FLOAT));
					varCount++;
				}
				i++;
			}

			i++; // Skip '{' after "def (args...)"

			funcArity.insert(std::pair<string, int>(funcName, currDef->paramNames.size()));
			scopes.Push(currDef);
		}
		else if(token == "extern"){
			string funcName = tokens[i+1];
			i += 3;

			FuncDef* currDef = new FuncDef();
			currDef->isExtern = true;
			currDef->name = funcName;

			while(tokens[i] != ")"){
				if(tokens[i] == "int"){
					varRegs.insert(std::pair<string, int>(tokens[i+1], varCount));
					varTypes.insert(std::pair<string,ValueType>(tokens[i+1], ValueType::INT));
					varCount++;
				}
				else if(tokens[i] == "float"){
					varRegs.insert(std::pair<string, int>(tokens[i+1], varCount));
					varTypes.insert(std::pair<string,ValueType>(tokens[i+1], ValueType::FLOAT));
					varCount++;
				}
				i++;
			}

			i++; // Skip ';' after "def (args...)"
			vm.externFuncNames.push_back(funcName);
			externFuncArity.insert(std::pair<string, int>(funcName, currDef->paramNames.size()));
		}
		else if(token == "var"){ // || token == "int" || token == "float"){
			varRegs.insert(std::pair<string, int>(tokens[i+1], varCount));
			varTypes.insert(std::pair<string,ValueType>(tokens[i+1], ValueType::INT));
			varCount++;
		}
		else if(token == "int"){ // || token == "int" || token == "float"){
			varRegs.insert(std::pair<string, int>(tokens[i+1], varCount));
			varTypes.insert(std::pair<string,ValueType>(tokens[i+1], ValueType::INT));
			varCount++;
		}
		else if(token == "float"){ // || token == "int" || token == "float"){
			varRegs.insert(std::pair<string, int>(tokens[i+1], varCount));
			varTypes.insert(std::pair<string,ValueType>(tokens[i+1], ValueType::FLOAT));
			varCount++;
		}
		else if(token == "PRINT" || token == "READ" || token == "READF" || token == "return" || token == "RETURN" || token == "itof"){
			Builtin* builtin = new Builtin(token);
			for(int i = 0; i < builtin->numParams; i++){
				builtin->AddParameter(values.Pop());
			}

			values.Push(builtin);
		}
		else if(funcArity.find(token) != funcArity.end()){
			FuncCall* func = new FuncCall();
			func->funcName = token;
			func->numParams = funcArity[token];
			func->varCount = varCount;
			for(int i = 0; i < funcArity[token]; i++){
				func->AddParameter(values.Pop());
			}

			values.Push(func);
		}
		else if(std::find(vm.externFuncNames.begin(), vm.externFuncNames.end(), token) != vm.externFuncNames.end()){
			ExternFunc* func = new ExternFunc();
			func->funcName = token;
			func->numParams = externFuncArity[token];

			for(int i = 0; i < func->numParams; i++){
				func->AddParameter(values.Pop());
			}

			values.Push(func);
		}
		else if(varTypes.find(token) != varTypes.end()){
			Variable* var = new Variable();
			var->varName = token;
			var->type = varTypes.find(token)->second;
			var->reg = varRegs[token];
			values.Push(var);
		}
		else if(token == ":"){
			Assignment* assmt = new Assignment();
			assmt->val = values.Pop();
			Variable* reg = (Variable*)values.Pop();
			assmt->reg = reg->reg;
			assmt->varName = reg->varName;
			assmt->type = reg->type;

			values.Push(assmt);
		}
		else if(token == ";"){
			if(scopes.stackSize > 0){
				scopes.Peek()->AddStatement(values.Pop());
			}
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
	vm.byteCodeLoaded.push_back(INT_DLIT);
	int retAddrIdx = vm.byteCodeLoaded.size();
	vm.byteCodeLoaded.push_back(253); //Return Addr?
	vm.byteCodeLoaded.push_back(253); //Return Addr?

	for(int i = 0; i < numParams; i++){
		parameterVals[i]->AddByteCode(vm);
	}

	int funcAddr = vm.funcPointers.find(funcName)->second;

	vm.byteCodeLoaded.push_back(INT_LIT);
	vm.byteCodeLoaded.push_back(varCount);
	vm.byteCodeLoaded.push_back(INT_DLIT);
	vm.byteCodeLoaded.push_back(funcAddr / 256);
	vm.byteCodeLoaded.push_back(funcAddr % 256);
	vm.byteCodeLoaded.push_back(CALL);

	vm.byteCodeLoaded[retAddrIdx]   = vm.byteCodeLoaded.size() / 256;
	vm.byteCodeLoaded[retAddrIdx+1] = vm.byteCodeLoaded.size() % 256;
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
	else if(funcName == "READF"){
		vm.byteCodeLoaded.push_back(READF);
	}
	else if(funcName == "return"){
		vm.byteCodeLoaded.push_back(RETURN);
	}
	else if(funcName == "RETURN"){
		vm.byteCodeLoaded.push_back(RETURN_FINAL);
	}
	else if(funcName == "itof"){
		vm.byteCodeLoaded.push_back(INT_TO_FLT);
	}
}

int Assignment::Evaluate(){
	return -1;
}

void Assignment::AddByteCode(VM& vm){
	Literal(reg).AddByteCode(vm);
	val->AddByteCode(vm);
	vm.byteCodeLoaded.push_back(SAVE_REG);
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

void FloatLiteral::AddByteCode(VM& vm){
	unsigned char* castPtr = (unsigned char*)&value;
	vm.byteCodeLoaded.push_back(FLT_LIT);
	for(int i = 0; i < 4; i++){
		vm.byteCodeLoaded.push_back(castPtr[i]);
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
	if(!isExtern){
		for(int paramIdx = paramNames.size() - 1; paramIdx >= 0; paramIdx--){
			vm.byteCodeLoaded.push_back(INT_LIT);
			vm.byteCodeLoaded.push_back(paramIdx);
			vm.byteCodeLoaded.push_back(PARAM);
		}
		for(int i = 0; i < numStatements; i++){
			statements[i]->AddByteCode(vm);
		}
	}
}

int ExternFunc::Evaluate(){
	return -1;
}

void ExternFunc::AddByteCode(VM& vm){
	int index = -1;
	for(int i = 0; i < vm.externFuncNames.size(); i++){
		if(vm.externFuncNames[i] == funcName){
			index = i;
			break;
		}
	}

	if(index == -1){
		cout << "Could not find extern function with the name '" << funcName << "'\n";
	}
	else{
		if(numParams != 1){
			cout << "For now, number of params for extern must be 1.\n";
		}
		parameterVals[0]->AddByteCode(vm);
		vm.byteCodeLoaded.push_back(INT_LIT);
		vm.byteCodeLoaded.push_back(index);
		vm.byteCodeLoaded.push_back(EXTERN_CALL);
	}
}

int IfStatement::Evaluate(){
	return -1;
}

void IfStatement::AddByteCode(VM& vm){
	test->AddByteCode(vm);
	vm.byteCodeLoaded.push_back(INT_DLIT);
	int jumpAddrIdx = vm.byteCodeLoaded.size();
	vm.byteCodeLoaded.push_back(253);
	vm.byteCodeLoaded.push_back(253);
	vm.byteCodeLoaded.push_back(BRANCH);

	for(int i = 0; i < numStatements; i++){
		statements[i]->AddByteCode(vm);
	}

	vm.byteCodeLoaded[jumpAddrIdx]   = vm.byteCodeLoaded.size() / 256;
	vm.byteCodeLoaded[jumpAddrIdx+1] = vm.byteCodeLoaded.size() % 256;
}

int WhileStatement::Evaluate(){
	return -1;
}

void WhileStatement::AddByteCode(VM& vm){
	int recurAddrIdx = vm.byteCodeLoaded.size();

	test->AddByteCode(vm);
	vm.byteCodeLoaded.push_back(INT_DLIT);
	int jumpAddrIdx = vm.byteCodeLoaded.size();
	vm.byteCodeLoaded.push_back(253);
	vm.byteCodeLoaded.push_back(253);
	vm.byteCodeLoaded.push_back(BRANCH);

	for(int i = 0; i < numStatements; i++){
		statements[i]->AddByteCode(vm);
	}

	vm.byteCodeLoaded.push_back(INT_LIT);
	vm.byteCodeLoaded.push_back(0);
	vm.byteCodeLoaded.push_back(INT_DLIT);
	vm.byteCodeLoaded.push_back(recurAddrIdx / 256);
	vm.byteCodeLoaded.push_back(recurAddrIdx % 256);
	vm.byteCodeLoaded.push_back(BRANCH);

	vm.byteCodeLoaded[jumpAddrIdx]   = vm.byteCodeLoaded.size() / 256;
	vm.byteCodeLoaded[jumpAddrIdx+1] = vm.byteCodeLoaded.size() % 256;
}
