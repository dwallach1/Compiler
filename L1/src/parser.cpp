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

#include <L1.h>
#include <parser.h>
#define DEBUGGING 1
#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;

using namespace pegtl;
using namespace std;

namespace L1 {

  /* 
   * Data required to parse
   */ 
  std::vector<std::string> parsed_registers;
  std::vector<std::string> assignmentVec;
  std::vector<std::string> compareVec;
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

  struct label:
    pegtl::seq<
      seps,
      pegtl::one<':'>,
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

    struct reg:
      pegtl::seq<
        pegtl::sor<
          pegtl::string< 'r','d','i' >,
          pegtl::string< 'r','s','i' >,
          pegtl::string< 'r','d','x' >,
          pegtl::string< 'r','c','x' >,
          pegtl::string< 'r','s','p' >,
          pegtl::string< 'r','8' >,
          pegtl::string< 'r','9' >,
          pegtl::string< 'r','a','x' >,
          pegtl::string< 'r','1', '0' >,
          pegtl::string< 'r','1', '1' >,
          pegtl::string< 'r','1', '2' >,
          pegtl::string< 'r','1', '3' >,
          pegtl::string< 'r','1', '4' >,
          pegtl::string< 'r','1', '5' >,
          pegtl::string< 'r','b', 'p' >,
          pegtl::string< 'r','b', 'x' >
        >
      > {};

    struct memory:
      pegtl::seq<
        seps,
        pegtl::string< 'm', 'e', 'm'>,
        seps,
        reg,
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


    struct compare_assign:
      pegtl::seq<
        seps,
        pegtl::seq<
          seps,
          reg,
          seps,
          assignOp,
          seps,
          pegtl::sor<
            reg,
            number
          >
        >,
        seps,
        comparison,
        seps,
        pegtl::sor< 
          reg,
          number
        >
      >{};

   struct assignment:
    pegtl::seq<
      seps,
      reg,
      seps,
      assignOp,
      seps,
      pegtl::sor<
        reg,
        number,
        label
      >,
      seps,
      pegtl::not_at<
        comparison
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
          pegtl::one< '=' >,
          pegtl::one< '+' >,
          pegtl::one< '-' >
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

  struct arithmetic:
    pegtl::seq<
      seps,
      pegtl::sor<
        number,
        reg,
        memory
      >,
      seps,
      arithOp,
      seps,
      pegtl::sor<
        number,
        reg
      >
    >{};


  struct inc_dec:
    pegtl::seq<
      seps,
      reg,
      seps,
      pegtl::sor<
        pegtl::string< '+', '+' >,
        pegtl::string< '-','-' >
      >

    >{}; 

  struct load:
    pegtl::seq<
      seps,
      reg,
      seps,
      pegtl::one< '<' >,
      pegtl::one< '-' >,
      seps,
      memory
    >{};

  struct store:
    pegtl::seq<
      seps,
      memory,
      seps,
      pegtl::one< '<' >,
      pegtl::one< '-' >,
      seps,
      pegtl::sor<
        number,
        reg,
        label
      >
    >{};

  
  struct compare:
    pegtl::seq<
      seps,
      pegtl::string<'c', 'm', 'p'>,
      seps,
      pegtl::sor<
        reg,
        number
      >,
      seps,
      comparison,
      seps,
      pegtl::sor<
        reg,
        number
      >
    >{};

    

    struct cjump:
      pegtl::seq<
        seps,
        pegtl::string< 'c', 'j', 'u', 'm', 'p'>,
        seps,
        pegtl::sor<
          reg,
          number
        >,
        seps,
        comparison,
        seps,
        pegtl::sor<
          reg,
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


  struct call:
    pegtl::seq<
      seps,
      pegtl::string< 'c', 'a', 'l', 'l'>,
      seps,
      pegtl::sor<
        label,
        pegtl::string<'p', 'r', 'i', 'n', 't'>,
        pegtl::string<'a', 'l', 'l', 'o', 'c', 'a', 't', 'e'>,
        pegtl::string<'a', 'r', 'r', 'a', 'y', '-', 'e', 'r', 'r', 'o', 'r'>,
        reg
      >,
      seps,
      number
    >{};

