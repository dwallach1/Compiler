#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdlib.h>
#define DEBUGGING 0
#define DEBUG_S 0
#include <code_generator.h>

using namespace std;

namespace L3{

    std::string generate_unique_var(Function* f) {
        std::string tmpVar = "";
        for(int i=0; i < f->uniques + 1; i++) {
            tmpVar += '?';
        }
        return tmpVar;
     }


    std::string convert_instruction(Function* f, Instruction* I) {
        

        if (Instruction_Load* i = dynamic_cast<Instruction_Load *> (I)) {

            // TODO
            return "load instruction -- not yet implemented";
        }
        else if (Instruction_Store* i = dynamic_cast<Instruction_Store *> (I)) {

            // TODO
            return "store instruction -- not yet implemented";
        }
        else if (Instruction_br* i = dynamic_cast<Instruction_br *> (I)) {

            return "goto " + i->label->name;
        }
        else if (Instruction_brCmp* i = dynamic_cast<Instruction_brCmp *> (I)) {

            return "cjump " + i->comparitor->name + " = 1 " + i->trueLabel->name + " " + i->falseLabel->name;
        }
        else if (Instruction_Call* i = dynamic_cast<Instruction_Call *> (I)) {

            // TODO
            // going to need to store parameters of the call in proper registers  
            return "call instruction -- not yet implemented";
        }
        else if (Instruction_CallAssign* i = dynamic_cast<Instruction_CallAssign *> (I)) {

            // TODO
            // going to need to store parameters of the call in proper registers  
            return "call_assign instruction -- not yet implemented";
        }
        else if (Instruction_opAssignment* i = dynamic_cast<Instruction_opAssignment *> (I)) {

            // need to split into two instructions and store into a new variable 
            std::string tmpVar = generate_unique_var(f);
            f->uniques++;

            //generate the two instructions
            std::string line1 = tmpVar + " <- " + i->arg1->name + " " + i->operation + " " + i->arg2->name + "\n";
            std::string line2 = "\t\t" + i->dst->name + " <- " + tmpVar;
            
            return line1 + line2;
        }
        else if (Instruction_cmpAssignment* i = dynamic_cast<Instruction_cmpAssignment *> (I)) {

            // check if we need to reverse the arguments
            if (i->operation == ">") {
                return i->dst->name + " " + i->arg2->name + " < " + i->arg1->name;
            } 
            else if (i->operation == ">=") {
                return i->dst->name + " " + i->arg2->name + " <= " + i->arg1->name;
            }
            else {
                return i->instruction;
            }
        }
        else if (Instruction_ReturnVal* i = dynamic_cast<Instruction_ReturnVal *> (I)) {

            // first store the return value in rax and then do the return
            return "rax <- " + i->retVal->name + "\n\t\treturn";
        }
        else {
            /*  These instructions are already valid L2 instructions 
             *   
             *    Types:
             *      (1) Instruction_Assignment
             *      (2) Instruction_Label
             *      (3) Instruction_return
             *
             */

            return I->instruction;
        }
     }

    void updateArgumentsAndLocals(Function* f) {
        // this function will update the argument and local integers necessary for all L2 functions

        // the arguments are synonymous with the parameters vector
        f->arguments = f->parameters.size();

        // the locals will be harder to determine, need to look through instruction variables used
        }

    std::string convert_function(Function* f) {

        f->uniques = 0;

        std::string funcStr = "";

        funcStr.append( "\t(" + f->name + "\n");

        updateArgumentsAndLocals(f);

        funcStr.append("\t\t" + to_string(f->arguments) + " " + to_string(f->locals) + "\n");
        
        for(Instruction* I : f->instructions){
            funcStr.append("\t\t" + convert_instruction(f, I) + "\n");
        }
        

        funcStr.append("\t)\n");
        return funcStr;
     }

    void L3_generate_code(Program p) {
        // set up file stream
        std::fstream fs;
        fs.open("prog.L2", std::fstream::in | std::fstream::out | std::fstream::app);
        
        // we know the starting point has to be main to be a valid L3 program
        fs << "(:main" << "\n"; 
        
        // convert all the fucntions
        for(auto f : p.functions){

            // this will write out an entire L2 function translated from L3
            fs << convert_function(f) << "\n";
        }

        // close the outermost container that holds the entry_point & close the filestream
        fs << ")\n";
        fs.close();
      }
        
}

 
