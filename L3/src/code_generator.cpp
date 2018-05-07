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
        return "";
    }

    std::string convert_function(Function* f) {

        std::string funcStr = "";

        funcStr.append( "(" + f->name + "\n\t" + to_string(f->arguments) + " " + to_string(f->locals) + "\n");
        
        for(Instruction* I : f->instructions){
            funcStr.append("\t" + convert_instruction(I) + "\n");
        }
        

        funcStr.append(")\n");
        return funcStr;
    }

    void L3_generate_code(Program p) {
        std::fstream fs;
        fs.open("prog.L2", std::fstream::in | std::fstream::out | std::fstream::app);
        fs << "(:main" << "\n"; 
        for(auto f : p.functions){
            fs << convert_function(f) << "\n";
            // fs << "(" << f->name << "\n\t" << f->arguments << " " << f->locals << "\n";
            // for(Instruction* I : f->instructions){
            //     fs << "\t" << I->instruction << "\n";
            // }
            // fs << ")\n";
        }
        fs << ")\n";

        fs.close();
      }
        
}

 
