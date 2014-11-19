#include "../header/VM.h"
#include "../header/Instruction.h"

int main(){
	unsigned char basicProgram[8];
	basicProgram[0] = Instruction::INT_LIT;
	basicProgram[1] = 8;
	basicProgram[2] = Instruction::READ;
	basicProgram[3] = Instruction::INT_MUL;
	basicProgram[4] = Instruction::PRINT;

	VM x;
	x.Execute(basicProgram, 5);


	return 0;
}