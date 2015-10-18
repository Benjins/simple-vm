#ifndef AST_H
#define AST_H

#include <string>
#include <cstring>
#include "Parser.h"
#include "VM.h"

using std::string;

struct VM; struct FuncDef; struct Value; struct Statement; struct StructDef; struct AST;

Statement* CompileTokenToAST(const string& token);

AST* MakeASTFromTokens(const vector<string>& tokens);

struct Type{
	string name;
	int sizeInWords;
};

struct AST{
	vector<FuncDef*> defs;
	vector<StructDef*> structDefs;

	void GenerateByteCode(VM& vm);
	bool TypeCheck();
};

struct Expression{
	virtual int Evaluate() = 0;
	virtual void AddByteCode(VM& vm) = 0;
	virtual bool TypeCheck() = 0;
};

struct Statement : public Expression{
	virtual int Evaluate() = 0;
	virtual void AddByteCode(VM& vm) = 0;
	virtual int NumParams() = 0;
	virtual bool TypeCheck() = 0;
};

struct Value : public Statement{
	Type type;
	virtual int Evaluate() = 0;
	virtual void AddByteCode(VM& vm) = 0;
	virtual int NumParams() = 0;
	virtual bool TypeCheck() = 0;
};

struct FuncCall : public Value{
	string funcName;
	Value** parameterVals;
	FuncDef* def;
	int numParams;
	int currParams;
	int varCount;

	FuncCall(){
		funcName = "";
		parameterVals = nullptr;
		currParams = 0;
		numParams = 0;
		def = nullptr;
	}

	void AddParameter(Value* newVal){
		Value** newParameterVals = new Value*[++currParams];
		if(parameterVals != NULL){
			memcpy(newParameterVals, parameterVals, sizeof(Value*) * (currParams - 1));
			delete[] parameterVals;
		}
		newParameterVals[currParams - 1] = newVal;
		parameterVals = newParameterVals;
	}

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){
		return numParams;
	}

	virtual bool TypeCheck();
};

struct Builtin : public FuncCall{

	Builtin(const string& name){
		funcName = name;
		currParams = 0;

		if(funcName == "PRINT"){
			numParams = 1;
		}
		else if(funcName == "PRINTF"){
			numParams = 1;
		}
		else if(funcName == "READ"){
			numParams = 0;
		}
		else if(funcName == "READF"){
			numParams = 0;
		}
		else if(funcName == "return"){
			numParams = 1;
		}
		else if(funcName == "RETURN"){
			numParams = 1;
		}
		else if(funcName == "itof"){
			numParams = 1;
		}
	}

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){
		if(funcName == "PRINT"){
			return 1;
		}
		else if(funcName == "READ"){
			return 0;
		}
	}

	virtual bool TypeCheck(){
		bool retVal = true;
		for(int i = 0; i < numParams; i++){
			retVal &= parameterVals[i]->TypeCheck();
		}

		if(!retVal){
			return false;
		}

		if(funcName == "PRINT"){
			type.name = "void";
			type.sizeInWords = 0;
			retVal &= (numParams == 1);
			retVal &= (parameterVals[0]->type.name == "int");
		}
		else if(funcName == "PRINTF"){
			type.name = "void";
			type.sizeInWords = 0;
			retVal &= (numParams == 1);
			retVal &= (parameterVals[0]->type.name == "float");
		}
		else if(funcName == "READ"){
			type.name = "int";
			type.sizeInWords = 1;
			retVal &= (numParams == 0);
		}
		else if(funcName == "READF"){
			type.name = "float";
			type.sizeInWords = 1;
			retVal &= (numParams == 0);
		}
		else if(funcName == "return"){
			type = parameterVals[0]->type;
			retVal &= (numParams == 1);
		}
		else if(funcName == "itof"){
			type.name = "float";
			type.sizeInWords = 1;
			retVal &= (numParams == 1);
			retVal &= (parameterVals[0]->type.name == "int");
		}

		return retVal;
	}
};

