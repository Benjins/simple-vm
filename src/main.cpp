#include "../header/VM.h"
#include "../header/Instruction.h"

int main(){

	unsigned char code[6] = {INT_DLIT, 1, 14, READ, INT_MUL, PRINT};

	VM vm;
	vm.Execute(code, 6);

    return 0;
}
