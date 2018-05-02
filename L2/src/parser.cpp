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

#include <L2.h>
#include <parser.h>
#define DEBUGGING 0
#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L2 {

  /* 
   * Data required to parse
   */ 
  std::vector<L2::Arg*> parsed_registers;
  std::vector<std::string> assignmentVec;
  std::vector<std::string> compareVec;
  std::vector<std::string> labelInsts;
  int debugging = 0;

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
      pegtl::not_at<
        pegtl::sor<
          pegtl::string<'m', 'e', 'm'>,
          pegtl::string<'s', 't', 'a', 'c', 'k','-','a', 'r', 'g'>
        >
      >,
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

  struct label:
    pegtl::seq<
      seps,
      pegtl::one<':'>,
      var
    > {};

  struct function_name:
    label {};

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

  struct argument_number:
    number {};

  struct local_number:
    number {} ;



    /*
    *     START OF WORK
    *
    */

    

    struct memory:
      pegtl::seq<
        seps,
        pegtl::string< 'm', 'e', 'm'>,
        seps,
        var,
        seps,
        number
      >{};




    struct comparison:
    pegtl::seq<
      pegtl::sor<
        pegtl::string< '<', '=' >,
        pegtl::one< '<' >,
        pegtl::one< '=' >
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

  struct incOrDec:
    pegtl::seq<
      pegtl::sor<
        pegtl::string<'+', '+'>,
        pegtl::string<'-', '-'>
      >
    >
  {};

  struct assignOp:
    pegtl::seq<
      seps,
      pegtl::string< '<', '-' >
    >
  {};

    struct compare_assign:
      pegtl::seq<
        seps,
        pegtl::seq<
          seps,
          var,
          seps,
          assignOp,
          seps,
          pegtl::sor<
            var,
            number
          >
        >,
        seps,
        comparison,
        seps,
        pegtl::sor< 
          var,
          number
        >
      >{};

   struct stackArg:
    pegtl::seq<
      seps,
      pegtl::string<'s', 't', 'a', 'c', 'k','-','a','r','g'>,
      seps,
      number
    >{};

   struct assignment:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      pegtl::sor<
        var,
        number,
        label
      >,
      seps,
      pegtl::not_at<
        comparison
      >
    >{};



  struct arithmetic:
    pegtl::seq<
      seps,
      pegtl::sor<
        number,
        var,
        memory
      >,
      seps,
      arithOp,
      seps,
      pegtl::sor<
        number,
        var,
        memory
      >
    >{};


  struct inc_dec:
    pegtl::seq<
      seps,
      var,
      seps,
      incOrDec
    >{}; 

  struct load:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      pegtl::sor<
        memory,
        stackArg
      >
    >{};

  struct store:
    pegtl::seq<
      seps,
      memory,
      seps,
      assignOp,
      seps,
      pegtl::sor<
        number,
        var,
        label
      >
    >{};
   

    struct cjump:
      pegtl::seq<
        seps,
        pegtl::string< 'c', 'j', 'u', 'm', 'p'>,
        seps,
        pegtl::sor<
          var,
          number
        >,
        seps,
        comparison,
        seps,
        pegtl::sor<
          var,
          number
        >,
        seps,
        label,
        seps,
        label
      >{};


  struct goto_inst:
    pegtl::seq<
      seps,
      pegtl::string< 'g', 'o', 't', 'o'>,
      seps,
      label
    >{};


  struct return_inst:
    pegtl::seq<
      seps,
      pegtl::string<'r', 'e', 't', 'u', 'r', 'n'>
    >{};

  struct paa_value:
    pegtl::sor<
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
      pegtl::sor<
        label,
        paa_value,
        var
      >,
      seps,
      number
    >{};

  struct lea:
    pegtl::seq<
      seps,
      var,
      seps,
      pegtl::one<'@'>,
      seps,
      var,
      seps,
      var,
      seps,
      number
    >{};


  struct label_inst:
    label {};

  struct instruction:
    pegtl::sor<
      assignment,
      arithmetic,
      load,
      store,
      cjump,
      goto_inst,
      return_inst,
      call,
      lea,
      label_inst,
      compare_assign,
      inc_dec
    >{};


  struct instructions_rules:
    pegtl::seq< 
      seps,
      pegtl::plus< instruction >
    >{};

 
  struct L2_function_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      function_name,
      seps,
      argument_number,
      seps,
      local_number,
      seps,
      instructions_rules,
      seps,
      pegtl::one< ')' >,
      seps
    > {};

  struct L2_functions_rule:
    pegtl::seq<
      seps,
      pegtl::plus< L2_function_rule >
    > {};

  struct L2_Spill_Rule:
    pegtl::seq<
    L2_function_rule,
    seps,
    var,
    seps,
    var
    >
  {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      label,
      seps,
      L2_functions_rule,
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
      L2_function_rule
    >{};

  struct spill_grammar :
    pegtl::must<
      L2_Spill_Rule
    >{};


  /* 
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if (p.entryPointLabel.empty()){
        if(DEBUGGING) std::cout << "found entry point " <<  in.string() << std::endl;
        p.entryPointLabel = in.string();
        

      }
      else {
        if(DEBUGGING) std::cout << "returning from label " <<  in.string() << std::endl;
        parsed_registers.pop_back(); // get rid of var part of label
        L2::Arg* arg = new L2::Arg;
        arg->name = in.string();
        arg->type = LBL;
        parsed_registers.push_back(arg);
        labelInsts.push_back(in.string());
      }
    }
  };

  template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if(DEBUGGING) std::cout << "Found a new function " <<  in.string() << std::endl;
      L2::Function *newF = new L2::Function();
      newF->name = in.string();
      p.functions.push_back(newF);
    }
  };



  template<> struct action < arithmetic > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found an arithmetic " <<  in.string() << std::endl;
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();

        L2::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* dest = parsed_registers.back();
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
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "found an assignment " <<  in.string() << std::endl;
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();

        L2::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* dest = parsed_registers.back();
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
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "found a load " <<  in.string() << std::endl;
        
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();

        L2::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();

        L2::Arg* num = new L2::Arg();
        num = parsed_registers.back();
        parsed_registers.pop_back();

        L2::Arg* dest = parsed_registers.back();
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
          L2::Arg* newArg = new L2::Arg();
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
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "found a store " <<  in.string() << std::endl;
        
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();
        
        L2::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();

        L2::Arg* dest = parsed_registers.back();
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
        L2::Arg* newArg = new L2::Arg();
        newArg->name = regInMem;
        newArg->type = MEM;
        instruction->arguments.push_back(newArg);
        instruction->operation.push_back(oper);
        

        instruction->type = STORE;
        currentF->instructions.push_back(instruction);
    }
  };


    template<> struct action < memory > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a memory " << in.string() << std::endl;
        //getting rid of number in parsed registers and the memory register
        parsed_registers.pop_back();
        parsed_registers.pop_back();

        L2::Arg* arg = new L2::Arg();
        arg->name = in.string();
        arg->type = MEM;
        parsed_registers.push_back(arg);
    }
  };

  template<> struct action < stackArg > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a stackArg " << in.string() << std::endl;


      
        L2::Arg* arg = new L2::Arg();
        arg->name = in.string();
        arg->type = S_ARG;
        parsed_registers.push_back(arg);
    }
  };

  template<> struct action < compare_assign > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a compare_assign " <<  in.string() << std::endl;
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();
        

        L2::Arg* comparitor = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* dest = parsed_registers.back();
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
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a label_inst " <<  in.string() << std::endl;
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();
        int found = 0;

        L2::Arg* arg = new L2::Arg();
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

        L2::Arg* arg1 = new L2::Arg();
        arg1->name = instruction->instruction;
        arg1->type = LBL;
        instruction->arguments.push_back(arg1);

        instruction->type = LABEL;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < inc_dec > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a inc_dec " <<  in.string() << std::endl;
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();

        L2::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();
        std::string oper = assignmentVec.back();
        assignmentVec.pop_back();
        instruction->instruction = dest->name + ' ' + oper;
        instruction->arguments.push_back(dest);
        instruction->operation.push_back(oper);

        instruction->type = INC_DEC;
        currentF->instructions.push_back(instruction);
    }
  };


  template<> struct action < var > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a var " <<  in.string() << std::endl;

        L2::Arg* arg = new L2::Arg();
        arg->name = in.string();
        arg->type = VAR; 
        parsed_registers.push_back(arg);
    }
  };


  template<> struct action < cjump > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a cjump " <<  in.string() << std::endl;
        
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();
        
        L2::Arg* label1 = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* label2 = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* source = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();
        std::string comparitor = compareVec.back();
        compareVec.pop_back();

        if(label1->name[0] != ':'){
          label1->name.insert(0,1,':');
          label1->type = LBL;
        }
        if(label2->name[0] != ':'){
          label2->name.insert(0,1,':');
          label2->type = LBL;
        }
        instruction->instruction = "cjump " + dest->name + ' ' + comparitor + ' ' + source->name + ' ' + label2->name + ' ' + label1->name;
        if(DEBUGGING) printf("Wrote to the inst: %s\n", instruction->instruction.c_str());
        instruction->arguments.push_back(label1);
        instruction->arguments.push_back(label2);
        instruction->arguments.push_back(source);
        instruction->arguments.push_back(dest);
        instruction->operation.push_back(comparitor);

        instruction->type = CJUMP;
        currentF->instructions.push_back(instruction);
    }
  };


  template<> struct action < goto_inst > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a goto inst " <<  in.string() << std::endl;
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();

        L2::Arg* label = parsed_registers.back();
        parsed_registers.pop_back();
        instruction->instruction = "goto " + label->name;
        instruction->arguments.push_back(label);
        instruction->arguments.push_back(label);
        
        
        instruction->type = GOTO;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < return_inst > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a return_inst " <<  in.string() << std::endl;
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();
        instruction->instruction = "return";
        instruction->type = RET;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < call > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a call " <<  in.string() << std::endl;
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();
        
        L2::Arg* args = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* callee = parsed_registers.back();
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
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "Found a comment " <<  in.string() << std::endl;
    }
  };

  template<> struct action < lea > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
        if(DEBUGGING) std::cout << "found a lea " <<  in.string() << std::endl;
        
        L2::Function *currentF = p.functions.back();
        L2::Instruction *instruction = new L2::Instruction();
        
        L2::Arg* num = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* multer = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* adder = parsed_registers.back();
        parsed_registers.pop_back();
        L2::Arg* dest = parsed_registers.back();
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
    static void apply( const Input & in, L2::Program & p){
      if(DEBUGGING) std::cout << "found an argument number " <<  in.string() << std::endl;
      
      L2::Function *currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if(DEBUGGING) std::cout << "Found a number " << in.string() << std::endl;

      L2::Arg* arg = new L2::Arg();
      arg->name = in.string();
      arg->type = NUM;
      parsed_registers.push_back(arg);

    }
  };


  template<> struct action < local_number > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if(DEBUGGING) std::cout << "Found a local number " <<  in.string() << std::endl;
      L2::Function *currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
    }
  };

  template<> struct action < arithOp > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if(DEBUGGING) std::cout << "Found an arithmetic operation " << in.string() << std::endl;
      assignmentVec.push_back(in.string());

    }
  };

  template<> struct action < incOrDec > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if(DEBUGGING) std::cout << "Found an inc/dec operation " << in.string() << std::endl;
      assignmentVec.push_back(in.string());

    }
  };

  template<> struct action < assignOp > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if(DEBUGGING) std::cout << "Found an assignment operation " << in.string() << std::endl;
      assignmentVec.push_back(in.string());

    }
  };

  template<> struct action < paa_value > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if(DEBUGGING) std::cout << "Found a print allocate or array-error " << in.string() << std::endl;
      L2::Arg* arg = new L2::Arg();
      if(in.string().find("array-error") != std::string::npos){
        arg->name = "array_error";
      }
      else{
        arg->name = in.string();
      }
      arg->type = LBL;
      parsed_registers.push_back(arg);
    }
  };

  template<> struct action < comparison > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if(DEBUGGING) std::cout << "Found a comparison " << in.string() << std::endl;
      compareVec.push_back(in.string());

    }
  };

  template<> struct action < L2_Spill_Rule > {
    template< typename Input >
    static void apply( const Input & in, L2::Program & p){
      if(DEBUGGING) std::cout << "Found a SPILL function " << in.string() << std::endl;
      L2::Function *currentF = p.functions.back();
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
    pegtl::analyze< L2::grammar >();

    if(DEBUGGING) std::cout << "Finished checking grammar" << std::endl;


    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    L2::Program p;
    if(DEBUGGING) std::cout << "Begin Parsing" << std::endl;
    parse< L2::grammar, L2::action >(fileInput, p);
    if(DEBUGGING) std::cout << "Done Parsing" << std::endl;
    return p;
  }

  Program parse_function_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    if(DEBUGGING) std::cout << "Checking the grammar" << std::endl;
    pegtl::analyze< L2::function_grammar >();

    if(DEBUGGING) std::cout << "Finished checking grammar" << std::endl;


    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    L2::Program p;
    if(DEBUGGING) std::cout << "Begin Parsing Function File" << std::endl;
    parse<L2::function_grammar, L2::action>(fileInput, p);
    if(DEBUGGING) std::cout << "Done Parsing Function File" << std::endl;
    return p;
  }

  Program parse_spill_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    if(DEBUGGING) std::cout << "Checking the grammar" << std::endl;
    pegtl::analyze< L2::spill_grammar >();

    if(DEBUGGING) std::cout << "Finished checking grammar" << std::endl;


    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    L2::Program p;
    if(DEBUGGING) std::cout << "Begin Parsing Spill File" << std::endl;
    parse<L2::spill_grammar, L2::action>(fileInput, p);
    if(DEBUGGING) std::cout << "Done Parsing Spill File" << std::endl;
    return p;
  }

};// L2
