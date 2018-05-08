#pragma once
#include <vector>

namespace L3 {
  struct Variable;
  struct InterferenceGraph;
  struct Instruction;
  struct Function;
  struct Program;
  struct DataFlowResult;

  enum ArgType {
    NUM,
    VAR, 
    LBL,
    CALLEE
  };


  struct Arg {
    std::string name;
    L3::ArgType type;
  };



  struct Instruction {
    std::string instruction;
    int64_t instNum;
    Instruction* prevInst;
    Instruction* nextInst;

    virtual ~Instruction() {};
  };


  struct Instruction_Assignment : Instruction {
    L3::Arg* src;
    L3::Arg* dst;
  };


  struct Instruction_opAssignment : Instruction {
    L3::Arg*  dst;
    L3::Arg*  arg1;
    L3::Arg*  arg2;
    std::string operation;
  };

  struct Instruction_cmpAssignment : Instruction_opAssignment {

  };


  struct Instruction_Load : Instruction_Assignment {

  };

  struct Instruction_Store : Instruction_Load {

  };

  struct Instruction_br : Instruction {
    L3::Arg* label;
  };


  struct Instruction_brCmp : Instruction {
    L3::Arg* comparitor;
    L3::Arg* trueLabel;
    L3::Arg* falseLabel;
  };

  struct Instruction_Return : Instruction {

  };

  struct Instruction_ReturnVal : Instruction {
    L3::Arg* retVal;
  };

  struct Instruction_Call : Instruction {
    L3::Arg* callee;
    std::vector< L3::Arg *> parameters;
  };

  struct Instruction_CallAssign : Instruction_Call {
    L3::Arg* dst;
  };

  struct Instruction_Label : Instruction {
    L3::Arg* label;
  };

  struct Function{
    std::string name;
    int64_t arguments;
    int64_t locals;
    int64_t uniques;
    std::vector< L3::Arg *> parameters;
    std::vector< L3::Instruction *> instructions;
  };

  struct Program{
    std::vector<L3::Function *> functions;
  };

  struct DataFlowResult {
      std::string result; 
  };

} // L3
