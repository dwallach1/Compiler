#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>
#include <IR.h>
#include <parser.h>
#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#define DEBUGGING 0
#define DEBUG_S 1

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;
using namespace pegtl;
using namespace std;

namespace IR {

  Arg* findVariable(Function* f, Arg* arg);
  /* 
   * Data required to parse
   */ 


  std::vector<Arg *> parsed_registers;
  std::vector<Type *> type_declarations;
  std::vector<Arg *> index_holder;
  std::vector<Operation *> operations;

  /* 
   * Grammar rules from now on.
   */

  struct comment: 
    pegtl::disable< 
      TAOCPP_PEGTL_STRING( "//" ), 
      pegtl::until< pegtl::eolf > 
    > {};

  struct seps: 
    pegtl::star< 
      pegtl::sor< 
        pegtl::ascii::space, 
        comment 
      > 
    > {};

  struct pa:
    pegtl::sor<
        pegtl::string<'p', 'r', 'i', 'n', 't'>,
        pegtl::string<'a', 'r', 'r', 'a', 'y', '-', 'e', 'r', 'r', 'o', 'r'>
      >{};

  struct keyword:
    pegtl::sor<
        pegtl::string< 'c', 'a', 'l', 'l'>,
        pa,
        pegtl::string<  'r', 'e', 't', 'u', 'r', 'n' >
      >{};

  struct word:
    pegtl::seq<
      seps,
      pegtl::not_at< keyword >,
      seps,
      pegtl::plus< 
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >
        >
      >,
      pegtl::star<
        pegtl::sor<
          pegtl::alpha,
          pegtl::one< '_' >,
          pegtl::digit
        >
      >
    >{};

  struct var:
    pegtl::seq<
      seps,
      pegtl::one< '%' >,
      word
    >{};

  struct label:
    pegtl::seq<
      seps,
      pegtl::one<':'>,
      word
    > {};

  struct number:
    pegtl::seq<
      seps,
      pegtl::opt<
        pegtl::sor<
          pegtl::one< '-' >,
          pegtl::one< '+' >
        >
      >,
      pegtl::plus< 
        pegtl::digit
      >
    >{};

  struct op:
    pegtl::seq<
      seps,
      pegtl::sor<
        pegtl::one< '+' >,
        pegtl::one< '-' >,
        pegtl::one< '*' >,
        pegtl::one< '&' >,
        pegtl::string< '<', '<' >,
        pegtl::string< '>', '>' >,
        pegtl::one< '<' >,
        pegtl::string< '<', '=' >,
        pegtl::one< '=' >,
        pegtl::string< '>', '=' >,
        pegtl::one< '>' >
      >
    >{};

  struct u:
    pegtl::sor<
      var,
      label
    >{};

  struct t:
    pegtl::sor<
      var,
      number
    > {};

  struct s: 
    pegtl::sor<
      t,
      label
    >{};

  struct args:
    pegtl::star<
      pegtl::seq<
        seps,
        t,
        seps,
        pegtl::opt<
          pegtl::one<','>
        >
      >
    >{};

  struct vars:
    pegtl::star<
      pegtl::seq<
        seps,
        var,
        seps,
        pegtl::opt<
          pegtl::one<','>
        >
      >
    >{};

  struct callee:
    pegtl::sor<
        u,
        pa
      >{};

  struct type:
    pegtl::seq<
      seps,
      pegtl::sor<
        pegtl::seq<
          pegtl::string< 'i', 'n', 't', '6', '4' >,
          pegtl::star<
            pegtl::seq<
              seps,
              pegtl::one<'['>,
              seps,
              pegtl::one<']'>
            >
          >
        >,
        pegtl::string< 't', 'u', 'p', 'l', 'e' >,
        pegtl::string< 'c', 'o', 'd', 'e' >
      >
    >{};

  struct T:
    pegtl::seq<
      seps,
      pegtl::sor<
        type,
        pegtl::string< 'v', 'o', 'i', 'd'>
      >
    >{};

  struct declaration:
    pegtl::seq<
      seps,
      type,
      seps,
      var
    >{};

