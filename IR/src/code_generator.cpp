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


    BasicBlock* findBasicBlock(Function* f, std::string label) {

        for (BasicBlock* bb : f->basicBlocks) {
            if (bb->label == label) { return bb; }
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
                    bb->successor.push_back();
            }
            else if (iBrCmp) {
                // we have two successors
                BasicBlock* successor1 = findBasicBlock(f, iBr->trueLabel->name);
                BasicBlock* successor2 = findBasicBlock(f, iBr->falseLabel->name);
                if (successor1)
                    bb->successor.push_back();

                if (successor2)
                    bb->successor.push_back();

            }
            else if (iRet || iRetVal) {
                // we make the successor a special block to indicate we are done
                ExitBlock* exitBlock = new ExitBlock();
                bb->successor.pushBack(exitBlock);
            }
            else {

                std::cerr << "BasicBlock's te field was invalid\n";
            }
        }
    }


    void L3_generate_code(Program p) {
        
        

        // set up file stream
        std::fstream fs;
        fs.open("prog.L3", std::fstream::in | std::fstream::out | std::fstream::app);
        
        for (Function* f : p.functions) {

            orderBasicBlocks(f);
        }
        
        fs.close();
      }
        
}

 
