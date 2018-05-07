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
#define DEBUGGING 0
#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L3 {

  /* 
   * Data required to parse
   */ 
  std::vector<L3::Arg*> parsed_registers;
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
    pegtl::sor<
      pegtl::one< '+' >,
      pegtl::one< '-' >,
      pegtl::one< '*' >,
      pegtl::one< '&' >,
      pegtl::string< '<', '<' >,
      pegtl::string< '>', '>' >
    >{};

  struct cmp:
    pegtl::seq<
      pegtl::sor<
        pegtl::string< '<', '=' >,
        pegtl::one< '<' >,
        pegtl::one< '=' >,
        pegtl::one< '>' >,
        pegtl::string< '>', '=' >
      >
    >{};

    struct arithOp:
    pegtl::seq<
    seps,
      pegtl::seq<
        pegtl::sor<
            pegtl::one< '-' >,
            pegtl::one< '+' >,
            pegtl::one< '*' >,
            pegtl::string< '>', '>' >,
            pegtl::string< '<', '<' >,
            pegtl::one< '&' >
        >,
        pegtl::sor<
          pegtl::one< '=' >
        >
      >
    >
  {};

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
      s
    >;

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
      s,
      seps,
      pegtl::not_at<
        comparison
      >
    >{};

  struct arithmetic_assign:
    pegtl::seq<
      seps,
      var,
      seps,
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
      >
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
      >
    >{};
   
  struct return_nothing:
    pegtl::seq<
      seps,
      pegtl::string<'r', 'e', 't', 'u', 'r', 'n'>
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
      args
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
      pegtl::one< '(' >,
      seps,
      vars,
      seps,
      pegtl::one< ')' >
    >{};
 
  struct L3_function_rule:
    pegtl::seq<
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


  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      label,
      seps,
      L3_functions_rule,
      seps,
      pegtl::one< ')' >,
      seps
    > { };

  struct grammar : 
    pegtl::must< 
      entry_point_rule
    > {};

  struct function_grammar :
    pegtl::must<
      L3_function_rule
    >{};



  /* 
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if (p.entryPointLabel.empty()){
        if(DEBUGGING) std::cout << "found entry point " <<  in.string() << std::endl;
        p.entryPointLabel = in.string();
        

      }
      else {
        if(DEBUGGING) std::cout << "returning from label " <<  in.string() << std::endl;
        parsed_registers.pop_back(); // get rid of var part of label
        L3::Arg* arg = new L3::Arg;
        arg->name = in.string();
        arg->type = LBL;
        parsed_registers.push_back(arg);
        labelInsts.push_back(in.string());
      }
    }
  };

  template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "Found a new function " <<  in.string() << std::endl;
      L3::Function *newF = new L3::Function();
      newF->name = in.string();
      p.functions.push_back(newF);
    }
  };



  template<> struct action < arithmetic > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "Found an arithmetic " <<  in.string() << std::endl;
        L3::Function *currentF = p.functions.back();
        L3::Instruction *instruction = new L3::Instruction();

        L3::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();
        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();
        std::string oper = assignmentVec.back();
        assignmentVec.pop_back();
        instruction->instruction = dest->name + ' ' + oper + ' ' + source->name;
        instruction->arguments.push_back(dest);
        instruction->arguments.push_back(source);
        instruction->operation.push_back(oper);

        if(DEBUGGING) printf("Pushing back the instruction: %s\n", instruction->instruction.c_str());

        instruction->type = AOP;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < assignment > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found an assignment " <<  in.string() << std::endl;
        L3::Function *currentF = p.functions.back();
        L3::Instruction *instruction = new L3::Instruction();

        L3::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();
        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();
        std::string oper = assignmentVec.back();
        assignmentVec.pop_back();

        size_t loc = in.string().find(source->name);
        if(loc != std::string::npos){
          //Checking to see if the callee is actually a label
          if (in.string()[loc-1] == ':'){
            //Found a label
            source->name.insert(0,1,':');
            source->type = LBL;
         }
        }

        instruction->instruction = dest->name + ' ' + oper + ' ' + source->name;
        if(DEBUGGING) std::cout << "For the assignment, we wrote: " << instruction->instruction << std::endl;
        instruction->arguments.push_back(dest);
        instruction->arguments.push_back(source);
        instruction->operation.push_back(oper);
        
        instruction->type = ASSIGN;
        currentF->instructions.push_back(instruction);
    }
  };



  template<> struct action < load > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a load " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        L3::Instruction *instruction = new L3::Instruction();

        L3::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();

        L3::Arg* num = new L3::Arg();
        num = parsed_registers.back();
        parsed_registers.pop_back();

        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();
        std::string oper = assignmentVec.back();
        assignmentVec.pop_back();
        instruction->instruction = dest->name + ' ' + oper + ' ' + source->name;
 
        instruction->arguments.push_back(dest);
        instruction->arguments.push_back(source);
        instruction->operation.push_back(oper);
        if(source->type == MEM){ 
          std::string regInMem;
          for(int i = 4; source->name[i] != ' '; i++){
            regInMem.append(source->name.substr(i,1));
          }
          L3::Arg* newArg = new L3::Arg();
          newArg->name = regInMem;
          newArg->type = MEM;
          instruction->arguments.push_back(newArg);
        }
        
        if (source->type == S_ARG) { 
         
          instruction->arguments.push_back(num);
          if (DEBUGGING) printf("assigning instruction to STACKARG\n");
          instruction->type == STACKARG; 
        }
        else { instruction->type = LOAD; }
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) printf("Writing load as: %s\n", instruction->instruction.c_str());
    }
  };

  template<> struct action < store > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a store " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        L3::Instruction *instruction = new L3::Instruction();
        
        L3::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();

        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();
        std::string oper = assignmentVec.back();
        assignmentVec.pop_back();

        size_t loc = in.string().find(source->name);
        if(loc != std::string::npos){
          //Checking to see if the callee is actually a label
          if (in.string()[loc-1] == ':'){
            //Found a label
            source->name.insert(0,1,':');
            source->type = LBL;
          }
        }


        instruction->instruction = dest->name + ' ' + oper + ' ' + source->name;
        instruction->arguments.push_back(dest);
        instruction->arguments.push_back(source);
        std::string regInMem;
        for(int i = 4; dest->name[i] != ' '; i++){
          regInMem.append(dest->name.substr(i,1));
        }
        L3::Arg* newArg = new L3::Arg();
        newArg->name = regInMem;
        newArg->type = MEM;
        instruction->arguments.push_back(newArg);
        instruction->operation.push_back(oper);
        

        instruction->type = STORE;
        currentF->instructions.push_back(instruction);
    }
  };


  template<> struct action < compare_assign > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "Found a compare_assign " <<  in.string() << std::endl;
        L3::Function *currentF = p.functions.back();
        L3::Instruction *instruction = new L3::Instruction();
        

        L3::Arg* comparitor = parsed_registers.back();
        parsed_registers.pop_back();
        L3::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();
        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();
        std::string oper = assignmentVec.back();
        assignmentVec.pop_back();
        std::string compareOp = compareVec.back();
        compareVec.pop_back();

        instruction->instruction = dest->name + ' ' + oper + ' ' + source->name + ' ' + compareOp + ' ' + comparitor->name;
        instruction->arguments.push_back(dest);
        instruction->arguments.push_back(source);
        instruction->arguments.push_back(comparitor);
        instruction->operation.push_back(oper);
        instruction->operation.push_back(compareOp);
        

        instruction->type = CMP_ASSIGN;
        currentF->instructions.push_back(instruction);
    }
  };


  template<> struct action < label_inst > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "Found a label_inst " <<  in.string() << std::endl;
        L3::Function *currentF = p.functions.back();
        L3::Instruction *instruction = new L3::Instruction();
        int found = 0;

        L3::Arg* arg = new L3::Arg();
        arg->name = in.string();
        arg->type = LBL;
        instruction->arguments.push_back(arg);

        for(std::string curLabel : labelInsts){
          //finding the which label I need
          if(curLabel.find(in.string()) != std::string::npos){
            instruction->instruction = curLabel;
            labelInsts.push_back(curLabel);
            found = 1;
            if(DEBUGGING) std::cout << curLabel << " was found in " << in.string() << std::endl;
            break;
          }
          //The label has not been found yet (most likely a loop)  
        }
        if(!found){
            if(DEBUGGING) std::cout << "The label was not found in the vector of labels " <<  in.string() << std::endl;
            //Begin the label at the appropriate spot.
            std::string newLabel = in.string().substr(in.string().find(":"));
            labelInsts.push_back(newLabel);
            instruction->instruction = newLabel;
          }
        else{
          if(DEBUGGING) std::cout << "The label WAS found in the vector of labels " <<  in.string() << std::endl;
        }

        L3::Arg* arg1 = new L3::Arg();
        arg1->name = instruction->instruction;
        arg1->type = LBL;
        instruction->arguments.push_back(arg1);

        instruction->type = LABEL;
        currentF->instructions.push_back(instruction);
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



  template<> struct action < return_inst > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "Found a return_inst " <<  in.string() << std::endl;
        L3::Function *currentF = p.functions.back();
        L3::Instruction *instruction = new L3::Instruction();
        instruction->instruction = "return";
        instruction->type = RET;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < call > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "Found a call " <<  in.string() << std::endl;
        L3::Function *currentF = p.functions.back();
        L3::Instruction *instruction = new L3::Instruction();
        
        L3::Arg* args = parsed_registers.back();
        parsed_registers.pop_back();
        L3::Arg* callee = parsed_registers.back();
        //Need to add a new support for handling allowing vars, which will trigger the call when it may be calling a label
        size_t loc = in.string().find(callee->name);
        if(loc != std::string::npos){
          //Checking to see if the callee is actually a label
          if (in.string()[loc-1] == ':'){
            //Found a label
            callee->name.insert(0,1,':');
            callee->type = LBL;
         }
        }

        parsed_registers.pop_back();
        instruction->instruction = "call " + callee->name + ' ' + args->name;
        instruction->arguments.push_back(callee);
        instruction->arguments.push_back(args);

        instruction->type = CALL;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < comment > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "Found a comment " <<  in.string() << std::endl;
    }
  };

  template<> struct action < lea > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
        if(DEBUGGING) std::cout << "found a lea " <<  in.string() << std::endl;
        
        L3::Function *currentF = p.functions.back();
        L3::Instruction *instruction = new L3::Instruction();
        
        L3::Arg* num = parsed_registers.back();
        parsed_registers.pop_back();
        L3::Arg* multer = parsed_registers.back();
        parsed_registers.pop_back();
        L3::Arg* adder = parsed_registers.back();
        parsed_registers.pop_back();
        L3::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();
        instruction->instruction = dest->name + " @ " + adder->name + ' ' + multer->name + ' ' + num->name;
        instruction->arguments.push_back(dest);
        instruction->arguments.push_back(adder);
        instruction->arguments.push_back(multer);
        instruction->arguments.push_back(num);
        instruction->operation.push_back("@");

        instruction->type = LEA;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < argument_number > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "found an argument number " <<  in.string() << std::endl;
      
      L3::Function *currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
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


  template<> struct action < local_number > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "Found a local number " <<  in.string() << std::endl;
      L3::Function *currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
    }
  };

  template<> struct action < arithOp > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "Found an arithmetic operation " << in.string() << std::endl;
      assignmentVec.push_back(in.string());

    }
  };

  template<> struct action < incOrDec > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "Found an inc/dec operation " << in.string() << std::endl;
      assignmentVec.push_back(in.string());

    }
  };

  template<> struct action < assignOp > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "Found an assignment operation " << in.string() << std::endl;
      assignmentVec.push_back(in.string());

    }
  };

  template<> struct action < paa_value > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "Found a print allocate or array-error " << in.string() << std::endl;
      L3::Arg* arg = new L3::Arg();
      if(in.string().find("array-error") != std::string::npos){
        arg->name = "array-error";
      }
      else{
        arg->name = in.string();
      }
      arg->type = PAA;
      parsed_registers.push_back(arg);
    }
  };

  template<> struct action < comparison > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "Found a comparison " << in.string() << std::endl;
      compareVec.push_back(in.string());

    }
  };

  template<> struct action < L3_Spill_Rule > {
    template< typename Input >
    static void apply( const Input & in, L3::Program & p){
      if(DEBUGGING) std::cout << "Found a SPILL function " << in.string() << std::endl;
      L3::Function *currentF = p.functions.back();
      currentF->replaceSpill = (parsed_registers.back()->name);
      parsed_registers.pop_back();
      currentF->toSpill = (parsed_registers.back()->name);

    }
  };
  


  

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
    parse< L3::grammar, L3::action >(fileInput, p);
    if(DEBUGGING) std::cout << "Done Parsing" << std::endl;
    return p;
  }

  Program parse_function_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    if(DEBUGGING) std::cout << "Checking the grammar" << std::endl;
    pegtl::analyze< L3::function_grammar >();

    if(DEBUGGING) std::cout << "Finished checking grammar" << std::endl;


    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    L3::Program p;
    if(DEBUGGING) std::cout << "Begin Parsing Function File" << std::endl;
    parse<L3::function_grammar, L3::action>(fileInput, p);
    if(DEBUGGING) std::cout << "Done Parsing Function File" << std::endl;
    return p;
  }

  Program parse_spill_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    if(DEBUGGING) std::cout << "Checking the grammar" << std::endl;
    pegtl::analyze< L3::spill_grammar >();

    if(DEBUGGING) std::cout << "Finished checking grammar" << std::endl;


    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    L3::Program p;
    if(DEBUGGING) std::cout << "Begin Parsing Spill File" << std::endl;
    parse<L3::spill_grammar, L3::action>(fileInput, p);
    if(DEBUGGING) std::cout << "Done Parsing Spill File" << std::endl;
    return p;
  }

};// L3
