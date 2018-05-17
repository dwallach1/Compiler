#pragma once
#include <vector>
#include <set>

namespace IR {

  // enum ArgType {
  //   NUM,
  //   VAR, 
  //   LBL,
  //   CALLEE,
  //   PA,
  //   S_ARG,
  //   RSPMEM,
  //   ARRAY
  // };

  // enum Type {
  //   INT64,
  //   ARRAY,
  //   CODE,
  //   LBL
  // };



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
    virtual ~Type() = default;
  };

    struct Int64 : Type {

    };

    struct Code : Type {

    };

    struct Array : Type {
      virtual ~Array() = default;
      int dims;
    };

    struct Tuple : Array {

    };

    struct VoidT : Type {

    };




  struct Arg {
    virtual ~Arg() = default;
    std::string name;
    Type* type;
  };

    struct Label : Arg {

    };

    struct Callee : Arg {
      virtual ~Callee() = default;
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
    virtual ~Instruction() = default;
    std::string instruction;
    int64_t instNum;
    Instruction* prevInst;
    Instruction* nextInst;
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

    struct Instruction_Lea : Instruction {

    };

    struct Instruction_MemWithNonZeroConst : Instruction {

    };

    struct Instruction_Assignment : Instruction {
      virtual ~Instruction_Assignment() = default;
      Arg* src;
      Arg* dst;
      Operation* operation;
    };
      
      struct Instruction_Length : Instruction_Assignment{

      };


      struct Instruction_Load : Instruction_Assignment {
        virtual ~Instruction_Load() = default;
        std::vector<Arg*> indexes;
      };
        struct Instruction_stackArg : Instruction_Load {
        };
    
      struct Instruction_Store : Instruction_Assignment {
        virtual ~Instruction_Store() = default;
        std::vector<Arg*> indexes;
      };
        struct Instruction_stackStore : Instruction_Store {

        };
  
    struct Instruction_opAssignment : Instruction {
      virtual ~Instruction_opAssignment() = default;
      Arg*  dst;
      Arg*  arg1;
      Arg*  arg2;
      Operation* operation;
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
      virtual ~Instruction_Call() = default;
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

} // IR
