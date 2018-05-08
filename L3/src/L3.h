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

  struct Operation{
    std::string str;
    Op op;
  };


  struct Arg {
    virtual ~Arg() = default;
    std::string name;
    L3::ArgType type;
  };

    struct Number : Arg {
      int num;
    };


  struct Instruction {
    virtual ~Instruction() = default;
    std::string instruction;
    int64_t instNum;
    Instruction* prevInst;
    Instruction* nextInst;
  };


    struct Instruction_Assignment : Instruction {
      L3::Arg* src;
      L3::Arg* dst;
    };
  
  
    struct Instruction_opAssignment : Instruction {
      virtual ~Instruction_opAssignment() = default;
      L3::Arg*  dst;
      L3::Arg*  arg1;
      L3::Arg*  arg2;
      L3::Operation* operation;
    };
  
      struct Instruction_cmpAssignment : Instruction_opAssignment {
    
      };
    
      struct Instruction_addAssignment : Instruction_opAssignment {
    
      };
    
      struct Instruction_subAssignment : Instruction_opAssignment {
    
      };

      struct Instruction_andAssignment : Instruction_opAssignment {
    
      };
    
      struct Instruction_shiftAssignment : Instruction_opAssignment {
        virtual ~Instruction_shiftAssignment() = default;
      };
  
        struct Instruction_leftShiftAssignment : Instruction_shiftAssignment {
  
        };
        struct Instruction_rightShiftAssignment : Instruction_shiftAssignment {
  
        };
    
      struct Instruction_multAssignment : Instruction_opAssignment {
        virtual ~Instruction_multAssignment() = default;
      };
        //This is for a multiply by 2 4 8 or 16
        struct Instruction_specialMultAssignment : Instruction_multAssignment {
          unsigned locOfNum;
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
