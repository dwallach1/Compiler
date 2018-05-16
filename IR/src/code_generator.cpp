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
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdlib.h>
#include <code_generator.h>
#define DEBUGGING 0
#define DEBUG_S 0

using namespace std;

namespace IR {

    void convertInstruction(Function* f, Instruction* I) {

        if(Instruction_Declaration* i = dynamic_cast<Instruction_Declaration *> (I)) {

            f->declared_variables.insert(i->var);
        }

    }

    BasicBlock* findBasicBlock(Function* f, std::string label) {

        for (BasicBlock* bb : f->basicBlocks) {
            if (bb->label->name == label) { return bb; }
        }
        return NULL;
    }

    void orderBasicBlocks(Function* f) {

        for (BasicBlock* bb : f->basicBlocks) {

            Instruction_br* iBr = dynamic_cast<Instruction_br *> (bb->te);
            Instruction_brCmp* iBrCmp = dynamic_cast<Instruction_brCmp *> (bb->te);
            Instruction_Return* iRet = dynamic_cast<Instruction_Return *> ( bb->te);
            Instruction_ReturnVal* iRetVal = dynamic_cast<Instruction_ReturnVal *> (bb->te);

            if (iBr) {
                // we only have one successor
                BasicBlock* successor = findBasicBlock(f, iBr->label->name);
                if (successor)
                    bb->successors.push_back(successor);
            }
            else if (iBrCmp) {
                // we have two successors
                BasicBlock* successor1 = findBasicBlock(f, iBrCmp->trueLabel->name);
                BasicBlock* successor2 = findBasicBlock(f, iBrCmp->falseLabel->name);
                if (successor1)
                    bb->successors.push_back(successor1);

                if (successor2)
                    bb->successors.push_back(successor2);

            }
            else if (iRet || iRetVal) {
                // we make the successor a special block to indicate we are done
                ExitBlock* exitBlock = new ExitBlock();
                bb->successors.push_back(exitBlock);
            }
            else {

                std::cerr << "BasicBlock's te field was invalid\n";
            }
        }
    }

    void IR_generate_code(Program p) {
        
        // set up file stream
        std::fstream fs;
        fs.open("prog.L3", std::fstream::in | std::fstream::out | std::fstream::app);
        
        for (Function* f : p.functions) {

            orderBasicBlocks(f);
            
            for (Instruction* I : f->instructions) {

                convertInstruction(f, I);
            }
        }
        
        fs.close();
      }
        
}

 