  struct lea:
    pegtl::seq<
      seps,
      reg,
      seps,
      pegtl::one<'@'>,
      seps,
      reg,
      seps,
      reg,
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
      compare,
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


  // struct L1_label_rule:
  //   label {};
 
  struct L1_function_rule:
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

  struct L1_functions_rule:
    pegtl::seq<
      seps,
      pegtl::plus< L1_function_rule >
    > {};

  struct entry_point_rule:
    pegtl::seq<
      seps,
      pegtl::one< '(' >,
      seps,
      label,
      seps,
      L1_functions_rule,
      seps,
      pegtl::one< ')' >,
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

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      if (p.entryPointLabel.empty()){
        p.entryPointLabel = in.string();
        
      }
      else {
        parsed_registers.push_back(in.string());
        if(DEBUGGING) std::cout << "returning from label " <<  in.string() << std::endl;
      }
    }
  };

  template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Function *newF = new L1::Function();
      newF->name = in.string();
      p.functions.push_back(newF);
      if(DEBUGGING) std::cout << "returning from new function " <<  in.string() << std::endl;
    }
  };

  // template<> struct action < L1_label_rule > {
  //   template< typename Input >
		// static void apply( const Input & in, L1::Program & p){
  //     L1_item i;
  //     i.labelName = in.string();
  //     parsed_registers.push_back(i);
  //     if(DEBUGGING) std::cout << "returning from L1 label " <<  in.string() << std::endl;
  //   }
  // };

  template<> struct action < arithmetic > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();

        std::string source = parsed_registers.pop_back();
        std::string dest = parsed_registers.pop_back();
        std::string oper = assignmentVec.pop_back();
        instruction->instruction = dest + ' ' + oper + ' ' + source;
        instruction->registers.push_back(dest);
        instruction->registers.push_back(source);
        instruction->operation.push_back(oper);

        instruction->type = 0;
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "returning from arithmetic " <<  in.string() << std::endl;
    }
  };

  template<> struct action < assignment > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();

        std::string source = parsed_registers.pop_back();
        std::string dest = parsed_registers.pop_back();
        std::string oper = assignmentVec.pop_back();
        instruction->instruction = dest + ' ' + oper + ' ' + source;
        instruction->registers.push_back(dest);
        instruction->registers.push_back(source);
        instruction->operation.push_back(oper);
        
        instruction->type = 1;

        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "returning from assignment " <<  in.string() << std::endl;
    }
  };

  template<> struct action < load > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();

        std::string source = parsed_registers.pop_back();
        std::string dest = parsed_registers.pop_back();
        std::string oper = assignmentVec.pop_back();
        instruction->instruction = dest + ' ' + oper + ' ' + source;
        instruction->registers.push_back(dest);
        instruction->registers.push_back(source);
        instruction->operation.push_back(oper);
        
        instruction->type = 2;
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "returning from load " <<  in.string() << std::endl;
    }
  };

  template<> struct action < store > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        
        std::string source = parsed_registers.pop_back();
        std::string dest = parsed_registers.pop_back();
        std::string oper = assignmentVec.pop_back();
        instruction->instruction = dest + ' ' + oper + ' ' + source;
        instruction->registers.push_back(dest);
        instruction->registers.push_back(source);
        instruction->operation.push_back(oper);
        

        instruction->type = 3;
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "returning from store " <<  in.string() << std::endl;
    }
  };

  template<> struct action < reg > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        parsed_registers.push_back(in.string());
    }
  };
    template<> struct action < memory > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        parsed_registers.push_back(in.string());
    }
  };

  template<> struct action < compare > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 4;
        if(DEBUGGING) printf("This instruction doesn't actually exist\n");
    }
  };

  template<> struct action < compare_assign > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        

        std::string comparitor = parsed_registers.pop_back();
        std::string source = parsed_registers.pop_back();
        std::string dest = parsed_registers.pop_back();
        std::string oper = assignmentVec.pop_back();
        std::string compareOp = compareVec.pop_back();

        instruction->instruction = dest + ' ' + oper + ' ' + source + ' ' + compareOp + ' ' + comparitor;
        instruction->registers.push_back(dest);
        instruction->registers.push_back(source);
        instruction->registers.push_back(comparitor);
        instruction->operation.push_back(oper);
        instruction->operation.push_back(compareOp);
        

        instruction->type = 10;
        if(DEBUGGING) std::cout << "returning from compare_assign " <<  in.string() << std::endl;
        currentF->instructions.push_back(instruction);
    }
  };


  template<> struct action < label_inst > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = parsed_registers.pop_back();
        instruction->registers.push_back(instruction->instruction);
        instruction->type = 11;
        if(DEBUGGING) std::cout << "returning from label_inst " <<  in.string() << std::endl;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < inc_dec > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();

        std::string dest = parsed_registers.pop_back();
        std::string oper = assignmentVec.pop_back();
        instruction->instruction = "cjump " + dest + ' ' + comparitor + ' ' + source + ' ' label2 + ' ' + label1;
        instruction->registers.push_back(dest);
        instruction->operation.push_back(oper);

        instruction->type = 12;
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "returning from inc_dec " <<  in.string() << std::endl;
    }
  };

  template<> struct action < cjump > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        
        std::string label1 = parsed_registers.pop_back();
        std::string label2 = parsed_registers.pop_back();
        std::string source = parsed_registers.pop_back();
        std::string dest = parsed_registers.pop_back();
        std::string comparitor = compareVec.pop_back();
        instruction->instruction = "cjump " + dest + ' ' + comparitor + ' ' + source + ' ' label2 + ' ' + label1;
        instruction->registers.push_back(label1);
        instruction->registers.push_back(label2);
        instruction->registers.push_back(source);
        instruction->registers.push_back(dest);
        instruction->operation.push_back(comparitor);

        instruction->type = 5;
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "returning from cjump " <<  in.string() << std::endl;
    }
  };


  template<> struct action < goto_inst > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();

        std::string label = parsed_registers.pop_back();
        instruction->instruction = "goto " + label;
        instruction->registers.push_back(label);
        
        
        instruction->type = 6;
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "returning from goto inst " <<  in.string() << std::endl;
    }
  };

  template<> struct action < return_inst > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 7;
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "returning from return_inst " <<  in.string() << std::endl;
    }
  };

  template<> struct action < call > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 8;
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "returning from call " <<  in.string() << std::endl;
    }
  };

  template<> struct action < comment > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        if(DEBUGGING) std::cout << "Found a comment " <<  in.string() << std::endl;
    }
  };

  template<> struct action < lea > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 9;
        currentF->instructions.push_back(instruction);
        if(DEBUGGING) std::cout << "returning from lea " <<  in.string() << std::endl;
    }
  };

  template<> struct action < argument_number > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Function *currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
      if(DEBUGGING) std::cout << "returning from argument number " <<  in.string() << std::endl;
    }
  };

  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      parsed_registers.push_back(in.string());
    }
  };


  template<> struct action < local_number > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Function *currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
      if(DEBUGGING) std::cout << "returning from local number " <<  in.string() << std::endl;
    }
  };

  template<> struct action < arithOp > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      assignmentVec.push_back(in.string());
    }
  };

  template<> struct action < assignOp > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      assignmentVec.push_back(in.string());
    }
  };

  template<> struct action < comparison > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      compareVec.push_back(in.string());
    }
  };
  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      parsed_registers.push_back(in.string());
    }
  };

  

  Program parse_file (char *fileName){

    /* 
     * Check the grammar for some possible issues.
     */
    pegtl::analyze< L1::grammar >();

    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    L1::Program p;
    parse< L1::grammar, L1::action >(fileInput, p);
    return p;
  }

} // L1
