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
  std::vector<L1_item> parsed_registers;

  /* 
   * Grammar rules from now on.
   */
  struct label:
    pegtl::seq<
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
          pegtl::one< '<' >,
          pegtl::one< '-' >,
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
      pegtl::one< '<' >,
      pegtl::one< '-' >,
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

  struct arithmetic:
    pegtl::seq<
      seps,
      pegtl::sor<
        number,
        reg,
        memory
      >,
      seps,
      pegtl::sor<
        pegtl::one< '-' >,
        pegtl::one< '+' >,
        pegtl::one< '*' >,
        pegtl::string< '>', '>' >,
        pegtl::string< '<', '<' >,
        pegtl::one< '&' >
      >,
      pegtl::one< '=' >,
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
    pegtl::seq<
      seps,
      pegtl::not_at<
        pegtl::sor<
          pegtl::string<'g', 'o', 't', 'o'>,
          pegtl::string<'<', '-'>
        >
      >,
      seps,
      label,
      seps,
      pegtl::not_at<
        number
      >
    >{};

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


  struct L1_label_rule:
    label {};
 
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
        //std::cout << "returning from label " <<  in.string() << std::endl;
      }
    }
  };

  template<> struct action < function_name > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Function *newF = new L1::Function();
      newF->name = in.string();
      p.functions.push_back(newF);
    }
  };

  template<> struct action < L1_label_rule > {
    template< typename Input >
		static void apply( const Input & in, L1::Program & p){
      L1_item i;
      i.labelName = in.string();
      parsed_registers.push_back(i);
    }
  };

  template<> struct action < arithmetic > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 0;
        currentF->instructions.push_back(instruction);
        //std::cout << "returning from arithmetic " <<  in.string() << std::endl;
    }
  };

  template<> struct action < assignment > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 1;

        currentF->instructions.push_back(instruction);
        //std::cout << "returning from assignment " <<  in.string() << std::endl;
    }
  };

  template<> struct action < load > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 2;
        currentF->instructions.push_back(instruction);
        //std::cout << "returning from load " <<  in.string() << std::endl;
    }
  };

  template<> struct action < store > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 3;
        currentF->instructions.push_back(instruction);
        //std::cout << "returning from store " <<  in.string() << std::endl;
    }
  };

  template<> struct action < compare > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 4;
        //printf("This instruction doesn't actually exist\n");
    }
  };

  template<> struct action < compare_assign > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 10;
        //std::cout << "returning from compare_assign " <<  in.string() << std::endl;
        currentF->instructions.push_back(instruction);
    }
  };


  template<> struct action < label_inst > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 11;
        //std::cout << "returning from label_inst " <<  in.string() << std::endl;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < inc_dec > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 12;
        currentF->instructions.push_back(instruction);
        //std::cout << "returning from inc_dec " <<  in.string() << std::endl;
    }
  };

  template<> struct action < cjump > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 5;
        currentF->instructions.push_back(instruction);
    }
  };


  template<> struct action < goto_inst > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
        L1::Function *currentF = p.functions.back();
        L1::Instruction *instruction = new L1::Instruction();
        instruction->instruction = in.string();
        instruction->type = 6;
        currentF->instructions.push_back(instruction);
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
        //std::cout << "returning from return_inst " <<  in.string() << std::endl;
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
        //std::cout << "returning from call " <<  in.string() << std::endl;
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
        //std::cout << "returning from lea " <<  in.string() << std::endl;
    }
  };

  template<> struct action < argument_number > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Function *currentF = p.functions.back();
      currentF->arguments = std::stoll(in.string());
    }
  };

  template<> struct action < local_number > {
    template< typename Input >
    static void apply( const Input & in, L1::Program & p){
      L1::Function *currentF = p.functions.back();
      currentF->locals = std::stoll(in.string());
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
