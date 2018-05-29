#pragma once

#include <LA.h>


namespace LA {
	void decodeArg(Arg*, std::vector<Instruction*>*);
	void encodeArg(Arg*, std::vector<Instruction*>*);
	void check_memory_access(Instruction_Assignment* I, std::vector<Instruction*>*);
	void number_instructions(Function* f);
  	void LA_generate_code(Program p);
}
