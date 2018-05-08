#pragma once

#include <L3.h>


namespace L3{

  void L3_generate_code(Program p);
  void updateArgumentsAndLocals(Function* f);
  std::string convert_function(Function* f);
  std::string convert_instruction(Instruction* I);

}
