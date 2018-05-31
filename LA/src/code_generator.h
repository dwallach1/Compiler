#pragma once

#include <LA.h>


namespace LA {
	std::string decodeArg(Arg*, std::vector<Instruction*>*);
	std::string encodeArg(Arg*, std::vector<Instruction*>*);
	void check_memory_access(Instruction_Assignment* I, std::vector<Instruction*>*);
	void generate_basic_blocks(Function* f, std::vector<Instruction *>* newInsts);
	void number_instructions(Function* f);
  	void LA_generate_code(Program p);
}
