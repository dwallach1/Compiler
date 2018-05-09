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

    std::vector<std::string> argumentRegs = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};


    std::string generate_unique_var(Function* f) {
        std::string tmpVar = "";
        for(int i=0; i < f->uniques + 1; i++) {
            tmpVar += '?';
        }
        return tmpVar;
     }

    void numberInstructions(Program* p){
        for(Function* f : p->functions){
            int i = 0;
            for(Instruction* I : f->instructions){
                I->instNum = i;
            }
        }
    }

    //This function will find all instructions that make a call to it and add it into its set of instructions
    void linkCallsToFunctions(Program* p){
        for(Function* f : p->functions){
            for(Instruction I* : f->instructions){
                if(Instruction_Call* callInst = dynamic_cast<Instruction_Call *>(I)){
                    for(Function* f0 : p->functions){
                        if(f0->name == callInst->callee->name){
                            f0->callers.insert(callInst);
                        }
                    }
                }
            }
        }
    }

    //This function will add all the call instructions found in a program into a set. May be worthless
    void gatherAllCalls(Program* p){
        for(Function* f : p->functions){
            for(Instruction* I : f->instructions){
                if(Instruction_Call* callInst = dynamic_cast<Instruction_Call *>(I)){
                    p->calls.insert(callInst);
                }
            }
        }
    }


    //This function will handle creating new instructions to handle the new call instruction.
    //1) Adds the argument loading within the function.
    //2) Adds the argument setting for instructions calling it.
    //3) Writes a unique returnLabel and stores it into mem rsp -8
    //4) Writes rax to the return variable if applicable
    //5) Writes the return label immediately following the function call instruction.
    void addFunctionArgumentLoadAndStoreAndRetInst(Program* p){
        std::vector<Instruction* >:iterator iter;
        for(Function* f : p->functions){
            // 1
            //This first part will add the argument loading at the start of a function
            for(int i = 0; i < f->arguments; i++){
                //This will have the arguments stored in a register
                if(i < 6){
                    Instruction_Assignment* I = new Instruction_Assignment();
                    I->dst = f->parameters[i];
                    Arg* a = new Arg();
                    a->name = argumentRegs[i];
                    a->type = VAR;
                    I->src = a;
                    I->parentFunction = f;
                    I->instruction = I->dst->name + " <- " + I->src->name;
                    iter = f->instructions.begin();
                    f->instructions->insert(iter, I);
                }
                //This will require a stack-arg instruction
                else{
                    Instruction_stackArg* I = new Instruction_stackArg();
                    I->dst = f->parameters[i];
                    Arg* a = new Arg();
                    a->name = "stack-arg " + itoa((f->arguments - i - 1) * 8);
                    a->type = S_ARG;
                    I->src = a;
                    I->parentFunction = f;
                    I->instruction = I->dst->name + " <- " + I->src->name;
                    iter = f->instructions.begin();
                    f->instructions->insert(iter, I);
                }
            }

            int callNum = 0;
            // 2 
            //This second part will find all of the calls to the function and add the return address, and storing arguments
            for(Instruction_Call* tempI : f->callers){
                for(int i = 0; i < f->arguments; i++){
                    if(i < 6){
                        Instruction_Assignment* I = new Instruction_Assignment();
                        I->src = tempI->parameters[i];
                        Arg* a = new Arg();
                        a->name = argumentRegs[i];
                        a->type = VAR;
                        I->dst = a;
                        I->parentFunction = tempI->parentFunction;
                        I->instruction = I->dst->name + " <- " + I->src->name;
                        iter = std::find(tempI->parentFunction->instructions.begin(), tempI->parentFunction->instructions.end(), tempI);
                        tempI->parentFunction->instructions->insert(iter, I);
                    }
                    else{
                        Instruction_stackStore* I = new Instruction_stackStore();
                        I->src = I->parameters[i];
                        Arg* a = new Arg();
                        a->name = "mem rsp -" + atoi((i - 6) * 8 + 16);
                        a->type = RSPMEM;
                        I->dst = a;
                        I->parentFunction = tempI->parentFunction; 
                        I->instruction = I->dst->name + " <- " + I->src->name;
                        iter = std::find(tempI->parentFunction->instructions.begin(), tempI->parentFunction->instructions.end(), tempI);
                        tempI->parentFunction->instructions->insert(iter, I);
                    }
                }

                // 3
                //Create the return label store
                Instruction_stackStore* retLabelStore = new Instruction_stackStore();
                Arg* retLabel = new Arg();
                retLabel->type = LBL;
                retLabel->name = f->name + "R_E_T_U_R_N_L_A_B_E_L" + atoi(callNum);
                retLabelStore->src = retLabel;
                Arg* a0 = new Arg();
                a0->type = RSPMEM;
                a0->name = "mem rsp -8";
                retLabelStore->dst = a0;
                retLabelStore->parentFunction = tempI->parentFunction;
                retLabelStore->instruction = retLabelStore->dst->name + " <- " + retLabelStore->src->name;
                iter = std::find(tempI->parentFunction->instructions.begin(), tempI->parentFunction->instructions.end(), tempI);
                tempI->parentFunction->instructions->insert(iter, retLabelStore);

                // 4
                //check if it is a return value call, if so we will load the return inst for when the call returns
                if(Instruction_CallAssign* callAssInst = dynamic_cast<Instruction_CallAssign>(tempI)){
                    Instruction_Assignment* raxAssign = new Instruction_Assignment();
                    Arg* rax = new Arg();
                    rax->name = "rax";
                    rax->type = VAR;
                    raxAssign->src = rax;
                    raxAssign->dst = callAssInst->dst;
                    raxAssign->instruction = raxAssign->dst->name + " <- " + raxAssign->src->name;
                    raxAssign->parentFunction = tempI->parentFunction;
                    iter = std::find(tempI->parentFunction->instructions.begin(), tempI->parentFunction->instructions.end(), tempI);
                    tempI->parentFunction->instructions->insert(iter + 1, raxAssign);

                }

                // 5
                //This will add the return label underneath the function call
                Instruction_Label* labelInst = new Instruction_Label();
                labelInst->label = retLabel;
                labelInst->parentFunction = tempI->parentFunction;
                labelInst->instruction = retLabel->name;
                iter = std::find(tempI->parentFunction->instructions.begin(), tempI->parentFunction->instructions.end(), tempI);
                f->instructions->insert(iter + 1, labelInst);

                callNum++;
            }
        }



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
            std::string line1 = tmpVar + " <- " + i->arg1->name + " " + i->operation->str + " " + i->arg2->name + "\n";
            std::string line2 = "\t\t" + i->dst->name + " <- " + tmpVar;
            
            return line1 + line2;
        }
        else if (Instruction_cmpAssignment* i = dynamic_cast<Instruction_cmpAssignment *> (I)) {

            // check if we need to reverse the arguments
            if (i->operation->op == GT) {
                i->operation->op = LT;
                return i->dst->name + " " + i->arg2->name + " < " + i->arg1->name;
            } 
            else if (i->operation->op == GTE) {
                i->operation->op = LTE;
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

 
