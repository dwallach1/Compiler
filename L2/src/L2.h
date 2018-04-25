#pragma once

#include <vector>
#include <set>

namespace L2 {

  struct L2_item {
    std::string labelName;
  };
 
  struct Variable {
      std::string name;
      std::set<std::string> edges;
  };
  
  struct InterferenceGraph {
      std::set<L2::Variable*> variables;

       std::string toString() {
         printf("%s", "toString method of IG called");
         std::string str = "";
         for(L2::Variable* V : variables) {
              str.append(V->name);
              printf("%s", V->name.c_str());
              for(std::string E : V->edges){
                  str.append(" ");
                  str.append(E);
              }
              str.append("\n");
         }
        return str;
      };
  };

  struct Instruction {
    std::string instruction;
    int64_t type;

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
    //std::set<std::string> vars; 
    L2::InterferenceGraph* interferenceGraph;
  };

  struct Program{
    std::string entryPointLabel;
    std::vector<L2::Function *> functions;
  };

  struct DataFlowResult {
      std::string result; 
  };

} // L2