  struct assignOp:
    pegtl::seq<
      seps,
      pegtl::string< '<', '-' >
    >{};

  struct assignment:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      not_at<
      keyword
      >,
      s
    >{};

  struct op_assign:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      t,
      seps,
      op,
      seps,
      t
    >{};

  struct index:
    pegtl::seq<
      seps,
      pegtl::one< '[' >,
      t,
      pegtl::one< ']' > 
    >{};

  struct indexes:
    pegtl::plus<  
      index
    >{};

  struct load: 
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      var,
      indexes
    >{};

  struct store:
    pegtl::seq<
      seps,
      var,
      indexes,
      seps,
      assignOp,
      seps,
      s
    >{};

  struct length:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      pegtl::string< 'l', 'e', 'n', 'g', 't', 'h' >,
      seps,
      var,
      seps, 
      t
    >{};

  struct call:
    pegtl::seq<
      seps,
      pegtl::string< 'c', 'a', 'l', 'l'>,
      seps,
      callee,
      seps,
      pegtl::one< '(' >,
      seps,
      args,
      seps,
      pegtl::one< ')' >
    >{};

  struct call_assign:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      call
    >{};

  struct array:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      pegtl::string< 'n', 'e', 'w', ' ' >,
      seps,
      pegtl::string< 'A', 'r', 'r', 'a', 'y' >,
      seps,
      pegtl::one< '(' >,
      pegtl::plus<
        pegtl::seq<
          seps,
          t,
          seps,
          pegtl::opt<
            pegtl::one<','>
          >
        >
      >,
      pegtl::one < ')' >
    >{};

  struct tuple:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      pegtl::string< 'n', 'e', 'w', ' ' >,
      seps,
      pegtl::string< 'T', 'u', 'p', 'l', 'e' >,
      seps,
      pegtl::one< '(' >,
      seps,
      t,
      seps,
      pegtl::one< ')' >
    >{};

  struct label_inst:
    pegtl::seq<
      seps,
      label
    >{};

  struct i:
    pegtl::seq<
      seps,
      pegtl::sor<
        declaration,
        assignment,
        op_assign,
        load,
        store,
        length,
        call,
        call_assign,
        array,
        tuple,
        label_inst
      >
    >{};

  struct i_star:
    pegtl::star<
      pegtl::seq<
        seps,
        i
      >
    >{};

  struct br_single:
    pegtl::seq<
      seps,
      pegtl::string< 'b', 'r' >,
      seps,
      label
    >{};

  struct br_cmp:
    pegtl::seq<
      pegtl::string< 'b', 'r' >,
      seps,
      var,
      seps,
      label,
      seps,
      label
    >{};

  struct return_nothing:
    pegtl::seq<
      seps,
      pegtl::string<'r', 'e', 't', 'u', 'r', 'n'>,
      seps,
      pegtl::not_at< t >
    >{};

  struct return_val:
    pegtl::seq<
      seps,
      pegtl::string<'r', 'e', 't', 'u', 'r', 'n'>,
      seps,
      t
    >{};

  struct te:
    pegtl::seq<
      seps,
      pegtl::sor<
        br_single,
        br_cmp,
        return_nothing,
        return_val
      >
    >{};

  struct bb:
    pegtl::seq<
      seps,
      label,
      seps,
      i_star,
      seps,
      te
    >{};

  struct f_name: 
    pegtl::seq<
      seps,
      pegtl::string< 'd', 'e', 'f', 'i', 'n', 'e' >,
      seps,
      T,
      seps,
      label,
      seps, 
      pegtl::one< '(' >,
      seps,
      pegtl::seq<
        pegtl::star<
          declaration,
          seps,
          pegtl::opt<
            pegtl::one< ',' >
          > // closes op
        > // closes star
      >, // closes seq
      seps,
      pegtl::one< ')' >
    >{};

  struct f:
    pegtl::seq<
      seps,
      f_name,
      seps,
      pegtl::one< '{' >,
      seps,
      pegtl::plus< bb >,
      seps,
      pegtl::one< '}' >
    >{};  

