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
#define DEBUG_S 0

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;
using namespace pegtl;
using namespace std;

namespace IR {


  /* 
   * Data required to parse
   */ 


  std::vector<IR::Arg*> parsed_registers;
  std::vector<IR::Arg*> index_holder;
  std::vector<IR::Operation*> operations;

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
              pegtl::one<']'>,
            >,
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

  struct declare:
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

  struct i:
    pegtl::seq<
      seps,
      pegtl::sor<
        declare,
        assign,
        op_assign,
        load,
        store,
        length,
        call,
        call_assign,
        array,
        tuple
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

  struct f:
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
          declare,
          seps,
          pegtl::op<
            pegtL::one< ',' >
          >
        >
      >
    >{};  

  struct p:
    pegtl::seq<
      seps,
      pegtl::plus<
        f
      >
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

  template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      
      if(DEBUGGING || DEBUG_S) std::cout << "Found a new function " <<  in.string() << std::endl;
      
      IR::Function *newF = new IR::Function();
      Arg* a = parsed_registers.back();
      parsed_registers.pop_back();

      while(a->type != LBL) {
        
        if(DEBUGGING) std::cout << "adding new parameter to function: " <<  a->name << std::endl;
        
        newF->parameters.push_back(a);
        a = parsed_registers.back();
        parsed_registers.pop_back();
      }
      
      // newF->name = in.string();
      newF->name = a->name;
      if(DEBUGGING) std::cout << "setting this function name to: " <<  a->name << std::endl;

