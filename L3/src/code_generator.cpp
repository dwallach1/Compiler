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

namespace L3{

    std::vector<std::string> argumentRegs = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};


    std::string generate_unique_var(Function* f) {
        std::string appendage = "U_N_I_Q_U_E";
        std::string newVar = "";

        bool unique = false;
        while (!unique) {
            unique = true;
            newVar.append(appendage);
            for (Arg* variable : f->variables) {
                if (variable->name == newVar) {
                    unique = false;
                }
            }
        }

        return newVar;
     }

    void numberInstructions(Program* p){
        for(Function* f : p->functions){
            int i = 0;
            for(Instruction* I : f->instructions){
                I->instNum = i;
                i++;
            }
        }
     }

     bool isFunctionLabel(std::string labelName, Program* p){
        for(Function* f : p->functions){
            if(f->name == labelName){
                return true;
            }
        }
        return false;
     }

     void renameAllLabels(Program* p){
        for(Function* f : p->functions){
            for(Instruction* I : f->instructions){
                std::string functName = f->name;
                functName.erase(0,1);
                if(Instruction_Assignment* i = dynamic_cast<Instruction_Assignment*>(I)){
                    if(i->src->type == LBL && !isFunctionLabel(i->src->name, p)){
                        i->src->name = i->src->name + functName;
                        i->instruction = i->dst->name + " <- " + i->src->name;
                    }
                }
                else if(Instruction_Store* i = dynamic_cast<Instruction_Store*>(I)){
                    if(i->src->type == LBL && !isFunctionLabel(i->src->name, p)){
                        i->src->name = i->src->name + functName;
                    }
                }
                else if(Instruction_brCmp* i = dynamic_cast<Instruction_brCmp*>(I)){
                    i->trueLabel->name = i->trueLabel->name + functName;
                    i->falseLabel->name = i->falseLabel->name + functName;
                }
                else if(Instruction_br* i = dynamic_cast<Instruction_br*>(I)){
                    i->label->name = i->label->name + functName;
                }
                else if(Instruction_Label* i = dynamic_cast<Instruction_Label*>(I)){
                    i->label->name = i->label->name + functName;
                    i->instruction = i->label->name;
                }

            }
        }
     }

    //This function will find all instructions that make a call to it and add it into its set of instructions
    void linkCallsToFunctions(Program* p){
        for(Function* f : p->functions){
            for(Instruction* I : f->instructions){
                if(Instruction_Call* callInst = dynamic_cast<Instruction_Call *>(I)){
                    bool found = false;
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
    void gatherAllSpecialCalls(Program* p){
        for(Function* f : p->functions){
            for(Instruction* I : f->instructions){
                if(Instruction_Call* callInst = dynamic_cast<Instruction_Call *>(I)){
                    if(callInst->callee->name[0] != ':'){
                        p->calls.insert(callInst);
                    }         
                }
            }
        }
     }


     void handleAllSpecialCalls(Program* p){
        std::vector<Instruction* >::iterator iter;
        int callNum = 0;
        if(DEBUGGING) std::cout << "Going through the special callers\n";
        for(Instruction_Call* tempI : p->calls){
            if(DEBUGGING) std::cout << "Dealing with instruction: " << tempI->instruction << "\n";
            for(int i = 0; i < tempI->parameters.size(); i++){
                if(i < 6){
                    if(DEBUGGING) std::cout << "Loading an arg into a reg.\n";
                    Instruction_Assignment* I = new Instruction_Assignment();
                    I->src = tempI->parameters[i];
                    Arg* a = new Arg();
                    a->name = argumentRegs[i];
                    a->type = VAR;
                    I->dst = a;
                    I->parentFunction = tempI->parentFunction;
                    I->instruction = I->dst->name + " <- " + I->src->name;
                    if(DEBUGGING) std::cout << "Loading arg: " << I->instruction << "\n";
                    numberInstructions(p);
                    iter = tempI->parentFunction->instructions.begin() + tempI->instNum;
                    tempI->parentFunction->instructions.insert(iter, I);
                }
                else{
                    if(DEBUGGING) std::cout << "Loading an arg into a stack location\n";
                    Instruction_stackStore* I = new Instruction_stackStore();
                    I->src = tempI->parameters[i];
                    Arg* a = new Arg();
                    a->name = "mem rsp -" + to_string((i - 6) * 8 + 16);
                    a->type = RSPMEM;
                    I->dst = a;
                    I->parentFunction = tempI->parentFunction; 
                    I->instruction = I->dst->name + " <- " + I->src->name;
                    if(DEBUGGING) std::cout << "Loading arg: " << I->instruction << "\n";
                    numberInstructions(p);
                    iter = tempI->parentFunction->instructions.begin() + tempI->instNum;
                    tempI->parentFunction->instructions.insert(iter, I);
                }
            }
            // 3
            //Create the return label store
            if(DEBUGGING) std::cout << "Storing the return label into the stack at mem rsp -8\n";
            Instruction_stackStore* retLabelStore = new Instruction_stackStore();
            Arg* retLabel = new Arg();
            retLabel->type = LBL;
            retLabel->name = tempI->parentFunction->name + tempI->parentFunction->name.substr(1) + "R_E_T_U_R_N_L_A_B_E_L" + to_string(callNum);
            retLabelStore->src = retLabel;
            Arg* a0 = new Arg();
            a0->type = RSPMEM;
            a0->name = "mem rsp -8";
            retLabelStore->dst = a0;
            retLabelStore->parentFunction = tempI->parentFunction;
            retLabelStore->instruction = retLabelStore->dst->name + " <- " + retLabelStore->src->name;
            numberInstructions(p);
            iter = tempI->parentFunction->instructions.begin() + tempI->instNum;
            tempI->parentFunction->instructions.insert(iter, retLabelStore);
            // 4
            //check if it is a return value call, if so we will load the return inst for when the call returns
            if(DEBUGGING) std::cout << "Checking for call_assign function\n";
            if(Instruction_CallAssign* callAssInst = dynamic_cast<Instruction_CallAssign*>(tempI)){
                if(DEBUGGING) std::cout << "It is a call assign\n";
                Instruction_Assignment* raxAssign = new Instruction_Assignment();
                Arg* rax = new Arg();
                rax->name = "rax";
                rax->type = VAR;
                raxAssign->src = rax;
                raxAssign->dst = callAssInst->dst;
                raxAssign->instruction = raxAssign->dst->name + " <- " + raxAssign->src->name;
                raxAssign->parentFunction = tempI->parentFunction;
                numberInstructions(p);
                iter = tempI->parentFunction->instructions.begin() + tempI->instNum;
                //if(DEBUGGING) std::cout << "Inserting instruction for rax assign above inst: " << *(iter + 1).instruction <<"\n";
                tempI->parentFunction->instructions.insert(iter + 1, raxAssign);
            }
            if(DEBUGGING) std::cout << "Adding the return label underneath the call\n";
            // 5
            //This will add the return label underneath the function call
            Instruction_Label* labelInst = new Instruction_Label();
            labelInst->label = retLabel;
            labelInst->parentFunction = tempI->parentFunction;
            labelInst->instruction = retLabel->name;
            if(DEBUGGING) std::cout << "Created the new Instruction_Label\n";
            numberInstructions(p);
            if(DEBUGGING) std::cout << "Renumbered instructions\n";
            iter = tempI->parentFunction->instructions.begin() + tempI->instNum;
            //if(DEBUGGING) std::cout << "Inserting label to return to above inst: " << *(iter + 1).instruction <<"\n";
            // if(iter >= tempI->parentFunction->instructions.end()){
            //     iter--;
            // }
            if(DEBUGGING) std::cout << "Attempting insert\n";
            tempI->parentFunction->instructions.insert(iter+1, labelInst);
            if(DEBUGGING) std::cout << "Inserted\n";
            numberInstructions(p);
            callNum++;
            if(DEBUGGING) std::cout << "Done with call inst, onto the next\n";
        }
     }


    //This function will handle creating new instructions to handle the new call instruction.
    //1) Adds the argument loading within the function.
    //2) Adds the argument setting for instructions calling it.
    //3) Writes a unique returnLabel and stores it into mem rsp -8
    //4) Writes rax to the return variable if applicable
    //5) Writes the return label immediately following the function call instruction.
    void addFunctionArgumentLoadAndStoreAndRetInst(Program* p){
        std::vector<Instruction* >::iterator iter;

        bool specialCalls = p->calls.size() > 0;

        for(Function* f : p->functions){
            if(DEBUGGING) std::cout << "AFALASARI for: " << f->name << " There are " + to_string(f->parameters.size())    + " parameters\n";
            // 1
            //This first part will add the argument loading at the start of a function
            std::reverse(f->parameters.begin(), f->parameters.end());
            for(int i = 0; i < f->parameters.size(); i++){
                //This will have the arguments stored in a register
                if(i < 6){
                    if(DEBUGGING) std::cout << "Adding parameters with regs (<6)\n";
                    Instruction_Assignment* I = new Instruction_Assignment();
                    I->dst = f->parameters[i];
                    Arg* a = new Arg();
                    a->name = argumentRegs[i];
                    a->type = VAR;
                    I->src = a;
                    I->parentFunction = f;
                    I->instruction = I->dst->name + " <- " + I->src->name;
                    iter = f->instructions.begin();
                    f->instructions.insert(iter, I);
                }
                //This will require a stack-arg instruction
                else{
                    if(DEBUGGING) std::cout << "Adding parameters with stack-arg\n";
                    Instruction_stackArg* I = new Instruction_stackArg();
                    I->dst = f->parameters[i];
                    Arg* a = new Arg();
                    a->name = "stack-arg " + to_string((f->arguments - i - 1) * 8);
                    a->type = S_ARG;
                    I->src = a;
                    I->parentFunction = f;
                    I->instruction = I->dst->name + " <- " + I->src->name;
                    iter = f->instructions.begin();
                    f->instructions.insert(iter, I);
                }
            }

            int callNum = 0;
            // 2 
            //This second part will find all of the calls to the function and add the return address, and storing arguments
            if(DEBUGGING) std::cout << "Going through the callers to the inst\n";
            for(Instruction_Call* tempI : f->callers){
                if(DEBUGGING) std::cout << "Dealing with instruction: " << tempI->instruction << "\n";
                for(int i = 0; i < f->parameters.size(); i++){
                    if(i < 6){
                        if(DEBUGGING) std::cout << "Loading an arg into a reg.\n";
                        Instruction_Assignment* I = new Instruction_Assignment();
                        I->src = tempI->parameters[i];
                        Arg* a = new Arg();
                        a->name = argumentRegs[i];
                        a->type = VAR;
                        I->dst = a;
                        I->parentFunction = tempI->parentFunction;
                        I->instruction = I->dst->name + " <- " + I->src->name;
                        if(DEBUGGING) std::cout << "Loading arg: " << I->instruction << "\n";
                        numberInstructions(p);
                        iter = tempI->parentFunction->instructions.begin() + tempI->instNum;
                        tempI->parentFunction->instructions.insert(iter, I);
                    }
                    else{
                        if(DEBUGGING) std::cout << "Loading an arg into a stack location\n";
                        Instruction_stackStore* I = new Instruction_stackStore();
                        I->src = tempI->parameters[i];
                        Arg* a = new Arg();
                        a->name = "mem rsp -" + to_string((i - 6) * 8 + 16);
                        a->type = RSPMEM;
                        I->dst = a;
                        I->parentFunction = tempI->parentFunction; 
                        I->instruction = I->dst->name + " <- " + I->src->name;
                        if(DEBUGGING) std::cout << "Loading arg: " << I->instruction << "\n";
                        numberInstructions(p);
                        iter = tempI->parentFunction->instructions.begin() + tempI->instNum;
                        tempI->parentFunction->instructions.insert(iter, I);
                    }
                }

                // 3
                //Create the return label store
                if(DEBUGGING) std::cout << "Storing the return label into the stack at mem rsp -8\n";
                Instruction_stackStore* retLabelStore = new Instruction_stackStore();
                Arg* retLabel = new Arg();
                retLabel->type = LBL;
                retLabel->name = f->name + tempI->parentFunction->name.substr(1) + "R_E_T_U_R_N_L_A_B_E_L" + to_string(callNum);
                retLabelStore->src = retLabel;
                Arg* a0 = new Arg();
                a0->type = RSPMEM;
                a0->name = "mem rsp -8";
                retLabelStore->dst = a0;
                retLabelStore->parentFunction = tempI->parentFunction;
                retLabelStore->instruction = retLabelStore->dst->name + " <- " + retLabelStore->src->name;
                numberInstructions(p);
                iter = tempI->parentFunction->instructions.begin() + tempI->instNum;
                tempI->parentFunction->instructions.insert(iter, retLabelStore);

                // 4
                //check if it is a return value call, if so we will load the return inst for when the call returns
                if(DEBUGGING) std::cout << "Checking for call_assign function\n";
                if(Instruction_CallAssign* callAssInst = dynamic_cast<Instruction_CallAssign*>(tempI)){
                    if(DEBUGGING) std::cout << "It is a call assign\n";
                    Instruction_Assignment* raxAssign = new Instruction_Assignment();
                    Arg* rax = new Arg();
                    rax->name = "rax";
                    rax->type = VAR;
                    raxAssign->src = rax;
                    raxAssign->dst = callAssInst->dst;
                    raxAssign->instruction = raxAssign->dst->name + " <- " + raxAssign->src->name;
                    raxAssign->parentFunction = tempI->parentFunction;
                    numberInstructions(p);
                    iter = tempI->parentFunction->instructions.begin() + tempI->instNum;
                    //if(DEBUGGING) std::cout << "Inserting instruction for rax assign above inst: " << *(iter + 1).instruction <<"\n";
                    tempI->parentFunction->instructions.insert(iter + 1, raxAssign);


                }

                if(DEBUGGING) std::cout << "Adding the return label underneath the call\n";
                // 5
                //This will add the return label underneath the function call
                Instruction_Label* labelInst = new Instruction_Label();
                labelInst->label = retLabel;
                labelInst->parentFunction = tempI->parentFunction;
                labelInst->instruction = retLabel->name;
                if(DEBUGGING) std::cout << "Created the new Instruction_Label\n";
                numberInstructions(p);
                if(DEBUGGING) std::cout << "Renumbered instructions\n";
                iter = tempI->parentFunction->instructions.begin() + tempI->instNum;
                //if(DEBUGGING) std::cout << "Inserting label to return to above inst: " << *(iter + 1).instruction <<"\n";
                // if(iter >= tempI->parentFunction->instructions.end()){
                //     iter--;
                // }
                if(DEBUGGING) std::cout << "Attempting insert\n";
                tempI->parentFunction->instructions.insert(iter+1, labelInst);
                if(DEBUGGING) std::cout << "Inserted\n";
                numberInstructions(p);
                callNum++;
                if(DEBUGGING) std::cout << "Done with call inst, onto the next\n";
            }
        }




        numberInstructions(p);
    }

    std::string convert_instruction(Function* f, Instruction* I) {
        

        if (Instruction_Load* i = dynamic_cast<Instruction_Load *> (I)) {
            if(Instruction_stackArg* ii = dynamic_cast<Instruction_stackArg*>(i)){
                return ii->instruction;
            }

            return i->dst->name + " <- " + "mem " + i->src->name + " 0";
        }
        else if (Instruction_Store* i = dynamic_cast<Instruction_Store *> (I)) {

            if(Instruction_stackStore* ii = dynamic_cast<Instruction_stackStore*>(i)){
                return ii->instruction;
            }

            return "mem " + i->dst->name + " 0 <- " + i->src->name;
        }
        else if (Instruction_br* i = dynamic_cast<Instruction_br *> (I)) {

            return "goto " + i->label->name;
        }
        else if (Instruction_brCmp* i = dynamic_cast<Instruction_brCmp *> (I)) {

            return "cjump " + i->comparitor->name + " = 1 " + i->trueLabel->name + " " + i->falseLabel->name;
        }
        else if (Instruction_CallAssign* i = dynamic_cast<Instruction_CallAssign *> (I)) {
            if(i->callee->type == PAA){
                if(DEBUGGING) std::cout << "In a call assign for: " << i->instruction << "\n";
                if(i->callee->name == "print"){
                    return "rdi <- " + i->parameters[0]->name + "\n\t\tcall print 1\n\t\t" + i->dst->name + " <- rax"; 
                }
                else if(i->callee->name == "allocate"){
                    return "rdi <- " + i->parameters[0]->name + "\n\t\trsi <- " + i->parameters[1]->name + "\n\t\tcall allocate 2\n\t\t" + i->dst->name + " <- rax"; 
                }
                else{
                    return "rdi <- " + i->parameters[0]->name + "\n\t\trsi <- " + i->parameters[1]->name + "\n\t\tcall array-error 2\n\t\t" + i->dst->name + " <- rax";
                }
            }
            // going to need to store parameters of the call in proper registers  
            return "call " + i->callee->name + " " + to_string(i->parameters.size());
        }
        else if (Instruction_Call* i = dynamic_cast<Instruction_Call *> (I)) {
            if(i->callee->type == PAA){
                if(i->callee->name == "print"){
                    return "rdi <- " + i->parameters[0]->name + "\n\t\tcall print 1"; 
                }
                else if(i->callee->name == "allocate"){
                    return "rdi <- " + i->parameters[0]->name + "\n\t\trsi <- " + i->parameters[1]->name + "\n\t\tcall allocate 2"; 
                }
                else{
                    return "rdi <- " + i->parameters[0]->name + "\n\t\trsi <- " + i->parameters[1]->name + "\n\t\tcall array-error 2";
                }
            }
            
            // going to need to store parameters of the call in proper registers  
            return "call " + i->callee->name + " " + to_string(i->parameters.size());
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
        else if (Instruction_opAssignment* i = dynamic_cast<Instruction_opAssignment *> (I)) {

            

            //This instruction will have 5 cases occur
            //1) Var <- # op #
            if(i->arg1->type == NUM && i->arg2->type == NUM){
                int a0, a1, a2;
                if(Number* a10 = dynamic_cast<Number*>(i->arg1)){
                    a1 = a10->num;
                }
                if(Number* a10 = dynamic_cast<Number*>(i->arg2)){
                    a2 = a10->num;
                }
                switch(i->operation->op){
                    case ADD:
                        a0 = a1 + a2; 
                        break;
                    case SUB:
                        a0 = a1 - a2;
                        break;
                    case MUL:
                        a0 = a1 * a2; 
                        break;
                    case AND:
                        a0 = a1 & a2;
                        break;
                    case SHL:
                        a0 = a1 << a2;
                        break;
                    case SHR:
                        a0 = a1 >> a2;
                        break;
                    case LT:
                        a0 = a1 < a2;
                        break;
                    case LTE:
                        a0 = a1 <= a2;
                        break;
                    case EQ:
                        a0 = a1 = a2;
                        break;
                    case GTE:
                        a0 = a1 >= a2;
                        break;
                    case GT:
                        a0 = a1 > a2;
                        break;
                    default: 
                        break;
                }
                return i->dst->name + " <- " + to_string(a0);

            }

            if(i->arg1->type == NUM){
                std::string newVarName = i->parentFunction->name.substr(1);
                newVarName = i->arg2->name + newVarName + "T_E_M_P_O_R_A_R_Y";
                return newVarName + " <- " + i->arg1->name + "\n\t\t" + newVarName + " " + i->operation->str + "= " + i->arg2->name + "\n\t\t" + i->dst->name + " <- " + newVarName;
            }

            //2) Var <- Var op Var1 (Var op Var and Var op # is equivalent)
            if(i->dst->name == i->arg1->name){
                switch(i->operation->op){
                    case ADD:
                        return i->dst->name + " += " + i->arg2->name; 
                        break;
                    case SUB:
                        return i->dst->name + " -= " + i->arg2->name; 
                        break;
                    case MUL:
                        return i->dst->name + " *= " + i->arg2->name; 
                        break;
                    case AND:
                        return i->dst->name + " &= " + i->arg2->name; 
                        break;
                    case SHL:
                        return i->dst->name + " <<= " + i->arg2->name; 
                        break;
                    case SHR:
                        return i->dst->name + " >>= " + i->arg2->name; 
                        break;
                    default: 
                        break;
                }
            
            }
            //5) Var <- Var1 op Var
            if(i->dst->name == i->arg2->name){
               switch(i->operation->op){
                    case ADD:
                        return i->dst->name + " += " + i->arg1->name; 
                        break;
                    case MUL:
                        return i->dst->name + " *= " + i->arg1->name; 
                        break;
                    case AND:
                        return i->dst->name + " &= " + i->arg1->name; 
                        break;
                    default: 
                        std::string newVarName = i->parentFunction->name.substr(1);
                        newVarName = i->arg1->name + newVarName + "T_E_M_P_O_R_A_R_Y";
                        return newVarName + " <- " + i->arg1->name + "\n\t\t" + newVarName + " " + i->operation->str + "= " + i->arg2->name + "\n\t\t" + i->dst->name + " <- " + newVarName;
                        break;
                } 
            }
            std::string newVarName = i->parentFunction->name.substr(1);
            newVarName = i->arg1->name + newVarName + "T_E_M_P_O_R_A_R_Y";
            return newVarName + " <- " + i->arg1->name + "\n\t\t" + newVarName + " " + i->operation->str + "= " + i->arg2->name + "\n\t\t" + i->dst->name + " <- " + newVarName;

            


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

    void generateContextBlocks(Function* f) {
        
        //ContextBlock* contextBlock = new ContextBlock();

        // for (Instruction* I : f->instructions) {
        //     if (Instruction_Label* i = dynamic_cast<Instruction_Label *> (I) ||
        //         Instruction_br* i = dynamic_cast<Instruction_br *> (I) ||
        //         Instruction_brCmp* i = dynamic_cast<Instruction_brCmp *> (I)) { 

        //         f->contextBlocks.push_back(contextBlock);
        //         contextBlock = new ContextBlock();
        //      }
        //     else {
        //         contextBlock->instructions.push_back(I);
        //     }
        // }
     }

    void generateTrees(ContextBlock* cb, std::vector<Tree *>* trees) {
        
        // for (Instruction* I : cb->instructions) {
        //     Tree* tree = new Tree();
            
        //     if (Instruction_Assignment* i = dynamic_cast<Instruction_Assignment *> (I)) {
        //         Node* child1 = new Node();
        //         left = i->src;

        //         Node* child2 = new Node();
        //         right = i->dst;

        //         Node* head = new Node();
        //         head = i->operation;
        //         head->children.push_back(child1);
        //         head->children.push_back(child2);

        //         tree->head = head;
        //     }

        //     // go through all possible instructions that can be in a code block and make a tree
        //     // 


        //     // each iteration a tree is made, insert it into the trees vector
        //     trees->push_back(tree);
        // }
    }


    std::string convert_function(Function* f) {

        std::string funcStr = "";

        funcStr.append( "\t(" + f->name + "\n");

        updateArgumentsAndLocals(f);
        funcStr.append("\t\t" + to_string(f->arguments) + " " + to_string(f->locals) + "\n");

        for(Instruction* I : f->instructions){
            funcStr.append("\t\t" + convert_instruction(f, I) + "\n");
        }

        //generateContextBlocks(f);
        // for (ContextBlock* cb : f->contextBlocks) {
        //     std::vector<Tree *> trees = {};
        //     generateTrees(cb, &trees);
        //     //mergeTrees(&trees);
        //     // for (Tree* tree : trees) {
        //     //     tileTree(tree);
        //     // }

        // }
        
        funcStr.append("\t)\n");
        return funcStr;
     }

    void L3_generate_code(Program p) {
        // set up file stream
        std::fstream fs;
        fs.open("prog.L2", std::fstream::in | std::fstream::out | std::fstream::app);
        
        // we know the starting point has to be main to be a valid L3 program
        fs << "(:main" << "\n"; 
        if(DEBUGGING) std::cout << "Running add Function Argument ld store and ret\n";
        addFunctionArgumentLoadAndStoreAndRetInst(&p);
        handleAllSpecialCalls(&p);
        // convert all the fucntions
        for(auto f : p.functions){

            // this will write out an entire L2 function translated from L3
            if(DEBUGGING) std::cout << "Converting function: " << f->name << "\n";
            fs << convert_function(f) << "\n";
        }

        // close the outermost container that holds the entry_point & close the filestream
        fs << ")\n";
        fs.close();
      }
        
}

 
