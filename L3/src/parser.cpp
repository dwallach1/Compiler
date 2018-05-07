#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>
#include <L3.h>
#include <parser.h>
#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

#define DEBUGGING 1


namespace pegtl = tao::TAO_PEGTL_NAMESPACE;
using namespace pegtl;
using namespace std;

namespace L3 {

  /* 
   * Data required to parse
   */ 

  std::vector<L3::Arg*> parsed_registers;
  std::vector<std::string> operations;

  std::vector<std::string> assignmentVec;
  std::vector<std::string> compareVec;
  std::vector<std::string> labelInsts;

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

  struct var:
    pegtl::seq<
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

  struct label:
    pegtl::seq<
      seps,
      pegtl::one<':'>,
      var
    > {};

  struct number:
    pegtl::seq<
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

  struct t:
    pegtl::sor<
      var,
      number
    > {};

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

  struct s: 
    pegtl::sor<
      t,
      label
    >{};

  struct u:
    pegtl::sor<
      var,
      label
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
        pegtl::string< '>', '>' >
      >
    >{};

  struct cmp:
    pegtl::seq<
      pegtl::sor<
        pegtl::string< '<', '=' >,
        pegtl::seq<
          pegtl::one< '<' >,
          pegtl::not_at< pegtl::one< '<' > >
        >,
        pegtl::one< '=' >,
        pegtl::seq<
          pegtl::one< '>' >,
          pegtl::not_at< pegtl::one< '>' > >
        >,
        pegtl::string< '>', '=' >
      >
    >{};

  struct assignOp:
    pegtl::seq<
      seps,
      pegtl::string< '<', '-' >
    >
  {};

  struct assign:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      s,
      seps,
      pegtl::not_at< op >
    >{};

  struct compare_assign:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      t,
      seps,
      cmp,
      seps,
      t
    >{};


   struct assignment:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      s
    >{};

  struct arithmetic_assign:
    pegtl::seq<
      seps,
      var,
      seps,
      assign,
      t,
      seps,
      op,
      seps,
      t
    >{};

  struct load:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      pegtl::string<'l', 'o', 'a', 'd'>,
      seps,
      var
    >{};

  struct store:
    pegtl::seq<
      seps,
      pegtl::string<'s', 't', 'o', 'r', 'e'>,
      seps,
      var,
      seps,
      assignOp,
      seps,
      s
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
      return_nothing,
      seps,
      t
    >{};

  struct callee:
    pegtl::sor<
        u,
        pegtl::string<'p', 'r', 'i', 'n', 't'>,
        pegtl::string<'a', 'l', 'l', 'o', 'c', 'a', 't', 'e'>,
        pegtl::string<'a', 'r', 'r', 'a', 'y', '-', 'e', 'r', 'r', 'o', 'r'>
      >
  {};

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
      pegtl::string< 'c', 'a', 'l', 'l'>,
      seps,
      callee,
      seps,
      pegtl::one< '(' >,
      seps,
      args,
      seps,
      pegtl::one < ')' >
    >{};

  struct label_inst:
    pegtl::seq<
      seps,
      label
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

  struct instruction:
    pegtl::sor<
      assign,
      arithmetic_assign,
      compare_assign,
      load,
      store,
      br_single,
      label_inst,
      br_cmp,
      return_nothing,
      return_val,
      call,
      call_assign
    >{};

  struct instructions_rule:
    pegtl::seq< 
      seps,
      pegtl::plus< instruction >
    >{};

  struct function_name:
    pegtl::seq<
      seps,
      pegtl::string< 'd', 'e', 'f', 'i', 'n', 'e' >,
      seps,
      label,
      seps,
      pegtl::one< '(' >,
      seps,
      vars,
      seps,
      pegtl::one< ')' >
    >{};
 
  struct L3_function_rule:
    pegtl::seq<
      seps,
      pegtl::string<'d', 'e', 'f', 'i', 'n', 'e'>,
      seps,
      function_name,
      seps,
      pegtl::one< '{' >,
      seps,
      instructions_rule,
      seps,
      pegtl::one< '}' >
    > {};

  struct L3_functions_rule:
    pegtl::seq<
      seps,
      pegtl::plus< L3_function_rule >
    > {};