      p.functions.push_back(newF);
    }
  };

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      
      if(DEBUGGING) std::cout << "returning from label: " <<  in.string() << std::endl;
      
      // get rid of var part of label
      parsed_registers.pop_back(); 
      
      IR::Arg* arg = new IR::Arg;
      arg->name = in.string();
      arg->type = LBL;
      parsed_registers.push_back(arg);

      // IR::Function *currentF = p.functions.back();
      // currentF->variables.insert(arg);
    }
  };

  template<> struct action < index > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      if(DEBUGGING) std::cout << "Found an index: " << in.string() << std::endl;
        Arg* newIndex = parsed_registers.back();
        parsed_registers.pop_back();
        index_holder.push_back(newIndex);
    }
  };

  template<> struct action < callee > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      
      if(DEBUGGING) std::cout << "returning from callee " <<  in.string() << std::endl;
      
      
      IR::Arg* arg = new IR::Arg;
      arg->name = in.string();
      if(arg->name == "print" || arg->name == "allocate" || arg->name == "array-error"){
        arg->type = PAA;
        if(DEBUGGING) std::cout << "Found paa value: " << arg->name << "\n";
      }
      else{
        arg->type = CALLEE;
      }
      parsed_registers.push_back(arg);

      // IR::Function *currentF = p.functions.back();
      // currentF->variables.insert(arg);
    }
  };

  template<> struct action < op > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      if(DEBUGGING) std::cout << "Found a op: " << in.string() << std::endl;
      IR::Operation* newOp = new IR::Operation();
      newOp->str = in.string();
      if(newOp->str == "+"){
        newOp->op = ADD;
      }
      else if(newOp->str == "-"){
        newOp->op = ADD;
      }
      else if(newOp->str == "&"){
        newOp->op = AND;
      }
      else if(newOp->str == "*"){
        newOp->op = MUL;
      }
      else if(newOp->str == "<<"){
        newOp->op = SHL;
      }
      else if(newOp->str == ">>"){
        newOp->op = SHR;
      }
      else if(newOp->str == "<"){
        newOp->op = LT;
      }
      else if(newOp->str == "<="){
        newOp->op = LTE;
      }
      else if(newOp->str == "="){
        newOp->op = EQ;
      }
      else if(newOp->str == ">="){
        newOp->op = GTE;
      }
      else if(newOp->str == ">"){
        newOp->op = GT;
      }
      else{
        newOp->op = NO_OP;
      }
      operations.push_back(newOp);

    }
  };




  template<> struct action < assignOp > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      if(DEBUGGING) std::cout << "Found an assignOp: " << in.string() << std::endl;
      // operations.push_back(in.string());

    }
  };

  template<> struct action < var > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      
      if(DEBUGGING) std::cout << "Found a var: " <<  in.string() << std::endl;
      
      IR::Arg* arg = new IR::Arg();
      arg->name = in.string();
      arg->type = VAR; 
      parsed_registers.push_back(arg);

      //IR::Function *currentF = p.functions.back();
      //currentF->variables.insert(arg);

      if(DEBUGGING) std::cout << "Leaving var\n";
    }
  };

  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
      
      if(DEBUGGING) std::cout << "Found a number " << in.string() << std::endl;

      IR::Number* arg = new IR::Number();
      arg->name = in.string();
      arg->type = NUM;
      arg->num = atoi(in.string().c_str());
      parsed_registers.push_back(arg);

      // IR::Function *currentF = p.functions.back();
      // currentF->variables.insert(arg);
    }
  };

  template<> struct action < assign > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found an assign " <<  in.string() << std::endl;
        
        IR::Function *currentF = p.functions.back();
        
        IR::Instruction_Assignment *instruction = new IR::Instruction_Assignment();

        IR::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();

        IR::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();


        size_t loc = in.string().find(source->name);
        if(loc != std::string::npos){
          //Checking to see if the callee is actually a label
          if (in.string()[loc-1] == ':'){
            //Found a label
            source->name.insert(0,1,':');
            source->type = LBL;
         }
        }

        instruction->instruction = dest->name + " <- " + source->name;
  
        instruction->dst = dest;
        instruction->src = source;

        IR::Operation* op = new Operation();
        op->str = "<-";
        op->op = ASSIGN;

        instruction->operation = op;


        instruction->parentFunction = currentF;
        currentF->instructions.push_back(instruction);
        if (DEBUG_S) std::cout << "--> added an Assignment instruction: " <<  instruction->instruction << std::endl;
    }
  };

  template<> struct action < arithmetic_assign > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found an arithmetic_assign " <<  in.string() << std::endl;
        
        IR::Function *currentF = p.functions.back();
        
        IR::Operation* operation = operations.back();
        operations.pop_back();

        IR::Arg* arg2 = parsed_registers.back();
        parsed_registers.pop_back();

        IR::Arg* arg1 = parsed_registers.back();
        parsed_registers.pop_back();

        IR::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        if(operation->op != MUL){
          if(operation->op == ADD){
            IR::Instruction_addAssignment *instruction = new IR::Instruction_addAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->str + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            instruction->parentFunction = currentF;
            currentF->instructions.push_back(instruction);
            if(DEBUG_S) std::cout << "--> added an arithmetic_add instruction: " <<  instruction->instruction << std::endl;
          }
          else if(operation->op == SUB){
            IR::Instruction_subAssignment *instruction = new IR::Instruction_subAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->str + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            instruction->parentFunction = currentF;
            currentF->instructions.push_back(instruction);
            if(DEBUG_S) std::cout << "--> added an arithmetic_sub instruction: " <<  instruction->instruction << std::endl;
          }
          else if(operation->op == AND){
            IR::Instruction_andAssignment *instruction = new IR::Instruction_andAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->str + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            instruction->parentFunction = currentF;
            currentF->instructions.push_back(instruction);
            if(DEBUG_S) std::cout << "--> added an arithmetic_and instruction: " <<  instruction->instruction << std::endl;
          }
          else if(operation->op == SHL){
            IR::Instruction_leftShiftAssignment *instruction = new IR::Instruction_leftShiftAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->str + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            instruction->parentFunction = currentF;
            currentF->instructions.push_back(instruction);
            if(DEBUG_S) std::cout << "--> added an arithmetic_leftShift instruction: " <<  instruction->instruction << std::endl;
          }
          else if(operation->op == SHR){
            IR::Instruction_rightShiftAssignment *instruction = new IR::Instruction_rightShiftAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->str + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            instruction->parentFunction = currentF;
            currentF->instructions.push_back(instruction);
            if(DEBUG_S) std::cout << "--> added an arithmetic_rightShift instruction: " <<  instruction->instruction << std::endl;
          }
          else{
            IR::Instruction_opAssignment *instruction = new IR::Instruction_opAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->str + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            instruction->parentFunction = currentF;
            currentF->instructions.push_back(instruction);
            if(DEBUG_S) std::cout << "--> added an arithmetic_assign instruction: " <<  instruction->instruction << std::endl;
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
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->str + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            instruction->parentFunction = currentF;
            currentF->instructions.push_back(instruction);
            if(DEBUG_S) std::cout << "--> added an arithmetic_specialMult instruction: " <<  instruction->instruction << std::endl;

          }
          //just a normal mult
          else{
            IR::Instruction_multAssignment *instruction = new IR::Instruction_multAssignment();
            instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->str + ' ' + arg2->name;
            instruction->dst = dest;
            instruction->arg1 = arg1;
            instruction->arg2 = arg2;
            instruction->operation = operation;
            instruction->parentFunction = currentF;
            currentF->instructions.push_back(instruction);
            if(DEBUG_S) std::cout << "--> added an arithmetic_mult instruction: " <<  instruction->instruction << std::endl;

          }
        }

        
        return;
    }
  };

  template<> struct action < compare_assign > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a compare_assign " <<  in.string() << std::endl;
        
        IR::Function *currentF = p.functions.back();
        
        IR::Instruction_cmpAssignment *instruction = new IR::Instruction_cmpAssignment();

        IR::Arg* arg2 = parsed_registers.back();
        parsed_registers.pop_back();

        IR::Arg* arg1 = parsed_registers.back();
        parsed_registers.pop_back();

        IR::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        IR::Operation* operation = operations.back();
        operations.pop_back();

        instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation->str + ' ' + arg2->name;
  
        instruction->dst = dest;
        instruction->arg1 = arg1;
        instruction->arg2 = arg2;
        instruction->operation = operation;
        instruction->parentFunction = currentF;
        currentF->instructions.push_back(instruction);
        if (DEBUG_S) std::cout << "--> added an cmpAssignment instruction: " <<  instruction->instruction << std::endl;
    }
  };

  template<> struct action < load > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a load " <<  in.string() << std::endl;
        
        IR::Function *currentF = p.functions.back();
        
        IR::Instruction_Load *instruction = new IR::Instruction_Load();

        IR::Arg* src = parsed_registers.back();
        parsed_registers.pop_back();


        IR::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();


        instruction->instruction = dest->name + " <- load " + src->name;
  
        instruction->dst = dest;
        instruction->src = src;
        instruction->parentFunction = currentF;
        currentF->instructions.push_back(instruction);
        if (DEBUG_S) std::cout << "--> added an Load instruction: " <<  instruction->instruction << std::endl;
    }
  };

  template<> struct action < store > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a store " <<  in.string() << std::endl;
        
        IR::Function *currentF = p.functions.back();
        
        IR::Instruction_Store *instruction = new IR::Instruction_Store();

        IR::Arg* src = parsed_registers.back();
        parsed_registers.pop_back();


        IR::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();


        instruction->instruction = "store " + dest->name + " <- " + src->name;
  
        instruction->dst = dest;
        instruction->src = src;
        instruction->parentFunction = currentF;
        currentF->instructions.push_back(instruction);
        if (DEBUG_S) std::cout << "--> added an Store instruction: " <<  instruction->instruction << std::endl;
    }
  };

  template<> struct action < return_val > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a return_val " <<  in.string() << std::endl;
        
        IR::BasicBlock *currentB = p.functions.back()->basicBlocks.back();
        
        IR::Instruction_ReturnVal *instruction = new IR::Instruction_ReturnVal();

        IR::Arg* val = parsed_registers.back();
        parsed_registers.pop_back();



        instruction->instruction = "return " + val->name;
  
        instruction->retVal = val;
        currentB->te = instruction;
        if (DEBUG_S) std::cout << "--> added an Return_val instruction: " <<  instruction->instruction << std::endl;
      }
  };

  template<> struct action < return_nothing > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a return_nothing " <<  in.string() << std::endl;
        
        IR::BasicBlock *currentB = p.functions.back()->basicBlocks.back();
        
        
        IR::Instruction_Return *instruction = new IR::Instruction_Return();

        instruction->instruction = "return" ;
        currentB->te = instruction;
        if (DEBUG_S) std::cout << "--> added a Return_nothing instruction: " <<  instruction->instruction << std::endl;
    }
  };

  template<> struct action < call > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a call " <<  in.string() << std::endl;
        
        IR::Function *currentF = p.functions.back();
        
        IR::Instruction_Call *instruction = new IR::Instruction_Call();

        IR::Arg* a = parsed_registers.back();
        parsed_registers.pop_back();

        while(a->type != CALLEE && a->type != PAA) {
          if(DEBUGGING) std::cout << "Adding call paramter: " <<  a->name << std::endl;
          instruction->parameters.push_back(a);

          a = parsed_registers.back();
          parsed_registers.pop_back();
        }

        std::reverse(instruction->parameters.begin(), instruction->parameters.end());

        instruction->callee = a;

        instruction->instruction = "call " + instruction->callee->name + " ( ";

        for (auto param : instruction->parameters) {
          instruction->instruction.append(" " + param->name + ",");
        }
        instruction->instruction.append(" )");
  
        instruction->parentFunction = currentF;
        currentF->instructions.push_back(instruction);
        if (DEBUG_S) std::cout << "--> added an Call instruction: " <<  instruction->instruction << std::endl;
    }
  };

  template<> struct action < call_assign > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a call_assign " <<  in.string() << std::endl;
        
        IR::Function *currentF = p.functions.back();
        
        IR::Instruction_CallAssign *instruction = new IR::Instruction_CallAssign();

        IR::Arg* a = parsed_registers.back();
        parsed_registers.pop_back();

        while(a->type != CALLEE && a->type != PAA) {
          if(DEBUGGING) std::cout << "Adding call paramter: " <<  a->name << std::endl;
          
          instruction->parameters.push_back(a);

          a = parsed_registers.back();
          parsed_registers.pop_back();
        }

        std::reverse(instruction->parameters.begin(), instruction->parameters.end());

        instruction->callee = a;

        parsed_registers.pop_back();
        
        IR::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        instruction->dst = dest;

        instruction->instruction = instruction->dst->name + " <- call " + instruction->callee->name + " ( ";

        for (auto param : instruction->parameters) {
          instruction->instruction.append(" " + param->name + ",");
        }
        instruction->instruction.append(" )");
  
        instruction->parentFunction = currentF;
        currentF->instructions.push_back(instruction);
        if (DEBUG_S) std::cout << "--> added an call_assign instruction: " <<  instruction->instruction << std::endl;
    }
  };

  template<> struct action < br_single > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a br_single " <<  in.string() << std::endl;
        
        IR::BasicBlock *currentB = p.functions.back()->basicBlocks.back();
        
        IR::Instruction_br *instruction = new IR::Instruction_br();

        IR::Arg* label = parsed_registers.back();
        parsed_registers.pop_back();

        instruction->instruction = "br " + label->name;
        instruction->label = label;
        currentB->te = instruction;
        if(DEBUG_S) std::cout << "--> added a br_single instruction: " <<  instruction->instruction << std::endl;

    }
  };

  template<> struct action < br_cmp > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found a br_cmp " <<  in.string() << std::endl;
        
        IR::BasicBlock *currentB = p.functions.back()->basicBlocks.back();
        
        
        IR::Instruction_brCmp *instruction = new IR::Instruction_brCmp();
        
        IR::Arg* falseLabel = parsed_registers.back();
        parsed_registers.pop_back();
        
        IR::Arg* trueLabel = parsed_registers.back();
        parsed_registers.pop_back();
        
        IR::Arg* comparitor = parsed_registers.back();
        parsed_registers.pop_back();

        instruction->instruction = "br " + comparitor->name + ' ' + trueLabel->name + ' ' + falseLabel->name;
        instruction->comparitor = comparitor;
        instruction->trueLabel = trueLabel;
        instruction->falseLabel = falseLabel;
        currentB->te = instruction;
        if (DEBUG_S) std::cout << "--> added an br_cmp instruction: " <<  instruction->instruction << std::endl;

    }
  };

  template<> struct action < label_inst > {
    template< typename Input >
    static void apply( const Input & in, IR::Program & p){
        if(DEBUGGING) std::cout << "found the beginning of a new basic block " <<  in.string() << std::endl;
        
        IR::Function *currentF = p.functions.back();
        
        BasicBlock* bb = new BasicBlock();



        IR::Arg* label = parsed_registers.back();
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

    if(DEBUGGING) std::cout << "Done parsing!\n";
    return p;
  }


}// IR
