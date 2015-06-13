#include <iostream>

extern "C" {

struct MaxType{
	int myInt;
	float myFloat;
};

void PrintTest(MaxType a){
	std::cout << "Yo, the print test worked.  " << a.myInt << "   " << a.myFloat << "\n";
}

}