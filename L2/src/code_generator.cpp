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

namespace L2{

  void L2_generate_code(Program p) {
    std::fstream fs;
    fs.open("prog.L1", std::fstream::in | std::fstream::out | std::fstream::app);
    fs << "(" << p.entryPointLabel << "\n"; 
    for(auto f : p.functions){
        fs << "(" << f->name << "\n\t" << f->arguments << " " << f->locals << "\n";
        for(Instruction* I : f->instructions){
            fs << "\t" << I->instruction << "\n";
        }
        fs << ")\n";
    }
    fs << ")\n";

    fs.close();
  }
        
}

 
