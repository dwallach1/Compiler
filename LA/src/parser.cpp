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
#include <LA.h>
#include <parser.h>
#include <tao/pegtl.hpp>
#include <tao/pegtl/analyze.hpp>
#include <tao/pegtl/contrib/raw_string.hpp>


#define DEBUGGING 0
#define DEBUG_S 0

namespace pegtl = tao::TAO_PEGTL_NAMESPACE;
using namespace pegtl;
using namespace std;

namespace LA {

  Arg* findVariable(Function* f, Arg* arg);
  /* 
   * Data requLAed to parse
   */ 


  vector<Arg *> parsed_registers;
  vector<Arg *> parsed_labels;
  vector<Type *> type_declarations;
  vector<Arg *> index_holder;
  vector<Operation *> operations;
  vector<Instruction* > basicBlockInsts;
  vector<Arg *> newFunctionArgs;

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

  struct keyname:
    pegtl::sor<
        pegtl::string< 'c', 'a', 'l', 'l', ' '>,
        pa,
        pegtl::string<  'r', 'e', 't', 'u', 'r', 'n', ' ' >
      >{};

  struct name:
    pegtl::seq<
      seps,
      //pegtl::not_at< keyname >,
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
      name
    >{};

  struct label:
    pegtl::seq<
      seps,
      pegtl::one<':'>,
      name
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
        pegtl::string< '<', '=' >,
        pegtl::string< '>', '=' >,
        pegtl::one< '<' >,
        pegtl::one< '=' >,
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
    seq<
      seps,
      pegtl::star<
        pegtl::seq<
          seps,
          t,
          pegtl::opt<
            seq<
              seps,
              pegtl::one<','>
            >
          >
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
        var,
        name
      >{};

  struct type:
    pegtl::seq<
      seps,
      pegtl::sor<
        pegtl::seq<
          pegtl::string< 'i', 'n', 't', '6', '4' >,
          seps,
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

  struct index:
    pegtl::seq<
      seps,
      pegtl::one< '[' >,
      t,
      pegtl::one< ']' > 
    >{};

  struct assignment:
    pegtl::seq<
      seps,
      var,
      seps,
      assignOp,
      seps,
      not_at<
        sor<
          keyname
        >
      >,
      s,
      not_at<
        sor<
          index,
          op
        >
      >
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

  struct f_name: 
    pegtl::seq<
      seps,
      T,
      seps,
      name,
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
      i_star ,
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
   * Actions attached to grammar rules.
   */
  template< typename Rule >
  struct action : pegtl::nothing< Rule > {};

  template<> struct action < f_name > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      
      if(DEBUGGING) cout << "Found a new function " <<  in.string() << endl;
      
      Function *newF = new Function();
      
      Arg* parameter = parsed_registers.back();
      parsed_registers.pop_back();

      Label* funcName = dynamic_cast<Label *> (parameter);


      while(funcName == NULL) {
        
        newF->declared_variables.insert(newFunctionArgs.back());
        if(DEBUGGING) cout << "Added arg: " << newFunctionArgs.back()->name << " to declared_variables\n";
        newFunctionArgs.pop_back();
        newF->parameters.push_back(parameter);
        parameter = parsed_registers.back();
        parsed_registers.pop_back();
        funcName = dynamic_cast<Label *> (parameter);
      }
      newFunctionArgs = {};
      newF->name = funcName;

      Type* returnType = type_declarations.back();
      type_declarations.pop_back();

