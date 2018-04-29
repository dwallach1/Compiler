#pragma once

#include <vector>
#include <set>

namespace L2 {
  struct L2_item;
  struct Variable;
  struct InterferenceGraph;
  struct Instruction;
  struct Function;
  struct Program;
  struct DataFlowResult;


  struct L2_item {
    std::string labelName;
  };
 
  struct Variable {
      int type;
      std::string name;
      std::set<std::string> edges;
      std::vector<L2::Instruction*> uses;
  };
  
  struct InterferenceGraph {
      std::set<L2::Variable*> variables;

  };

  struct Instruction {
    std::string instruction;
    int64_t type;
    int64_t instNum;
    bool stackArg;
    Instruction* prevInst;
    Instruction* nextInst;
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
    L2::InterferenceGraph* interferenceGraph;
    std::string toSpill;
    std::string replaceSpill;
  };

  struct Program{
    std::string entryPointLabel;
    std::vector<L2::Function *> functions;
  };

  struct DataFlowResult {
      std::string result; 
  };

} // L2
