#pragma once

#include <vector>
#include <set>

namespace L2 {

  struct Instruction;
  struct Variable;
  struct Function;
  struct Program;
  struct L2_item;
  struct DataFlowResult;

  struct L2_item {
    std::string labelName;
  };

  struct Instruction {
    std::string instruction;
    int64_t type;

    Instruction* prevInst;
    Instruction* nextInst;

    std::set<L2::Variable*> vars;
    
    std::vector<std::string> registers;
    std::vector<std::string> operation;

    std::vector<std::string> gen;
    std::vector<std::string> kill;

    std::vector<std::string> in;
    std::vector<std::string> out;
  };

  struct Variable {
    std::string name;
    std::set<std::string> edges;
    std::set<L2::Instruction*> uses;
  };

  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    std::vector<L2::Instruction *> instructions;
    std::set<std::string> vars;
    std::set<L2::Variable*> variables;
  };

  struct Program{
    std::string entryPointLabel;
    std::vector<L2::Function *> functions;
  };

  struct DataFlowResult {
      std::string result; 
  };

} // L2
