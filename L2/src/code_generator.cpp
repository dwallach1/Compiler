#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdlib.h>
#define DEBUGGING 0
#define DEBUG_S 0
#include <code_generator.h>
#include <L2.h>

using namespace std;

namespace L2{

  void L2_generate_code(Program p, std::vector<std::vector<Instruction*>> paas) {
    std::fstream fs;
    fs.open("prog.L1", std::fstream::in | std::fstream::out | std::fstream::app);
    fs << "(" << p.entryPointLabel << "\n"; 
    for(auto f : p.functions){
        int i = 0;
        fs << "(" << f->name << "\n\t" << f->arguments << " " << f->locals << "\n";
        for(Instruction* I : f->instructions){
            
            if(I->type == CALL){
                //printf("Found call inst: %s\n", I->instruction.c_str());
                if(I->paa == 1337){
                    fs << "\t" << f->paaInsts[i] << "\n";
                    i++;
                }
                else{
                    fs << "\t" << I->instruction << "\n";
                }
            }
            else{
                fs << "\t" << I->instruction << "\n";
            }
        }
        fs << ")\n";
    }
    fs << ")\n";

    fs.close();
  }
        
}

 
