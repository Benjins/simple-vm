#include "../header/VM.h"
#include "../header/Instruction.h"

int main(){
	//Multiply a given input by 270
	unsigned char basicProgram[8];
	basicProgram[0] = Instruction::INT_DLIT;
	basicProgram[1] = 1;
	basicProgram[2] = 14; //1 * 256 + 14 = 270
	basicProgram[3] = Instruction::READ;
	basicProgram[4] = Instruction::INT_MUL;
	basicProgram[5] = Instruction::PRINT;

	VM x;
	x.Execute(basicProgram, 6);


	return 0;
}