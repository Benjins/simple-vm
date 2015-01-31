#ifndef INSTRUCTION_H
#define INSTRUCTION_H

enum Instruction{
	INT_ADD = 1,
	INT_MUL = 2,
	INT_INV = 3,
	INT_LIT = 4,
	INT_DLIT = 5,
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
