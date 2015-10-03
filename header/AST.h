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
};

struct Expression{
	virtual int Evaluate() = 0;
	virtual void AddByteCode(VM& vm) = 0;
};

struct Statement : public Expression{
	virtual int Evaluate() = 0;
	virtual void AddByteCode(VM& vm) = 0;
	virtual int NumParams() = 0;
};

struct Value : public Statement{
	Type type;
	virtual int Evaluate() = 0;
	virtual void AddByteCode(VM& vm) = 0;
	virtual int NumParams() = 0;
};

struct FuncCall : public Value{
	string funcName;
	Value** parameterVals;
	int numParams;
	int currParams;
	int varCount;

	FuncCall(){
		funcName = "";
		parameterVals = NULL;
		currParams = 0;
		numParams = 0;
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
};

struct Assignment : public Value{
	int reg;
	string varName;

	Value* val;

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){return 1;}
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
};

struct Variable : public Value{
	int _reg;
	string varName;
	Type varType;

	virtual int GetRegister(){return _reg;}
	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
	virtual int NumParams(){
		return 0;
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
};

struct IfStatement : public Scope{
	Value* test;

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
};

struct WhileStatement : public IfStatement{

	virtual int Evaluate();
	virtual void AddByteCode(VM& vm);
};

#endif