struct Assignment : public Value{
	int reg;
	string varName;
	Type varType;

	Value* val;

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){return 1;}

	virtual bool TypeCheck(){
		bool retVal = val->TypeCheck();
		retVal &= (val->type.name == varType.name);

		return retVal;
	}
};

struct Literal : public Value{
	int value;

	Literal(int _value = 0){
		value = _value;
	}

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){
		return 0;
	}

	virtual bool TypeCheck(){
		type.name = "int";
		type.sizeInWords = 1;
		return true;
	}
};

struct FloatLiteral : public Value{
	float value;

	FloatLiteral(float _value = 0){
		value = _value;
	}

	virtual int Evaluate(){
		return (int)value;
	}
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){
		return 0;
	}

	virtual bool TypeCheck(){
		type.name = "float";
		type.sizeInWords = 1;
		return true;
	}
};

struct Variable : public Value{
	int _reg;
	string varName;

	virtual int GetRegister(){return _reg;}
	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){
		return 0;
	}

	virtual bool TypeCheck(){
		return true;
	}
};

struct FieldAccess : public Variable{
	Variable* variable;
	string fieldName;
	int offset;

	virtual int GetRegister(){
		int reg = variable->GetRegister() + offset;
		return reg;
	}
	virtual int Evaluate(){return -1;}
	virtual int NumParams(){
		return 0;
	}

	virtual bool TypeCheck(){
		return variable->TypeCheck();
	}
};

struct Operator : public Value{
	unsigned char instr;

	Value* left;
	Value* right;

	Operator(string opName = ""){
		if(opName != ""){
			instr = Compile(opName);
		}
	}

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){
		return 2;
	}

	virtual bool TypeCheck();
};


struct Scope : public Statement{
	Statement** statements;

	int numStatements;

	map<string, Type> variablesInScope;

	Scope(){
		numStatements = 0;
		statements = NULL;
	}

	void AddStatement(Statement* newStmt){
		Statement** newStatements = new Statement*[++numStatements];
		if(statements != NULL){
			memcpy(newStatements, statements, sizeof(Statement*) * (numStatements - 1));
			delete[] statements;
		}
		newStatements[numStatements-1] = newStmt;
		statements = newStatements;
	}

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){
		return 0;
	}

	virtual bool TypeCheck() override{
		bool retVal = true;
		for(int i = 0; i < numStatements; i++){
			retVal &= statements[i]->TypeCheck();
			retVal &= statements[i]->TypeCheck();
		}

		return retVal;
	}
};

struct FuncDef : public Scope{
	string name;
	map<string, Type> parameters;
	Type retType;

	bool isExtern;

	FuncDef(){isExtern = false;}

	vector<string> paramNames;

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
};

struct StructMember{
	string name;
	Type type;
	int offset;
};

struct StructDef{
	string name;
	int size;
	vector<StructMember> members;

	StructDef() : members(), name(), size(0){
	}
};

struct ExternFunc : public Value{
	string funcName;
	Value** parameterVals;
	int numParams;
	int currParams;

	ExternFunc(){
		funcName = "";
		parameterVals = nullptr;
		numParams = 0;
		currParams = 0;
	}

	void AddParameter(Value* newVal){
		Value** newParameterVals = new Value*[++currParams];
		if(parameterVals != NULL){
			memcpy(newParameterVals, parameterVals, sizeof(Value*) * (currParams - 1));
			cout << "Lol.\n";
			delete[] parameterVals;
			cout << "Done.\n";
		}
		newParameterVals[currParams - 1] = newVal;
		parameterVals = newParameterVals;
	}

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){
		return numParams;
	}

	virtual bool TypeCheck() override{
		return true;
	}
};

struct IfStatement : public Scope{
	Value* test;

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);

	virtual bool TypeCheck() override{
		bool testCheck = test->TypeCheck();

		if(testCheck && test->type.name == "int"){
			return Scope::TypeCheck();
		}

		return false;
	}
};

struct WhileStatement : public IfStatement{

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
};

#endif
