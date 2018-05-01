#pragma once

#include <vector>
#include <set>

namespace L2 {
  struct Variable;
  struct InterferenceGraph;
  struct Instruction;
  struct Function;
  struct Program;
  struct DataFlowResult;

  enum Type {
    AOP,
    ASSIGN,
    LOAD,
    STORE,
    CMP_ASSIGN,
    LABEL,
    INC_DEC,
    CJUMP,
    GOTO,
    RET,
    CALL,
    LEA
  };

  enum Color {
    RDI,
    RSI,
    RDX,
    RCX,
    R8,
    R9,
    RAX,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
    RBP,
    RBX,
    NO_COLOR
  };

  enum ArgType {
    NUM,
    MEM,
    VAR, 
    LBL
  };


  struct Arg {
    std::string name;
    L2::ArgType type;
  };


  struct Variable {
      int type;
      std::string name;
      std::set<std::string> edges;
      std::vector<L2::Instruction*> uses;
      L2::Color color;
      bool aliveColors[16];
  };
  
  struct InterferenceGraph {
      std::set<L2::Variable*> variables;

  };

  struct Instruction {
    std::string instruction;
    L2::Type type;
    int64_t instNum;
    // bool stackArg;
    Instruction* prevInst;
    Instruction* nextInst;
    std::vector<L2::Arg *> arguments;
    std::vector<std::string> operation;

    std::vector<std::string> gen;
    std::vector<std::string> kill;

    std::vector<std::string> in;
    std::vector<std::string> out;
  };


  // struct Instruction_AOP : Instruction {
  //   L2::Arg src;
  //   L2::Arg dst;
  //   std::string operation;
  // };


  // struct Instruction_Assign : Instruction_AOP {

  // };

  // struct Instruction_Load : Instruction {

  // }



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
