#pragma once

#include <vector>

namespace L2 {

  struct L2_item {
    std::string labelName;
  };

  struct Instruction {
    std::string instruction;
    int64_t type;
    std::vector<std::string> registers;
    std::vector<std::string> operation;

    std::vector<std::string> gen;
    std::vector<std::string> kill;

    std::vector<std::string> in;
    std::vector<std::string> out;
  };

  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<L2::Instruction *> instructions;
  };

  struct Program{
    std::string entryPointLabel;
    std::vector<L2::Function *> functions;
  };

  struct DataFlowResult {
      std::string result; 
  }

} // L2
