#pragma once

#include <vector>

namespace L1 {

  struct L1_item {
    std::string labelName;
  };

  struct Instruction {
    std::string instruction;
    int64_t type;
    std::vector<std::string> registers;
    std::vector<std::string> operation;
  };

  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<L1::Instruction *> instructions;
  };

  struct Program{
    std::string entryPointLabel;
    std::vector<L1::Function *> functions;
  };

} // L1