  struct L3_main_function_rule:
    pegtl::seq<
      seps,
      pegtl::string<'d', 'e', 'f', 'i', 'n', 'e'>,
      seps,
      pegtl::string<':', 'm', 'a', 'i', 'n'>,
      seps,
      pegtl::one< '(' >,
      seps,
      pegtl::one< ')' >,
      seps,
      pegtl::one< '{' >,
      seps,
      instructions_rule,
      seps,
      pegtl::one< '}' >
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      L3_main_function_rule,
      seps
    > { };

  struct grammar : 
    pegtl::must< 
      entry_point_rule
    > {};

  

  /* 
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      
      if(DEBUGGING) std::cout << "Found a new function " <<  in.string() << std::endl;
      
      L3::Function *newF = new L3::Function();
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
    static void apply( const Input & in, L3::Program & p){
      
      if(DEBUGGING) std::cout << "returning from label " <<  in.string() << std::endl;
      
      // get rid of var part of label
      parsed_registers.pop_back(); 
      
      L3::Arg* arg = new L3::Arg;
      arg->name = in.string();
      arg->type = LBL;
      parsed_registers.push_back(arg);
      labelInsts.push_back(in.string());
    }
  };

  template<> struct action < callee > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      
      if(DEBUGGING) std::cout << "returning from callee " <<  in.string() << std::endl;
      
      
      L3::Arg* arg = new L3::Arg;
      arg->name = in.string();
      arg->type = CALLEE;
      parsed_registers.push_back(arg);
      labelInsts.push_back(in.string());
    }
  };

  template<> struct action < cmp > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "Found a cmp " << in.string() << std::endl;
      operations.push_back(in.string());

    }
  };

  template<> struct action < op > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "Found a op " << in.string() << std::endl;
      operations.push_back(in.string());

    }
  };

  template<> struct action < var > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      
      if(DEBUGGING) std::cout << "Found a var " <<  in.string() << std::endl;
      
      L3::Arg* arg = new L3::Arg();
      arg->name = in.string();
      arg->type = VAR; 
      parsed_registers.push_back(arg);
    }
  };



  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      
      if(DEBUGGING) std::cout << "Found a number " << in.string() << std::endl;

      L3::Arg* arg = new L3::Arg();
      arg->name = in.string();
      arg->type = NUM;
      parsed_registers.push_back(arg);

    }
  };

  template<> struct action < assign > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found an assign " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_Assignment *instruction = new L3::Instruction_Assignment();

        L3::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();

        L3::Arg* dest = parsed_registers.back();
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
        
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < arithmetic_assign > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found an arithmetic_assign " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_opAssignment *instruction = new L3::Instruction_opAssignment();

        L3::Arg* arg2 = parsed_registers.back();
        parsed_registers.pop_back();

        L3::Arg* arg1 = parsed_registers.back();
        parsed_registers.pop_back();

        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        std::string operation = operations.back();
        operations.pop_back();

        instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation + ' ' + arg2->name;
  
        instruction->dst = dest;
        instruction->arg1 = arg1;
        instruction->arg2 = arg2;
        instruction->operation = operation;
        
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "--> added an arithmetic_assign instruction: " <<  instruction->instruction << std::endl;
    }
  };

  template<> struct action < compare_assign > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a compare_assign " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_cmpAssignment *instruction = new L3::Instruction_cmpAssignment();

        L3::Arg* arg2 = parsed_registers.back();
        parsed_registers.pop_back();

        L3::Arg* arg1 = parsed_registers.back();
        parsed_registers.pop_back();

        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        std::string operation = operations.back();
        operations.pop_back();

        instruction->instruction = dest->name + " <- " + arg1->name + ' ' + operation + ' ' + arg2->name;
  
        instruction->dst = dest;
        instruction->arg1 = arg1;
        instruction->arg2 = arg2;
        instruction->operation = operation;
        
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < load > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a load " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_Load *instruction = new L3::Instruction_Load();

        L3::Arg* src = parsed_registers.back();
        parsed_registers.pop_back();


        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();


        instruction->instruction = dest->name + " <- load " + src->name;
  
        instruction->dst = dest;
        instruction->src = src;
        
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < store > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a store " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_Store *instruction = new L3::Instruction_Store();

        L3::Arg* src = parsed_registers.back();
        parsed_registers.pop_back();


        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();


        instruction->instruction = "store " + dest->name + " <- " + src->name;
  
        instruction->dst = dest;
        instruction->src = src;
        
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < return_nothing > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a return_nothing " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_Return *instruction = new L3::Instruction_Return();

        instruction->instruction = "return" ;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < return_val > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a return_val " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_ReturnVal *instruction = new L3::Instruction_ReturnVal();

        L3::Arg* val = parsed_registers.back();
        parsed_registers.pop_back();



        instruction->instruction = "return " + val->name;
  
        instruction->retVal = val;
        
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < call > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a call " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_Call *instruction = new L3::Instruction_Call();

        L3::Arg* a = parsed_registers.back();
        parsed_registers.pop_back();

        while(a->type != CALLEE) {
          if(DEBUGGING) std::cout << "Adding call paramter: " <<  a->name << std::endl;
          instruction->parameters.push_back(a);

          a = parsed_registers.back();
          parsed_registers.pop_back();
        }

        instruction->callee = a;

        instruction->instruction = "call " + instruction->callee->name + " ( ";

        for (auto param : instruction->parameters) {
          instruction->instruction.append(" " + param->name);
        }
        instruction->instruction.append(" )");
  
        
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < call_assign > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a call_assign " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_CallAssign *instruction = new L3::Instruction_CallAssign();

        L3::Arg* a = parsed_registers.back();
        parsed_registers.pop_back();

        while(a->type != CALLEE) {
          if(DEBUGGING) std::cout << "Adding call paramter: " <<  a->name << std::endl;
          instruction->parameters.push_back(a);

          a = parsed_registers.back();
          parsed_registers.pop_back();
        }

        instruction->callee = a;

        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        instruction->dst = dest;

        instruction->instruction = instruction->dst->name + " <- call " + instruction->callee->name + " ( ";

        for (auto param : instruction->parameters) {
          instruction->instruction.append(" " + param->name);
        }
        instruction->instruction.append(" )");
  
        
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < label_inst > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a label_inst " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_Label *instruction = new L3::Instruction_Label();

        L3::Arg* label = parsed_registers.back();
        parsed_registers.pop_back();

        instruction->instruction = label->name;
        instruction->label = label;
        
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "--> added a label_inst instruction " <<  instruction->instruction << std::endl;

    }
  };

  template<> struct action < br_single > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a br_single " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_br *instruction = new L3::Instruction_br();

        L3::Arg* label = parsed_registers.back();
        parsed_registers.pop_back();

        instruction->instruction = "br " + label->name;
        instruction->label = label;
        
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "--> added a br_single instruction: " <<  instruction->instruction << std::endl;

    }
  };

  template<> struct action < br_cmp > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a br_cmp " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        
        L3::Instruction_brCmp *instruction = new L3::Instruction_brCmp();
        
        L3::Arg* falseLabel = parsed_registers.back();
        parsed_registers.pop_back();
        
        L3::Arg* trueLabel = parsed_registers.back();
        parsed_registers.pop_back();
        
        L3::Arg* comparitor = parsed_registers.back();
        parsed_registers.pop_back();

        instruction->instruction = "br " + comparitor->name + ' ' + trueLabel->name + ' ' + falseLabel->name;
        instruction->comparitor = comparitor;
        instruction->trueLabel = trueLabel;
        instruction->falseLabel = falseLabel;
        
        currentF->instructions.push_back(instruction);

    }
  };

  template<> struct action < comment > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
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
    if(DEBUGGING) std::cout << "Checking the grammar" << std::endl;
    pegtl::analyze< L3::grammar >();
    if(DEBUGGING) std::cout << "Finished checking grammar" << std::endl;


    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    L3::Program p;
    if(DEBUGGING) std::cout << "Begin Parsing" << std::endl;
    Function* mainF = new L3::Function();
    mainF->name = ":main";
    p.functions.push_back(mainF);
    parse< L3::grammar, L3::action >(fileInput, p);
    if(DEBUGGING) std::cout << "Done Parsing" << std::endl;
    return p;
  }


};// L3
