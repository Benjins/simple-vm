#ifndef INSTRUCTION_H
#define INSTRUCTION_H

enum Instruction{
	INT_ADD = 1,
	INT_MUL = 2,
	INT_SUB = 3,
	INT_LIT = 4,
	INT_DLIT = 5,
	INT_QLIT = 23,
	PRINT = 6,
	READ = 7,
	L_THAN = 8,
	G_THAN = 9,
	BOOL_AND = 10,
	BOOL_OR = 11,
	ASSIGN = 12,
	COMPARE = 13,
	BRANCH = 14,
	LOAD_REG = 15,
	SAVE_REG = 16,
	INT_DIV = 17,
	CALL = 18,
	RETURN = 19,
	STK_FRAME = 20,
	PARAM = 21,
	RETURN_FINAL = 22,
	FLT_LIT = 23,
	INT_TO_FLT = 24,
	READF = 25,
	PRINTF = 26,
	EXTERN_CALL = 27
};


/*
Possible future instructions:
-float operations
-return values
-division/subtraction
-subroutines
-declarations
*/

#endif
