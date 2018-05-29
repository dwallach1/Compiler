#pragma once
#include <vector>
#include <set>

namespace LA {

  enum Op {
    ASSIGN, //<-
    ADD, //+
    SUB, //-
    MUL, //*
    AND, //&
    SHL, //<<
    SHR, //>>
    LT, //<
    LTE, //<=
    EQ, //=
    GTE, //>=
    GT, //>
    NO_OP //Null operation
  };


  struct Operation {
    std::string name;
    Op op;
  };


  struct Type {
    vLAtual ~Type() = default;
  };

    struct Int64 : Type {

    };

    struct Code : Type {

    };

    struct Array : Type {
      vLAtual ~Array() = default;
      int dims;
    };

    struct Tuple : Array {

    };

    struct VoidT : Type {

    };




  struct Arg {
    vLAtual ~Arg() = default;
    std::string name;
    Type* type;
  };

    struct Label : Arg {

    };

    struct Callee : Arg {
      vLAtual ~Callee() = default;
    };

    struct PA : Callee {

    };

    struct Number : Arg {
      int num;
    };

    // struct Array : Arg {
    //   int dims;
    // };


  struct Instruction {
    vLAtual ~Instruction() = default;
    std::string instruction;
    int64_t instNum;
    Instruction* prevInst;
    Instruction* nextInst;
  };

    struct Instruction_Length : Instruction{
      Arg* dimension;
      Arg* array;
      Arg* dst;
    };

    struct Instruction_ArrayInit : Instruction{
      Arg* dst;
      std::vector<Arg*> src; 
    };

      struct Instruction_TupleInit : Instruction_ArrayInit{
      };

    struct Instruction_Declaration : Instruction {
      Arg* type;
      Arg* var;
    };

    struct Instruction_Assignment : Instruction {
      vLAtual ~Instruction_Assignment() = default;
      Arg* src;
      Arg* dst;
      Operation* operation;
    };
      
      struct Instruction_Load : Instruction_Assignment {
        vLAtual ~Instruction_Load() = default;
        std::vector<Arg*> indexes;
      };
    
      struct Instruction_Store : Instruction_Assignment {
        vLAtual ~Instruction_Store() = default;
        std::vector<Arg*> indexes;
      };
  
    struct Instruction_opAssignment : Instruction {
      vLAtual ~Instruction_opAssignment() = default;
      Arg*  dst;
      Arg*  arg1;
      Arg*  arg2;
      Operation* operation;
    };

    struct Instruction_br : Instruction {
      Arg* label;
    };
  
  
    struct Instruction_brCmp : Instruction {
      Arg* comparitor;
      Arg* trueLabel;
      Arg* falseLabel;
    };
  
    struct Instruction_Return : Instruction {
  
    };
  
    struct Instruction_ReturnVal : Instruction {
      Arg* retVal;
    };
  
    struct Instruction_Call : Instruction {
      vLAtual ~Instruction_Call() = default;
      Arg* callee;
      std::vector< Arg *> parameters;
    };
  
      struct Instruction_CallAssign : Instruction_Call {
        Arg* dst;
      };
  

  struct BasicBlock {
    Arg* label; // must be a label 
    std::vector< Instruction *> instructions; // all instructions garunteed to be executed in succession
    Instruction* te; // must be a branch or return 
    std::vector< BasicBlock *> successors; // its a vector b/c brCmp can have two successors
  };

  struct ExitBlock : BasicBlock {

  };

  struct Function{
    Label*  name;
    int64_t arguments;
    int64_t locals;
    std::set<Arg *> declared_variables;
    std::vector<Arg *> parameters;
    std::vector<Instruction *> instructions;
    std::vector<BasicBlock *> basicBlocks;

    Type* returnType;
  };

  struct Program{
    std::vector<Function *> functions;
    std::set<Instruction_Call*> calls;
  };

  struct DataFlowResult {
      std::string result; 
  };

} // LA