  struct p:
    pegtl::seq<
      seps,
      pegtl::plus< f >
    >{};
  
  struct entry_point_rule:
    pegtl::seq<
      seps,
      p
    >{};

  struct grammar : 
    pegtl::must< 
      entry_point_rule
    > {};

  /*
  *
  *   TODO: Update all actions , and fix bugs in struct declarations
  *
  */

  /* 
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < f_name > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      
      if(DEBUGGING) std::cout << "Found a new function " <<  in.string() << std::endl;
      
      Function *newF = new Function();
      
      Arg* parameter = parsed_registers.back();
      parsed_registers.pop_back();

      Label* funcName = dynamic_cast<Label *> (parameter);

      while(funcName == NULL) {
        
        newF->parameters.push_back(parameter);
        parameter = parsed_registers.back();
        parsed_registers.pop_back();

        funcName = dynamic_cast<Label *> (parameter);
      }

      newF->name = funcName;

      Type* returnType = type_declarations.back();
      type_declarations.pop_back();

      newF->returnType = returnType;

      p.functions.push_back(newF);
    }
  };

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      
      if(DEBUGGING) std::cout << "returning from label: " <<  in.string() << std::endl;
      
      // get rid of var part of label
      parsed_registers.pop_back(); 
      
      Label* label = new Label();
      label->name = in.string();
      parsed_registers.push_back(label);

      VoidT* noType = new VoidT();
      label->type = noType;

    }
  };

  template<> struct action < index > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      if(DEBUGGING) std::cout << "Found an index: " << in.string() << std::endl;

        // it will be a number -- get the number part [ num ]
        Arg* newIndex = parsed_registers.back();
        parsed_registers.pop_back();
        index_holder.push_back(newIndex);
    }
  };

  template<> struct action < callee > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      
      if(DEBUGGING) std::cout << "returning from callee " <<  in.string() << std::endl;
      
      if (in.string() == "print" || in.string() == "array-error") {
          PA* pa = new PA();
          pa->name = in.string();
          VoidT* noType = new VoidT();
          pa->type = noType;
          parsed_registers.push_back(pa);

      }
      else {
        Callee* callee = new Callee();
        callee->name = in.string();
        VoidT* noType = new VoidT();
        callee->type = noType;
        parsed_registers.push_back(callee);
      }
    }
  };

  template<> struct action < op > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      if(DEBUGGING) std::cout << "Found a op: " << in.string() << std::endl;
      
      Operation* newOp = new Operation();
      newOp->name = in.string();
      
      if(newOp->name == "+"){
        newOp->op = ADD;
      }
      else if(newOp->name == "-"){
        newOp->op = ADD;
      }
      else if(newOp->name == "&"){
        newOp->op = AND;
      }
      else if(newOp->name == "*"){
        newOp->op = MUL;
      }
      else if(newOp->name == "<<"){
        newOp->op = SHL;
      }
      else if(newOp->name == ">>"){
        newOp->op = SHR;
      }
      else if(newOp->name == "<"){
        newOp->op = LT;
      }
      else if(newOp->name == "<="){
        newOp->op = LTE;
      }
      else if(newOp->name == "="){
        newOp->op = EQ;
      }
      else if(newOp->name == ">="){
        newOp->op = GTE;
      }
      else if(newOp->name == ">"){
        newOp->op = GT;
      }
      else{
        newOp->op = NO_OP;
      }
      operations.push_back(newOp);

    }
  };

  template<> struct action < var > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      
      if(DEBUGGING) std::cout << "Found a var: " <<  in.string() << std::endl;
      
      Arg* arg = new Arg();
      arg->name = in.string();
      parsed_registers.push_back(arg);

    }
  };

  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      
      if(DEBUGGING) std::cout << "Found a number " << in.string() << std::endl;

      Number* num = new Number();
      num->name = in.string();
      num->num = atoi(in.string().c_str());
      parsed_registers.push_back(num);

    }
  };

  template<> struct action < T > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      if(DEBUGGING) std::cout << "found a Type: " <<  in.string() << std::endl;

      if(in.string() == "void") {
        VoidT* voidT = new VoidT();
        type_declarations.push_back(voidT);
      }
      else if(in.string() == "int64") {
        Int64* int64 = new Int64();
        type_declarations.push_back(int64);
      }
      else if (in.string() == "code") {
        Code* code = new Code();
        type_declarations.push_back(code);
      } 
      else if (in.string() == "tuple") {
        Tuple* tuple = new Tuple();
        type_declarations.push_back(tuple);

      }
      else {
        Array* array = new Array();

        int dims = index_holder.size();
        array->dims = dims;

        type_declarations.push_back(array);
      }
    }
  };

  template<> struct action < declaration > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      if(DEBUGGING) std::cout << "found a declaration " <<  in.string() << std::endl;

      Function* currentF = p.functions.back();
      BasicBlock* currentB = currentF->basicBlocks.back();

      Type* type = type_declarations.back();
      type_declarations.pop_back();

      Arg* arg = parsed_registers.back();
      parsed_registers.pop_back();

      arg->type = type;

      currentF->declared_variables.insert(arg);

    }
  };

  template<> struct action < assignment > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found an assign " <<  in.string() << std::endl;
        
        Function* currentF = p.functions.back();
        BasicBlock* currentB = currentF->basicBlocks.back();
        
        Instruction_Assignment *instruction = new IR::Instruction_Assignment();

        Arg* source = parsed_registers.back();
        parsed_registers.pop_back();

        Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        // map the variables to the declared variables 
        source = findVariable(currentF, source);
        dest = findVariable(currentF, dest);

        instruction->instruction = dest->name + " <- " + source->name;
  
        instruction->dst = dest;
        instruction->src = source;

        Operation* op = new Operation();
        op->name = "<-";
        op->op = ASSIGN;

        instruction->operation = op;


        currentB->instructions.push_back(instruction);
    }
  };

  template<> struct action < op_assign > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found an op_assign " <<  in.string() << std::endl;
        
        Function*   currentF = p.functions.back();
        BasicBlock* currentB = currentF->basicBlocks.back();
        
        Operation* operation = operations.back();
        operations.pop_back();

        IR::Arg* arg2 = parsed_registers.back();
        parsed_registers.pop_back();

        IR::Arg* arg1 = parsed_registers.back();
        parsed_registers.pop_back();

        IR::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        dest = findVariable(currentF, dest);
        arg1 = findVariable(currentF, arg1);
        arg2 = findVariable(currentF, arg2);

        if(operation->op == LT  || 
           operation->op == LTE ||
           operation->op == EQ  ||
           operation->op == GTE ||
           operation->op == GT   ) {

          Instruction_cmpAssignment* instruction = new Instruction_cmpAssignment();

          instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->name + ' ' + arg2->name;
          instruction->dst = dest;
          instruction->arg1 = arg1;
          instruction->arg2 = arg2;
          instruction->operation = operation;
          
          currentB->instructions.push_back(instruction);

        } 
        else if(operation->op != MUL){
          if(operation->op == ADD){
            Instruction_addAssignment *instruction = new Instruction_addAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->name + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            
            currentB->instructions.push_back(instruction);
          }
          else if(operation->op == SUB){
            IR::Instruction_subAssignment *instruction = new IR::Instruction_subAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->name + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            
            currentB->instructions.push_back(instruction);
          }
          else if(operation->op == AND){
            IR::Instruction_andAssignment *instruction = new IR::Instruction_andAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->name + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            
            currentB->instructions.push_back(instruction);
          }
          else if(operation->op == SHL){
            IR::Instruction_leftShiftAssignment *instruction = new IR::Instruction_leftShiftAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->name + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            
            currentB->instructions.push_back(instruction);
          }
          else if(operation->op == SHR){
            IR::Instruction_rightShiftAssignment *instruction = new IR::Instruction_rightShiftAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->name + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            
            currentB->te = instruction;
          }
          else{
            IR::Instruction_opAssignment *instruction = new IR::Instruction_opAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->name + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            
            currentB->instructions.push_back(instruction);
          }  
        }
        //Checking to see if there is a multiply by 2 4 8 or 16
        else if(operation->op == MUL){
          unsigned loc = 0;
          if(IR::Number* number = dynamic_cast<IR::Number*>(arg1)){
            if(number->num == 2 || number->num == 4 || number->num == 8 || number->num == 16){
              loc = 1;
            }
          }
          else if(IR::Number* number = dynamic_cast<IR::Number*>(arg2)){
            if(number->num == 2 || number->num == 4 || number->num == 8 || number->num == 16){
              loc = 2;
            }
          }
          if (loc){
            IR::Instruction_specialMultAssignment *instruction = new IR::Instruction_specialMultAssignment();
            instruction->locOfNum = loc;
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->name + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            
            currentB->instructions.push_back(instruction);

          }
          //just a normal mult
          else{
            IR::Instruction_multAssignment *instruction = new IR::Instruction_multAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->name + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            
            currentB->instructions.push_back(instruction);
          }
        }
    }
  };

  template<> struct action < load > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a load " <<  in.string() << std::endl;
        
        Function* currentF = p.functions.back();
        BasicBlock *currentB = currentF->basicBlocks.back();
        
        Instruction_Load* instruction = new Instruction_Load();

        Arg* src = parsed_registers.back();
        parsed_registers.pop_back();

        Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        src = findVariable(currentF, src);
        dest = findVariable(currentF, dest);

        instruction->instruction = dest->name + " <- load " + src->name;
  
        instruction->dst = dest;
        instruction->src = src;

        currentB->instructions.push_back(instruction);
    }
  };

  template<> struct action < store > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a store " <<  in.string() << std::endl;
        
        Function*   currentF = p.functions.back();
        BasicBlock* currentB = currentF->basicBlocks.back();
        
        Instruction_Store *instruction = new Instruction_Store();

        Arg* src = parsed_registers.back();
        parsed_registers.pop_back();

        Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        src = findVariable(currentF, src);
        dest = findVariable(currentF, dest);

        instruction->instruction = "store " + dest->name + " <- " + src->name;
  
        instruction->dst = dest;
        instruction->src = src;
        
        currentB->instructions.push_back(instruction);
    }
  };

  template<> struct action < call > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a call " <<  in.string() << std::endl;
        
        Function*   currentF = p.functions.back();
        BasicBlock* currentB = currentF->basicBlocks.back();
        
        Instruction_Call* instruction = new Instruction_Call();

        Arg* parameter = parsed_registers.back();
        parsed_registers.pop_back();

        Callee* callee = dynamic_cast<Callee *>(parameter);

        while(callee == NULL) {

          parameter = findVariable(currentF, parameter);
          instruction->parameters.push_back(parameter);
          
          parameter = parsed_registers.back();
          parsed_registers.pop_back();

          callee = dynamic_cast<Callee *>(parameter);
        }

        std::reverse(instruction->parameters.begin(), instruction->parameters.end());

        instruction->callee = callee;

        instruction->instruction = "call " + instruction->callee->name + " ( ";

        for (auto param : instruction->parameters) {
          instruction->instruction.append(" " + param->name + ",");
        }
        instruction->instruction.append(" )");
  
        currentB->instructions.push_back(instruction);
    }
  };

  template<> struct action < call_assign > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a call_assign " <<  in.string() << std::endl;
        
        Function*   currentF = p.functions.back();
        BasicBlock* currentB = currentF->basicBlocks.back();
        
        Instruction_CallAssign* instruction = new Instruction_CallAssign();

        Arg* parameter = parsed_registers.back();
        parsed_registers.pop_back();

        Callee* callee = dynamic_cast<Callee *> (parameter);

        while(callee == NULL) {
          
          parameter = findVariable(currentF, parameter);
          instruction->parameters.push_back(parameter);

          parameter = parsed_registers.back();
          parsed_registers.pop_back();

          callee =  dynamic_cast<Callee *> (parameter);
        }

        std::reverse(instruction->parameters.begin(), instruction->parameters.end());

        instruction->callee = callee;
        parsed_registers.pop_back();
        
        Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        dest = findVariable(currentF, dest);
        instruction->dst = dest;

        instruction->instruction = instruction->dst->name + " <- call " + instruction->callee->name + " ( ";

        for (auto param : instruction->parameters) {
          instruction->instruction.append(" " + param->name + ",");
        }
        instruction->instruction.append(" )");
  
        currentB->instructions.push_back(instruction);
    }
  };

  template<> struct action < return_val > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a return_val " <<  in.string() << std::endl;
        
        Function*   currentF = p.functions.back();
        BasicBlock* currentB = currentF->basicBlocks.back();
        
        Instruction_ReturnVal* instruction = new Instruction_ReturnVal();

        Arg* val = parsed_registers.back();
        parsed_registers.pop_back();
        val = findVariable(currentF, val);

        instruction->instruction = "return " + val->name;
  
        instruction->retVal = val;
        currentB->te = instruction;
      }
  };

  template<> struct action < return_nothing > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a return_nothing " <<  in.string() << std::endl;
        
        BasicBlock* currentB = p.functions.back()->basicBlocks.back();
        
        Instruction_Return* instruction = new Instruction_Return();
        instruction->instruction = "return" ;
        currentB->te = instruction;
    }
  };

  template<> struct action < br_single > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a br_single " <<  in.string() << std::endl;
        
        BasicBlock* currentB = p.functions.back()->basicBlocks.back();
        
        Instruction_br* instruction = new Instruction_br();

        Label* label = dynamic_cast<Label *> (parsed_registers.back());
        parsed_registers.pop_back();

        instruction->instruction = "br " + label->name;
        instruction->label = label;
        currentB->te = instruction;

    }
  };

  template<> struct action < br_cmp > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a br_cmp " <<  in.string() << std::endl;
        
        Function*   currentF = p.functions.back();
        BasicBlock* currentB = currentF->basicBlocks.back();
        
        Instruction_brCmp* instruction = new Instruction_brCmp();
        
        Label* falseLabel = dynamic_cast<Label *> (parsed_registers.back());
        parsed_registers.pop_back();
        
        Label* trueLabel = dynamic_cast<Label *> (parsed_registers.back());
        parsed_registers.pop_back();
        
        Arg* comparitor = parsed_registers.back();
        parsed_registers.pop_back();

        comparitor = findVariable(currentF, comparitor);

        instruction->instruction = "br " + comparitor->name + ' ' + trueLabel->name + ' ' + falseLabel->name;
        instruction->comparitor = comparitor;
        instruction->trueLabel = trueLabel;
        instruction->falseLabel = falseLabel;
        currentB->te = instruction;

    }
  };

  template<> struct action < label_inst > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found the beginning of a new basic block " <<  in.string() << std::endl;
        
        Function* currentF = p.functions.back();
        BasicBlock* bb = new BasicBlock();

        Label* label = dynamic_cast<Label *> (parsed_registers.back());
        parsed_registers.pop_back();
        bb->label = label;
        currentF->basicBlocks.push_back(bb);

    }
  };

  template<> struct action < comment > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "Found a comment " <<  in.string() << std::endl;
    }
  };


  
  /*
   *
   *  Functions to parse input file 
   *
   */  

  Program parse_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< IR::grammar >();


    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    IR::Program p;
    
    parse< IR::grammar, IR::action >(fileInput, p);

    if(DEBUGGING | DEBUG_S) std::cout << "Done parsing!\n";
    return p;
  }

  Arg* findVariable(Function* f, Arg* arg) {
    
    // if it is just a number, then it wont be stored in declared vars
    if (Number* num = dynamic_cast<Number *> (arg)) { return arg; }

    // reverse so we get the latest declaration -- can you redefine variables with new types?
    // std::reverse(f->declared_variables.begin(), f->declared_variables.end());
    for(Arg* var : f->declared_variables) {
      if (var->name == arg->name ) { return var; }
    }
    return NULL;
  }


}// IR
