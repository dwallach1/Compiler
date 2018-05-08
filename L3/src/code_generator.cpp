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


    std::string convert_instruction(Instruction* I) {
        
        if (dynamic_cast<Instruction_Load *> (I)) {

            // TODO
            return "";
        }
        else if (dynamic_cast<Instruction_Store *> (I)) {

            // TODO
            return "";
        }
        else if (dynamic_cast<Instruction_br *> (I)) {

            return "goto " + I->label->name;
        }
        else if (dynamic_cast<Instruction_brCmp *> (I)) {

            return "cjump " + I->comparitor->name + " = 1 " + I->trueLabel + " " + I->falseLabel;
        }
        else if (dynamic_cast<Instruction_Call *> (I)) {

            // TODO
            return "";
        }
        else if (dynamic_cast<Instruction_CallAssign *> (I)) {

            // TODO
            return "";
        }
        else if (dynamic_cast<Instruction_ReturnVal *> (I)) {

            // TODO
            return "";
        }
        else {
            /*  These instructions are already valid L2 instructions 
             *   
             *    Types:
             *      (1) Instruction_Assignment
             *      (2) Instruction_cmpAssignment
             *      (3) Instruction_opAssignment
             *      (4) Instruction_Label
             *      (5) Instruction_return
             *
             */

            return I->instruction;
        }
    }

    void updateArgumentsAndLocals(Function* f) {

    }


    std::string convert_function(Function* f) {

        std::string funcStr = "";

        funcStr.append( "\t(" + f->name + "\n");

        updateArgumentsAndLocals(f);

        funcStr.append("\t\t" + to_string(f->arguments) + " " + to_string(f->locals) + "\n");
        
        for(Instruction* I : f->instructions){
            funcStr.append("\t\t" + convert_instruction(I) + "\n");
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

 
