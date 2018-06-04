#pragma once

#include <LA.h>


namespace LA {
	std::string decodeArg(Function*, Arg*, std::vector<Instruction*>*);
	std::string encodeArg(Function*, Arg*, std::vector<Instruction*>*);
	Arg* var_exists(Function*, Arg*);
	void parse_instruction(Function*, Instruction*, std::vector<Instruction *>*);
	void check_memory_access(Instruction_Assignment*, std::vector<Instruction*>*);
	void generate_basic_blocks(Function*, std::vector<Instruction *>*);
	void number_instructions(Function*);
  	void LA_generate_code(Program);
}
