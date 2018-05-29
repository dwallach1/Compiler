#pragma once

#include <LA.h>


namespace LA {
	std::string decodeName(std::string name);
	std::string encodeName(std::string name);
	void check_memory_access(Instruction_Assignment* I);
	void number_instructions(Function* f);
  	void LA_generate_code(Program p);
}
