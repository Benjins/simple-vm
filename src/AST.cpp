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
	map<string, int> varRegs;

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
		if(definedFuncs.find(tokens[cursor]) == definedFuncs.end() && variables.find(tokens[cursor]) == variables.end()){
			cursor++;
			return true;
		}
		return false;
	}

	bool ExpectAndEatVariable(){
		if(variables.find(tokens[cursor]) != variables.end()){
			cursor++;
			return true;
		}

		return false;
	}

	bool ExpectAndEatField(StructDef* structType, StructMember* out){
		int oldCursor = cursor;

		for(auto& member : structType->members){
			if(member.name == tokens[cursor]){
				*out = member;
				cursor++;
				return true;
			}
		}

		cursor = oldCursor;
		return false;
	}

	bool ExpectAndEatAssignment(){
		int oldCursor = cursor;
		bool isDeclaration = false;

		if(!ExpectAndEatVariable()){
			cursor = oldCursor;
			if(!ExpectAndEatType()){
				cursor = oldCursor;
				return false;
			}
			else{
				if(!ExpectAndEatUniqueName()){
					cursor = oldCursor;
					return false;
				}
				else{
					const string& typeName = tokens[cursor-2];
					const string& varName  = tokens[cursor-1];
					const Type& varType = definedTypes.find(typeName)->second;
					AddVariable(varType, varName);
					isDeclaration = true;
				}
			}
		}

		string varName = tokens[cursor-1];
		Type varType = variables.find(varName)->second;
		StructDef* structDef = FindStructByName(varType.name);

		int offset = 0;

		if(!isDeclaration && structDef != nullptr){
			while(structDef != nullptr){
				if(ExpectAndEatToken(".")){
					StructMember member;
					if(ExpectAndEatField(structDef, &member)){
						offset += member.offset;

						const Type& fieldType = member.type;
						structDef = FindStructByName(fieldType.name);
					}
				}
				else{
					break;
				}
			}
		}

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
		assgn->reg = varRegs.find(varName)->second + offset;
		assgn->varName = varName;
		assgn->val = val;

		scopes.Peek()->AddStatement(assgn);

		return true;
	}

	void AddVariable(const Type& type, const string& name){
		stackSizeInWords += type.sizeInWords;
		varRegs.insert(std::make_pair(name, stackSizeInWords - 1));
		variables.insert(std::make_pair(name, type));
		if(scopes.stackSize > 0){
			scopes.Peek()->variablesInScope.insert(std::make_pair(name, type));
		}
	}

	void RemoveVariable(const string& name){
		auto variable = variables.find(name);
		stackSizeInWords -= variable->second.sizeInWords;
		variables.erase(variable);

		varRegs.erase(varRegs.find(name));
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
				if(token.find(".") != string::npos){
					out->type = ValueType::FLOAT;
					out->floatValue = atof(token.c_str());
				}
				else{
					out->type = ValueType::INT;
					out->intValue = atoi(token.c_str());
				}
			}
			return true;
		}

		return false;
	}

	bool EatFieldName(){
		return true;
	}

	StructDef* FindStructByName(const string& name){
		for(StructDef* astStructDef : ast->structDefs){
			if(astStructDef->name == name){
				return astStructDef;
				break;
			}
		}

		return nullptr;
	}

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

		vector<string> shuntedTokens = JustShuntingYard(valueTokens, definedFuncs, builtinFuncs);

		Stack<Value*> values;
		Stack<string> operatorStack;
		for(int index = 0; index < shuntedTokens.size(); index++){
			const string& str = shuntedTokens[index];
			VMValue val;
			if(ExpectAndConvertNumber(str, &val)){
				//push num onto stack
				if(val.type == ValueType::INT){
					values.Push(new Literal(val.intValue));
				}
				else{
					values.Push(new FloatLiteral(val.floatValue));
				}
			}
			else if(FIND(binaryOps, str) != binaryOps.end()){
				Operator* op = new Operator(str);
				op->right = values.Pop();
				op->left  = values.Pop();
				values.Push(op);
			}
			else if(builtinFuncs.find(str) != builtinFuncs.end() ){
				Builtin* builtin = new Builtin(str);
				FuncDef* def = builtinFuncs.find(str)->second;

				for(int i = 0; i < def->parameters.size(); i++){
					if(values.stackSize == 0){
						cout << "\n\nError, function '" << str << "' takes " << def->parameters.size() << " arguments.\n";
						return false;
					}
					builtin->AddParameter(values.Pop());
				}

				values.Push(builtin);
			}
			else if(definedFuncs.find(str) != definedFuncs.end()){
				FuncCall* call = new FuncCall();
				call->funcName = str;
				call->varCount = stackSizeInWords;
				FuncDef* def = definedFuncs.find(str)->second;
				for(int i = 0; i < def->parameters.size(); i++){
					if(values.stackSize == 0){
						cout << "\n\nError, function '" << str << "' takes " << def->parameters.size() << " arguments.\n";
						return false;
					}
					call->AddParameter(values.Pop());
				}

				values.Push(call);
			}
			else if(variables.find(str) != variables.end()){
				Variable* var = new Variable();
				var->varName = str;
				var->_reg = varRegs.find(str)->second;
				var->varType = definedTypes.find(variables.find(str)->second.name)->second;
				values.Push(var);
			}
			else if(str == "."){
				string fieldName = operatorStack.Pop();
				Value* val = values.Pop();
				Variable* var = static_cast<Variable*>(val);

				FieldAccess* fieldAccess = new FieldAccess();
				fieldAccess->fieldName = fieldName;
				fieldAccess->variable = var;
				StructDef* def = FindStructByName(var->varType.name);
				if(def == nullptr){
					printf("\nError: Tried to use '.' operator on variable '%s' of type: '%s'\n", var->varName, var->varType.name);
					break;
				}
				else{
					for(auto& member : def->members){
						if(member.name == fieldName){
							fieldAccess->varType = member.type;
							fieldAccess->offset = member.offset;
							values.Push(fieldAccess);
							break;
						}
					}
				}
			}
			else{
				operatorStack.Push(str);
			}
		}

		if(values.stackSize == 1){
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
			Value* val;
			if(ExpectAndEatValue(&val)){
				AddVariable(declType, uniqueName);

				Assignment* assgn = new Assignment();
				assgn->varName = uniqueName;
				assgn->reg = varRegs.find(uniqueName)->second;
				assgn->val = val;

				scopes.Peek()->AddStatement(assgn);
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
			def->size += member.type.sizeInWords;
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
		if(ExpectAndEatToken("}")){
			if(ExpectAndEatToken(";")){
				ast->structDefs.push_back(def);
				Type type =  {def->name, def->size};
				definedTypes.insert(std::make_pair(def->name, type));

				return true;
			}
		}

		delete def;
		cursor = oldCursor;
		return false;
	}

	void Setup(){
		Type intType = {"int", 1};
		Type floatType = {"float", 1};
		Type voidType = {"void", 0};

		FuncDef* printDef = new FuncDef();
		printDef->name = "PRINT";
		printDef->retType.name = "void";
		printDef->retType.sizeInWords = 0;
		printDef->parameters.insert(std::make_pair("val", intType));

		FuncDef* printfDef = new FuncDef();
		printfDef->name = "PRINTF";
		printfDef->retType.name = "void";
		printfDef->retType.sizeInWords = 0;
		printfDef->parameters.insert(std::make_pair("val", floatType));

		FuncDef* readDef = new FuncDef();
		readDef->name = "READ";
		readDef->retType.name = "int";
		readDef->retType.sizeInWords = 1;

		FuncDef* readfDef = new FuncDef();
		readfDef->name = "READF";
		readfDef->retType.name = "float";
		readfDef->retType.sizeInWords = 1;

		FuncDef* ftoiDef = new FuncDef();
		ftoiDef->name = "PRINTF";
		ftoiDef->retType.name = "int";
		ftoiDef->retType.sizeInWords = 1;
		ftoiDef->parameters.insert(std::make_pair("val", floatType));

		builtinFuncs.insert(std::make_pair("PRINT",  printDef));
		builtinFuncs.insert(std::make_pair("PRINTF", printfDef));
		builtinFuncs.insert(std::make_pair("READ",   readDef));
		builtinFuncs.insert(std::make_pair("READF",  readfDef));
		builtinFuncs.insert(std::make_pair("ftoi",  ftoiDef));

		definedTypes.insert(std::make_pair("int",   intType));
		definedTypes.insert(std::make_pair("float", floatType));
		definedTypes.insert(std::make_pair("void",  voidType));

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

		stackSizeInWords = 0;

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

	bool ExpectAndEatControlStatement();

	bool ExpectAndEatStatement(){
		int oldCursor = cursor;
		
		if(ExpectAndEatType()){
			if(ExpectAndEatUniqueName()){
				if(ExpectAndEatToken(";")){
					const string& typeName = tokens[cursor - 3];
					const string& varName  = tokens[cursor - 2];
					const Type& varType = definedTypes.find(typeName)->second;
					AddVariable(varType, varName);
					return true;
				}
			}
		}

		cursor = oldCursor;

		if(ExpectAndEatToken("return")){
			Value* val;
			if(ExpectAndEatValue(&val)){
				if(ExpectAndEatToken(";")){
					Builtin* builtin = new Builtin("return");
					builtin->AddParameter(val);
					scopes.Peek()->AddStatement(builtin);
					return true;
				}
				else{
					delete val;
					cursor = oldCursor;
					return false;
				}
			}
			else{
				cursor = oldCursor;
				return false;
			}
		}

		if(ExpectAndEatControlStatement()){
			return true;
		}

		if(ExpectAndEatAssignment()){
			return true;
		}
		
		Value* val;
		if(ExpectAndEatValue(&val)){
			if(ExpectAndEatToken(";")){
				scopes.Peek()->AddStatement(val);
				return true;
			}
			else{
				delete val;
				cursor = oldCursor;
				return false;
			}
		}

		cursor = oldCursor;
		return false;
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

		if(!ExpectAndEatToken("(")){
			cursor = oldCursor;
			return false;
		}

		FuncDef* def = new FuncDef();
		const Type& retType = definedTypes.find(retTypeName)->second;
		def->retType = retType;
		def->name = funcName;

		scopes.Push(def);

		int paramCount = 0;
		while(tokens[cursor] != ")"){
			if(paramCount != 0){
				if(!ExpectAndEatToken(",")){
					delete def;
					scopes.Pop();
					cursor = oldCursor;
					return false;
				}
			}

			if(!ExpectAndEatParameter()){
				delete def;
				cursor = oldCursor;
				return false;
			}

			const string& paramTypeName = tokens[cursor - 2];
			const string& paramName = tokens[cursor - 1];
			const Type& paramType = definedTypes.find(paramTypeName)->second;

			AddVariable(paramType, paramName);

			def->parameters.insert(std::make_pair(paramName, paramType));

			paramCount++;
		}

		if(!ExpectAndEatToken(")")){
			delete def;
			cursor = oldCursor;
			return false;
		}

		if(!ExpectAndEatToken("{")){
			delete def;
			cursor = oldCursor;
			return false;
		}

		
		ast->defs.push_back(def);
		definedFuncs.insert(std::make_pair(funcName, def));

		while(tokens[cursor] != "}"){
			if(!ExpectAndEatStatement()){
				definedFuncs.erase(definedFuncs.find(funcName));
				ast->defs.erase(FIND(ast->defs, def));
				delete def;
				cursor = oldCursor;
				return false;
			}
		}

		if(!ExpectAndEatToken("}")){
				definedFuncs.erase(definedFuncs.find(funcName));
				ast->defs.erase(FIND(ast->defs, def));
				delete def;
				cursor = oldCursor;
				return false;
			}

		scopes.Pop();
		for(const auto& variableInScope : def->variablesInScope){
			RemoveVariable(variableInScope.first);
		}

		return true;
	}

	AST* ParseAST(){
		Setup();

		while(cursor < tokens.size()){
			if(ExpectAndEatStructDef()){

			}
			else if(ExpectAndEatVarDecl()){

			}
			else if(ExpectAndEatFunctionDef()){

			}
			else{
				cout << "\nError: could not parse: '" << tokens[cursor]<< "'\n";
				break;
			}
		}

		return ast;
	}
};

bool TokenStream::ExpectAndEatControlStatement(){
	bool isWhile = false;
	int oldCursor = cursor;
	if(ExpectAndEatToken("if")){
		isWhile = false;
	}
	else if(ExpectAndEatToken("while")){
		isWhile = true;
	}
	else{
		cursor = oldCursor;
		return false;
	}

	if(!ExpectAndEatToken("(")){
		cursor = oldCursor;
		return false;
	}

	Value* val;
	if(!ExpectAndEatValue(&val)){
		cursor = oldCursor;
		return false;
	}

	if(!ExpectAndEatToken(")")){
		delete val;
		cursor = oldCursor;
		return false;
	}

	if(!ExpectAndEatToken("{")){
		delete val;
		cursor = oldCursor;
		return false;
	}

	IfStatement* ifStmt;
	if(isWhile){
		ifStmt = new WhileStatement();
	}
	else{
		ifStmt = new IfStatement();
	}

	scopes.Peek()->AddStatement(ifStmt);
	scopes.Push(ifStmt);

	while(tokens[cursor] != "}"){
		if(!ExpectAndEatStatement()){
			delete ifStmt;
			cursor = oldCursor;
			return false;
		}
	}

	if(!ExpectAndEatToken("}")){
		delete val;
		delete ifStmt;
		cursor = oldCursor;
		return false;
	}

	scopes.Pop();
	for(const auto& variableInScope : ifStmt->variablesInScope){
		RemoveVariable(variableInScope.first);
	}

	ifStmt->test = val;

	return true;
}

AST* MakeASTFromTokens(const vector<string>& tokens){
	TokenStream stream;
	stream.tokens = tokens;
	stream.cursor = 0;

	return stream.ParseAST();
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

	for(int i = 0; i < currParams; i++){
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
	else if(funcName == "PRINTF"){
		vm.byteCodeLoaded.push_back(PRINTF);
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
	Literal(GetRegister()).AddByteCode(vm);
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
		for(int paramIdx = parameters.size() - 1; paramIdx >= 0; paramIdx--){
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
