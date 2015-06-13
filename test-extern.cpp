#include "header/VM.h"
#include <iostream>
#include <cmath>

extern "C"{

VMValue Lolinomial(VMValue x){
	if(x.type != ValueType::INT){
		std::cout << "Warning: calling 'Lolinomial' on non-integer type.\n";
	}

	VMValue ret;
	ret.intValue = pow(2, x.intValue);
	ret.type = ValueType::INT;
	return ret;
}

VMValue LoliLoli(VMValue x){
	if(x.type != ValueType::FLOAT){
		std::cout << "Warning: calling 'LoliLoli' on non-float type.\n";
	}

	VMValue ret;
	ret.floatValue = pow(x.floatValue, x.floatValue);
	ret.type = ValueType::FLOAT;
	return ret;
}

}