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

            I->instruction = "//" + I->instruction;
        }
        else if(Instruction_Store i = dynamic_cast<Instruction_Store *> (I)){
            string storeLine = "";
            if(Tuple* t = dynamic_cast<Tuple *>(i->dst->type)){
                string uniqueVar = "uniqueVariableForTupleByDavidAndBrian";
                //calculate the offset
                storeLine.append(uniqueVar + " <- " + i->indexes[0]->name + " * 8\n");
                storeLine.append(uniqueVar + " <- " + uniqueVar + " + 8\n");
                storeLine.append( "store " + uniqueVar + " <- " + i->src->name);
                I->instruction = storeLine;
            }
            //Dealing with an array
            else{

            }
        }
        else if(Instruction_Load i = dynamic_cast<Instruction_Load *> (I)){
            string loadLine = "";
            if(Tuple* t = dynamic_cast<Tuple *>(i->src->type)){
                string uniqueVar = "uniqueVariableForTupleByDavidAndBrian";
                //calculate the offset
                loadLine.append(uniqueVar + " <- " + i->indexes[0]->name + " * 8\n");
                loadLine.append(uniqueVar + " <- " + uniqueVar + " + 8\n");
                loadLine.append(i->dst->name + " <- load " + uniqueVar);
                I->instruction = loadLine;
            }
            //Dealing with an array
            else{

            }
        }
        else if(Instruction_TupleInit i = dynamic_cast<Instruction_TupleInit *> (I)){
            i->Instruction = i->dst->name + " <- call allocate(" + i->src[0]->name + ",1)"; 
        }
        else if(Instruction_ArrayInit i = dynamic_cast<Instruction_ArrayInit *> (I)){
            vector<string> arrayInit = {};


            //This section will decode the arguments 
            vector<string> placeholdVars = {};
            for(Arg* a : i->src){
                string newName = a->name + "DavidAndBrian";
                placeholdVars.push_back(newName);
                string decodeLine = "";
                decodeLine = newName + " <- " + a->name + " >> 1";
                arrayInit.push_back(decodeLine);
            }
            //and then multiply them all together so we can get correct amount of space to allocate 
            for(int i = 1; i < placeholdVars.size(); i ++){
                string multLine = placeholdVars[0] + " <- " + placeholdVars[0] + " * " + placeholdVars[i];
                arrayInit.push_back(multLine);
            }
            //Add space for holding dimensionalities
            arrayInit.push_back(placeholdVars[0] + " <- " + placeholdVars[0] + " + " + placeholdVars.size()+1);

            //And then encode the value
            arrayInit.push_back(placeholdVars[0] + " <- " + placeholdVars[0] + " << 1");
            arrayInit.push_back(placeholdVars[0] + " <- " + placeholdVars[0] + " + 1");

            //Then we make the allocate instruction
            string allocateLine = i->dst->name + " <- call allocate(" + placeholdVars[0] + ", 1)";
            arrayInit.push_back(allocateLine);

            //Storing dimensions 
            arrayInit.push_back(placeholdVars[0] + " <- " + i->dst->name + " + 8");
            arrayInit.push_back("store " + placeholdVars[0] + " <- " + (placeholdVars.size()*2 + 1) );

            //and array lengths
            int count = 0;
            for(int i = 16; i < placeholdVars.size() * 8 + 8; i += 8){
                arrayInit.push_back(placeholdVars[0] + " <- " + i->dst->name + " + " + to_string(i));
                arrayInit.push_back("store " + placeholdVars[0] + " <- " + i->src[count]->name);
                count++;
            }

            //concatenate all the arrayInit vector strings into I->Instruction
            I->instruction = "";

            for(string str : arrayInit){
                I->instruction.append(str);
                I->instruction.append("\n");
            }

        }
        else if(Instruction_Length i = dynamic_cast<Instruction_Length *> (I)){
            string uniqueVar = "DavidAndBrianMakeALengthUnique";
            string lengthLine = "";
            lengthLine.append(uniqueVar + " <- " + i->dimension->name + " * 8\n");
            lengthLine.append(uniqueVar + " <- " + uniqueVar + " + 16\n");
            lengthLine.append(uniqueVar + " <- " + i->array->name + " + " + uniqueVar + "\n");
            lengthLine.append(i->dst->name + " <- load " + uniqueVar);

            I->instruction = lengthLine;
        }
        else{
            //This is an instruction that is already a valid L3 inst
            //Op_Assignment
            //Branches
            //Calls
        }

        //Strip away all % signs
        I->instruction.erase(remove(I->instruction.begin(), I->instruction.end(), '%'), I->instruction.end());
        return;
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

            //orderBasicBlocks(f);
            
            for (BasicBlock* B : f->basicBlocks) {
                for(Instruction* I : B->instructions){
                    convertInstruction(f, I);
                }
            }
        }
        
        fs.close();
      }
        
}

 