      newF->returnType = returnType;
      if(DEBUGGING) cout << "Pushing back new function\n";
      p.functions.push_back(newF);
      parsed_labels = {};
    }
  };

  template<> struct action < label > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      if(DEBUGGING) cout << "Found label: " <<  in.string() << endl;
      
       
      

      Label* label = new Label();
      label->name = in.string();
      parsed_registers.push_back(label);
      parsed_labels.push_back(label);

      VoidT* noType = new VoidT();
      label->type = noType;
      if(DEBUGGING) cout << "returning from label: " <<  in.string() << endl;


    }
  };

  template<> struct action < index > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      if(DEBUGGING) cout << "Found an index: " << in.string() << endl;

        // it will be a number -- get the number part [ num ]
        // Arg* newIndex = parsed_registers.back();
        // parsed_registers.pop_back();
        // index_holder.push_back(newIndex);
    }
  };

  template<> struct action < type > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      if(DEBUGGING) cout << "Found a type: " << in.string() << endl;

      
      if (in.string().find("code") != std::string::npos) {
        Code* code = new Code();
        type_declarations.push_back(code);
      } 
      else if (in.string().find("tuple") != std::string::npos) {
        Tuple* tuple = new Tuple();
        type_declarations.push_back(tuple);

      }
      else if (in.string().find("[") != std::string::npos){
        Array* array = new Array();
        if(DEBUGGING) cout << "This is an array \n";
        int dims = index_holder.size();
        array->dims = dims;

        type_declarations.push_back(array);
      }
      else{
        Int64* int64 = new Int64();
        type_declarations.push_back(int64);
      }
    }
  };

  template<> struct action < callee > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      
      if(DEBUGGING) cout << "returning from callee " <<  in.string() << endl;
      
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
    static void apply( const Input & in, Program & p){
      if(DEBUGGING) cout << "Found a op: " << in.string() << endl;
      
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
    static void apply( const Input & in, Program & p){
      
      if(DEBUGGING) cout << "Found a var: " <<  in.string() << endl;
      
      Arg* arg = new Arg();
      arg->name = in.string();
      parsed_registers.push_back(arg);

    }
  };

  template<> struct action < number > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      
      if(DEBUGGING) cout << "Found a number " << in.string() << endl;

      Number* num = new Number();
      num->name = in.string();
      num->num = atoi(in.string().c_str());
      parsed_registers.push_back(num);

    }
  };

  template<> struct action < T > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      if(DEBUGGING) cout << "found a Type: " <<  in.string() << endl;

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
        if(DEBUGGING) cout << "Making a new array\n";
        Array* array = new Array();

        int dims = index_holder.size();
        array->dims = dims;

        type_declarations.push_back(array);
      }
    }
  };

  template<> struct action < declaration > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
      if(DEBUGGING) cout << "found a declaration " <<  in.string() << endl;

      Function* currentF = p.functions.back();

      Type* type = type_declarations.back();
      type_declarations.pop_back();

      Arg* arg = parsed_registers.back();
      Arg* test = parsed_registers.back();
      arg->type = type;


      test = findVariable(currentF, test);
      if(test){
        newFunctionArgs.push_back(arg);
        //parsed_registers.pop_back();
      }
      else{
        currentF->declared_variables.insert(arg);
        newFunctionArgs.push_back(arg);

      }
      

    }
  };

  template<> struct action < assignment > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found an assign " <<  in.string() << endl;
        
        Function* currentF = p.functions.back();
        
        Instruction_Assignment *instruction = new Instruction_Assignment();
        if(DEBUGGING) cout << "Getting source\n";
        Arg* source = parsed_registers.back();
        parsed_registers.pop_back();
        if(DEBUGGING) cout << "Getting dest\n";

        Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        if(DEBUGGING) cout << "Mapping source\n";
        // map the variables to the declared variables 
        source = findVariable(currentF, source);
        if(DEBUGGING) cout << "Mapping dest\n";

        dest = findVariable(currentF, dest);
        if(DEBUGGING) cout << "Found dest\n";

        instruction->instruction = dest->name + " <- " + source->name;
  
        instruction->dst = dest;
        instruction->src = source;

        Operation* op = new Operation();
        op->name = "<-";
        op->op = ASSIGN;

        instruction->operation = op;

        currentF->instructions.push_back(instruction);
        if(DEBUGGING) cout << "Leaving assignment\n";
    }
  };

  template<> struct action < op_assign > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found an op_assign " <<  in.string() << endl;
        
        Function*   currentF = p.functions.back();
        
        Operation* operation = operations.back();
        operations.pop_back();

        Arg* arg2 = parsed_registers.back();
        parsed_registers.pop_back();

        Arg* arg1 = parsed_registers.back();
        parsed_registers.pop_back();

        Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        dest = findVariable(currentF, dest);
        arg1 = findVariable(currentF, arg1);
        arg2 = findVariable(currentF, arg2);

        Instruction_opAssignment* instruction = new Instruction_opAssignment();
        instruction->dst = dest;
        instruction->arg1 = arg1;
        instruction->arg2 = arg2;
        instruction->operation = operation;
        instruction->instruction = dest->name + " <- " + arg1->name + " " + operation->name + " " + arg2->name;
        
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < load > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found a load " <<  in.string() << endl;
        
        Function* currentF = p.functions.back();
        
        Instruction_Load* instruction = new Instruction_Load();

         while(1){
          Arg* idx = parsed_registers.back();
          idx = findVariable(currentF, idx);
          if(DEBUGGING) cout << "Dealing with arg: " << idx->name << "\n";
          if(Array* ar = dynamic_cast<Array*>(idx->type)){
            if(DEBUGGING) cout << "This is the array\n";
            break;
          }
          
          if(DEBUGGING) cout << "Pushing it back\n";
          instruction->indexes.push_back(idx);
          parsed_registers.pop_back();
          

        }

        if (DEBUGGING) cout << "-----> parsed_registers is\n";
        for(int j=0; j<parsed_registers.size(); j++) {
          if (DEBUGGING) cout << parsed_registers[j]->name << endl;
        }


        // //Add this to get to correct parsed regs
        // for(int k = 0; k < instruction->indexes.size(); k++){
        //   parsed_registers.pop_back();
        // }


        Arg* src = parsed_registers.back();
        if(DEBUGGING) cout << "Source of load is: " << src->name << endl;
        parsed_registers.pop_back();

        src = findVariable(currentF, src);

        Arg* dest = parsed_registers.back();
        if(DEBUGGING) cout << "dest of load is: " << dest->name << endl;

        parsed_registers.pop_back();
        dest = findVariable(currentF, dest);


        


        

        instruction->instruction = dest->name + " <- load " + src->name;
    if(DEBUGGING) cout << instruction->instruction << endl;
        instruction->dst = dest;
        instruction->src = src;

        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < store > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found a store " <<  in.string() << endl;
        
        Function*   currentF = p.functions.back();
        
        Instruction_Store *instruction = new Instruction_Store();


        Arg* src = parsed_registers.back();
        parsed_registers.pop_back();


        while(1){
          Arg* idx = parsed_registers.back();
          if (DEBUGGING) cout << "Finding the src with findVar\n";
          idx = findVariable(currentF, idx);
          if(DEBUGGING) cout << "Dealing with arg: " << idx->name << "\n";
          if(Array* ar = dynamic_cast<Array*>(idx->type)){
            if(DEBUGGING) cout << "This is the array\n";
            break;
          }
          
          if(DEBUGGING) cout << "Pushing it back\n";
          instruction->indexes.push_back(idx);
          parsed_registers.pop_back();
        }

        Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        if (DEBUGGING) cout << "Finding Variables src: " << src->name << "\n";
        src = findVariable(currentF, src);
        if (DEBUGGING) cout << "Finding Variables dst: " << dest->name << "\n";
        dest = findVariable(currentF, dest);
        if (DEBUGGING) cout << "Found dest: " << dest->name <<  "and found source: " << src->name <<"\n";

        instruction->instruction = "store " + dest->name + " <- " + src->name;
  
        instruction->dst = dest;
        instruction->src = src;
        

        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < length > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found a length instruction: " <<  in.string() << endl;
        
        Function* currentF = p.functions.back();
        
        Instruction_Length* instruction = new Instruction_Length();

        Arg* dimension = parsed_registers.back();
        parsed_registers.pop_back();

        Arg* array = parsed_registers.back();
        parsed_registers.pop_back();

        Arg* dst = parsed_registers.back();
        parsed_registers.pop_back();

        if(DEBUGGING) cout << "dimension name is: " << dimension->name << endl;
        if(DEBUGGING) cout << "array name is: " << array->name << endl;
        if(DEBUGGING) cout << "dst name is: " << dst->name << endl;

        dimension = findVariable(currentF, dimension);
        array = findVariable(currentF, array);
        dst = findVariable(currentF, dst);

        if(DEBUGGING) cout << "After matching vars --> dimension name is: " << dimension->name << endl;
        if(DEBUGGING) cout << "After matching vars --> array name is: " << array->name << endl;
        if(DEBUGGING) cout << "After matching vars --> dst name is: " << dst->name << endl;

        instruction->instruction = dst->name + " <- length " + array->name + " " + dimension->name;
  
        instruction->dimension = dimension;
        instruction->array = array;
        instruction->dst = dst;

        currentF->instructions.push_back(instruction);

        if(DEBUGGING) cout << "Pushed back the length instruction: " <<  in.string() << endl;
    }
  };

  template<> struct action < tuple > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found an tuple initialization: " <<  in.string() << endl;
        
        Function* currentF = p.functions.back();
        
        Instruction_TupleInit* instruction = new Instruction_TupleInit();

        Arg* src = parsed_registers.back();
        src = findVariable(currentF, src);
        parsed_registers.pop_back();
      
        if(DEBUGGING) cout << "Found the tuple\n";
        Arg* dest = parsed_registers.back();
        parsed_registers.pop_back();

        dest = findVariable(currentF, dest);


        // instruction->instruction = dest->name + " <- new Array(";

        // for(int i = 0; i < instruction->src.size(); i++){
        //   instruction->instruction.append(instruction->src[i]->name);
        //   if(i != instruction->src.size() - 1){
        //     instruction->instruction.append(", ");
        //   }
        // }
        // instruction->instruction.append(")");
        instruction->src.push_back(src);
        instruction->dst = dest;

        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < array > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found an array initialization: " <<  in.string() << endl;
        
        Function* currentF = p.functions.back();
        
        Instruction_ArrayInit* instruction = new Instruction_ArrayInit();

        while(1){
          Arg* src = parsed_registers.back();
          if (DEBUGGING) cout << "Finding the src with findVar\n";
          src = findVariable(currentF, src);
          if(DEBUGGING) cout << "Dealing with arg: " << src->name << "\n";
          if(Array* ar = dynamic_cast<Array*>(src->type)){
            if(DEBUGGING) cout << "Found the array\n";
            Arg* dest = parsed_registers.back();
            parsed_registers.pop_back();
            dest = findVariable(currentF, dest);
            instruction->dst = dest;
            break;
          }
          
          if(DEBUGGING) cout << "Pushing it back\n";
          instruction->src.push_back(src);
          parsed_registers.pop_back();
        }
        

      
        

        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < call > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found a call " <<  in.string() << endl;
        
        Function*   currentF = p.functions.back();
        
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

        reverse(instruction->parameters.begin(), instruction->parameters.end());

        instruction->callee = callee;

        instruction->instruction = "call " + instruction->callee->name + " ( ";

        for (auto param : instruction->parameters) {
          instruction->instruction.append(" " + param->name + ",");
        }
        instruction->instruction.append(" )");
  
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < call_assign > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found a call_assign " <<  in.string() << endl;
        
        Function*   currentF = p.functions.back();
        
        
        Instruction_CallAssign* instruction = new Instruction_CallAssign();

        Arg* parameter = parsed_registers.back();
        parsed_registers.pop_back();

        if(DEBUGGING) cout << "Attempting to cast as a Callee: " << parameter->name << "\n";
        Callee* callee = dynamic_cast<Callee *> (parameter);

        while(callee == NULL) {
          if(DEBUGGING) cout << "Is a parameter: " << parameter->name << "\n";
          parameter = findVariable(currentF, parameter);
          instruction->parameters.push_back(parameter);

          parameter = parsed_registers.back();
          parsed_registers.pop_back();
          if(DEBUGGING) cout << "Attempting to cast as a Callee: " << parameter->name << "\n";
          callee =  dynamic_cast<Callee *> (parameter);
        }

        reverse(instruction->parameters.begin(), instruction->parameters.end());
        if(DEBUGGING) cout << "Found Callee: " << callee->name << "\n";
        instruction->callee = callee;
        parsed_registers.pop_back();
        
        Arg* dest = parsed_registers.back();

        parsed_registers.pop_back();

        dest = findVariable(currentF, dest);
        if(DEBUGGING) cout << "Found destination: " << dest->name << "\n";
        instruction->dst = dest;

        instruction->instruction = instruction->dst->name + " <- call " + instruction->callee->name + " ( ";

        for (auto param : instruction->parameters) {
          instruction->instruction.append(" " + param->name + ",");
        }
        instruction->instruction.append(" )");
  
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < return_val > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found a return_val " <<  in.string() << endl;
        
        Function*   currentF = p.functions.back();
        
        Instruction_ReturnVal* instruction = new Instruction_ReturnVal();

        Arg* val = parsed_registers.back();
        parsed_registers.pop_back();
        val = findVariable(currentF, val);

        instruction->instruction = "return " + val->name;
  
        instruction->retVal = val;
        currentF->instructions.push_back(instruction);
      }
  };

  template<> struct action < return_nothing > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found a return_nothing " <<  in.string() << endl;
          
        Function*   currentF = p.functions.back();
        
        Instruction_Return* instruction = new Instruction_Return();
        instruction->instruction = "return" ;
        currentF->instructions.push_back(instruction);
    }
  };

  template<> struct action < br_single > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found a br_single " <<  in.string() << endl;
      
        Function*   currentF = p.functions.back();
        
        Instruction_br* instruction = new Instruction_br();

        Label* label = dynamic_cast<Label *> (parsed_registers.back());
        parsed_registers.pop_back();

        instruction->instruction = "br " + label->name;
        instruction->label = label;
        currentF->instructions.push_back(instruction);
        

    }
  };

  template<> struct action < br_cmp > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "found a br_cmp " <<  in.string() << endl;
        
        Function*   currentF = p.functions.back();
        
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
        currentF->instructions.push_back(instruction);
        

    }
  };


  template<> struct action < comment > {
    template< typename Input >
    static void apply( const Input & in, Program & p){
        if(DEBUGGING) cout << "Found a comment " <<  in.string() << endl;
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
    pegtl::analyze< grammar >();


    /*
     * Parse.
     */   
    file_input< > fileInput(fileName);
    Program p;
    
    parse< grammar, action >(fileInput, p);

    if(DEBUGGING | DEBUG_S) cout << "Done parsing!\n";
    return p;
  }

  Arg* findVariable(Function* f, Arg* arg) {
    
    // if it is just a number, then it wont be stored in declared vars
    if (Number* num = dynamic_cast<Number *> (arg)) { return arg; }

    if(Label* label = dynamic_cast<Label *>(arg)) { return arg; }

    // reverse so we get the latest declaration -- can you redefine variables with new types?
    // reverse(f->declared_variables.begin(), f->declared_variables.end());
    for(Arg* var : f->declared_variables) {
      if (var->name == arg->name ) { return var; }
    }
    return NULL;
  }


}// LA
