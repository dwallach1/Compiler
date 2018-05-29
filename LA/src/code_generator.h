#pragma once

#include <LA.h>


namespace LA {
	string decodeName(string name);
	string encodeName(string name);
	void check_memory_access(Instruction_Assignment* I);
	void number_instructions(Function* f);
  	void LA_generate_code(Program p);
}
