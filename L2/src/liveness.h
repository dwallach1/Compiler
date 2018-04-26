#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdlib.h>
#define DEBUGGING 0
#define DEBUG_S 1
//#include <L2.h>
//#include <code_generator.h>
std::vector<std::string> allRegs = {"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "rax", "rbx", "rbp", "rcx", "rdi", "rdx", "rsi"};
std::vector<std::string> callInstKill = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi"};
std::vector<std::string> callInstGen = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
std::vector<std::string> calleeSaveRegs = {"r12", "r13", "r14", "r15", "rbx", "rbp"};

//std::vector<std::string> allRegsWoRCX = {"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "rax", "rbx", "rbp", "rdi", "rdx", "rsi"};


using namespace std;

namespace L2{    

    L2::Variable* findCorrespondingVar(std::string name, L2::InterferenceGraph* iG){
        for(Variable* var : iG->variables){
            if(name == var->name){
                return var;
            }
        }

        return NULL;
    }

    
    void printInterferenceGraph(L2::InterferenceGraph* iG){
        for(L2::Variable* V : iG->variables){
            printf("%s", V->name.c_str());
            for(std::string E : V->edges){
                printf(" %s", E.c_str());
            }
            printf("\n");
        }
    }
    

    void instatiateVariables(L2::Function* f, L2::InterferenceGraph* iG){
        std::set<std::string> vars = {};

        //add all regs to the variable list
        std::copy(allRegs.begin(), allRegs.end(), std::inserter(vars, vars.end()));

        //Loop to add any new variables to the set of variables for interference graph
        for(Instruction* I : f->instructions){
            //In set
            if(DEBUG_S) printf("Instruction %s\nPrinting IN Set:\n", I->instruction.c_str());
            for(std::string curIn : I->in){
                vars.insert(curIn);
                if(DEBUG_S){
                    printf("%s ", curIn.c_str());
                }
            }
            if(DEBUG_S) printf("\nPrinting OUT set:\n");
            for(std::string curOut : I->out){
                vars.insert(curOut);
                if(DEBUG_S){
                    printf("%s ", curOut.c_str());
                }
            }
            if(DEBUG_S) printf("\n");

            if(DEBUG_S) printf("\nPrinting KILL set:\n");
            for(std::string curKill : I->kill){
                vars.insert(curKill);
                if(DEBUG_S){
                    printf("%s ", curKill.c_str());
                }
            }
            if(DEBUG_S) printf("\n");
        }
        

        for(std::string curVar : vars){
            L2::Variable* newVar = new L2::Variable();
            newVar->name = curVar;
            newVar->edges = {};
            iG->variables.insert(newVar);
            if(DEBUG_S) printf("Added new variable: %s\n", curVar.c_str());
        }
    }
    bool isVar(std::string V, L2::Function* f){
        if(DEBUG_S) printf("inside isVar\n");
        if(std::isdigit(V[0])){
            return false;
        }

        for(L2::Variable* VCheck : f->interferenceGraph->variables){
            if(V == VCheck->name){
                return true;
            }
        }
        return false;
    }

    void addToEdgeSet(Variable* V, std::vector<std::string> vec){
        if (std::find(vec.begin(), vec.end(), V->name) != vec.end()){
            for (std::string curVal : vec) {
                if (curVal != V->name) {
                    V->edges.insert(curVal);
                }
            }
        }
    }

    void getCallInstructionIntersection(int instNum, L2::Function* f, std::set<L2::Variable*>* result){
        std::set<L2::Variable*> beforeSet = {};
        std::set<L2::Variable*> afterSet = {};

        
        for(int i = 0; i < f->instructions.size(); i++){
            if(i < instNum){
                for(int j = 0; j < f->instructions[i]->registers.size(); j++){
                    Variable* V = findCorrespondingVar(f->instructions[i]->registers[j], f->interferenceGraph);
                    if(V != NULL){
                       beforeSet.insert(V); 
                    }
                    
                }
            }
            else if(i > instNum){
                for(int j = 0; j < f->instructions[i]->registers.size(); j++){
                    Variable* V = findCorrespondingVar(f->instructions[i]->registers[j], f->interferenceGraph);
                    if(V != NULL){
                       afterSet.insert(V); 
                    }
                }
            }
        }
        
        // //return beforeSet;
        set_intersection(beforeSet.begin(), beforeSet.end(), afterSet.begin(), afterSet.end(), std::inserter(*result, result->begin()));
        if(DEBUG_S){
            printf("The beforeSet is:\n");
            for(L2::Variable* V : beforeSet){
                printf("%s ", V->name.c_str());
            }
            printf("\n");
        }
        if(DEBUG_S){
            printf("The intersection is (size is %ld):\n", result->size());
            for(L2::Variable* curVar : *result){
                printf("%s ", curVar->name.c_str());
            }
            printf("\n");
        }
        std::vector<std::string> regsToAdd;
        if(result->size() != 0){
            regsToAdd = allRegs;
        }
        else{
            regsToAdd = calleeSaveRegs;
            for(L2::Variable* V : beforeSet){
                regsToAdd.push_back(V->name);
                if(DEBUG_S) printf("Pushing back %s into regsToAdd\n", V->name.c_str());
            }
        }
        for(std::string r : regsToAdd){
            Variable* V = findCorrespondingVar(r, f->interferenceGraph);
            if(V != NULL){
                if(DEBUG_S) printf("Inserting %s into result\n", V->name.c_str());
                result->insert(V);
            } 
            else{
                if(DEBUG_S) printf("Could not correpsonding var %s\n", r.c_str());
            } 
        }
        return;


    }

    void generateInterferenceGraph(L2::Function* f){
        if(DEBUG_S) printf("Beginning generateInterferenceGraph\n");
        L2::InterferenceGraph* iG = new L2::InterferenceGraph();
        f->interferenceGraph = iG;
        //iG->variables = {};
        if(DEBUG_S) printf("instatiateVariables Time\n");
        instatiateVariables(f, iG);

        if(DEBUG_S) printf("Linking all registers and IN and OUT sets\n");
        for(L2::Variable* V : iG->variables){ 
            std::string curVar = V->name;
            
            //Add all registers to Edge set if it is a reg
            addToEdgeSet(V, allRegs);
            
            for(Instruction* I : f->instructions){
                // addToEdgeSet checks to see if the variable is in the given vector
                // if it is, then it adds its elements to its edge set
                addToEdgeSet(V, I->in);
                addToEdgeSet(V, I->out);       
            }
        }

        if(DEBUG_S) printf("Linking KILL and OUT sets\n");
        //Link the kill sets and out sets
        int instNum = 0;
        for(Instruction* I : f->instructions){

            //if(I->type != 1 || (I->type == 1 && std::isdigit(I->registers[1][0]))){
            if(I->type != 1  ){ 
            // || ( I->type == 1 && !isVar(I->registers[1], f) )
                // for each variable in the kill sets, link to variables in the out sets
                for(std::string curVar : I->kill){
                    //Grab the correpsonding variable
                    L2::Variable* V = findCorrespondingVar(curVar, iG);
                    addToEdgeSet(V, I->out);
                }
            }
            else{
                if(DEBUG_S) printf("Assignment registers[1]: %s\n", I->registers[1].c_str());
                if(I->registers[1][0] == ':' || std::isdigit(I->registers[1][0])){
                    if(DEBUG_S) printf("NOT A VAR!!!\n");
                    for(std::string curVar : I->kill){
                        //Grab the correpsonding variable
                        L2::Variable* V = findCorrespondingVar(curVar, f->interferenceGraph);
                        if(V != NULL){
                            if(DEBUG_S) printf("Adding kill set to edgeset!!!\n");
                            addToEdgeSet(V, I->out);
                        }
                    }
                }
            }
            
            //Call
            if(I->type == 8){
                std::set<L2::Variable*> result = {};
                getCallInstructionIntersection(instNum, f, &result);
                if(DEBUG_S){
                    printf("The call intersection is:\n");
                    for(Variable* V : result){
                        printf("%s ", V->name.c_str());
                    }
                    printf("\n");
                }



                for(Variable* V : result){
                    for(Variable* V1 : result){
                        if(V != V1){
                            if(DEBUG_S) printf("adding the results to %s\n", V->name.c_str());
                            std::vector<std::string> edgesVector;
                            edgesVector.assign(V1->edges.begin(), V1->edges.end());
                            addToEdgeSet(V, edgesVector);
                        }
                    }
                }
                if(DEBUG_S){
                    for(Variable* V : result){
                        printf("%s edges:\n", V->name.c_str());
                        for(std::string str : V->edges){
                            printf("%s ", str.c_str());
                        }
                        printf("\n");
                    }
                }
            }

            //Time to see if we are a shift
            if(I->type == 0 && (I->operation[0] == "<<=" || I->operation[0] == ">>=")){

                //If not a digit, then add all registers except for rcx to the interence graph
                if(!(std::isdigit(I->registers[1][0]))){
                    L2::Variable* V = findCorrespondingVar(I->registers[1], iG);
                    addToEdgeSet(V, allRegs);
                    // need to make sure rcx isn't in edge set
                    V->edges.erase("rcx");
                }
            }
            instNum++;
        }

        printInterferenceGraph(iG);
        
    }

    void generatePrevInstPointers(L2::Function* f){
        int size = f->instructions.size();
        if(size == 1){
            f->instructions[0]->prevInst = NULL;
            f->instructions[0]->nextInst = NULL;
            return;
        }
        for (int i = 0; i < size; ++i)
        {   
            //first inst can't have a prev inst.
            if(i == 0){
                //if(DEBUGGING) printf("First instruction is: %s", f->instructions[i]->instruction.c_str());
                f->instructions[i]->nextInst = f->instructions[i+1];
                f->instructions[i]->prevInst = NULL;
            }
            else if(i != (size -1)){
                //set previous instruction to be above it
                //if(DEBUGGING) printf("Next instruction is: %s", f->instructions[i]->instruction.c_str());
                f->instructions[i]->prevInst = f->instructions[i-1];
                f->instructions[i]->nextInst = f->instructions[i+1];
                //if(DEBUGGING) printf("The next instruction should be: %s", f->instructions[i+1]->instruction.c_str());

            }
            //Last instruction doesn't have a next inst
            else{
                //if(DEBUGGING) printf("Last instruction is: %s", f->instructions[i]->instruction.c_str());
                f->instructions[i]->prevInst = f->instructions[i-1];
                f->instructions[i]->nextInst = NULL;
            }
        }
    }

    L2::DataFlowResult* computeLivenessAnalysis(L2::Program* p, L2::Function* f) {
        generatePrevInstPointers(f);

        //add all regs to the variable list
        //std::copy(allRegs.begin(), allRegs.end(), std::inserter(f->vars, f->vars.end()));
        


        
        //Iterate through each instruction and generate the instructions gen and kill sets
        for(Instruction* I : f->instructions) {
            std::istringstream iss(I->instruction);
            std::vector<std::string> result;

            switch(I->type){
                //arithmetic
                case 0:

                    if(I->registers[1].substr(0, 4) != "mem " && !(std::isdigit(I->registers[1][0]))) {
                        I->gen.push_back(I->registers[1]);
                    }
                    if(I->registers[0].substr(0, 4) != "mem " && !(std::isdigit(I->registers[0][0]))) {
                        I->kill.push_back(I->registers[0]);
                        I->gen.push_back(I->registers[0]);
                    }

                    if(I->registers[1].substr(0, 4) == "mem ") {
                        std::istringstream iss(I->registers[1]);
                        for(std::string s; iss >> s; )
                            result.push_back(s);

                        I->gen.push_back(result[1]);
                    }

                    if(I->registers[0].substr(0, 4) == "mem ") {
                        std::istringstream iss(I->registers[0]);
                        for(std::string s; iss >> s; )
                            result.push_back(s);

                        I->gen.push_back(result[1]);
                    }

                    break;


                //assignment
                case 1:
                    if(I->registers[1][0] != ':' && !(std::isdigit(I->registers[1][0]))) {
                        I->gen.push_back(I->registers[1] );
                        if(DEBUGGING) printf("I->reg[1] = %s is going to gen\n", I->registers[1].c_str());
                    }
                    I->kill.push_back(I->registers[0]);
                    if(DEBUGGING) printf("I->Reg[0] = %s is goign to kill\n", I->registers[0].c_str());
                    
                    break;


                // load
                case 2:
                    for(std::string s; iss >> s; )
                        result.push_back(s);

                    I->kill.push_back(I->registers[0]);

                    if(result[3] != "rsp"){
                        I->gen.push_back(result[3]);
                    }
                    
                    break;


                //store
                case 3:
                    if (I->registers[1][0] != ':' && !(std::isdigit(I->registers[1][0]))) {
                        I->gen.push_back(I->registers[1]);
                    }


                    for(std::string s; iss >> s; )
                        result.push_back(s);

                    if(result[1] != "rsp"){
                        I->gen.push_back(result[1]);
                    }

                    break;

                // cjump
                case 5:

                    // dest
                    if (!(std::isdigit(I->registers[3][0]))) {
                        I->gen.push_back(I->registers[3]);
                    }

                    // source
                    if (!(std::isdigit(I->registers[2][0]))) {
                        I->gen.push_back(I->registers[2]);
                    }

                    break;

                // goto    
                case 6:

                    break;

                // return 
                case 7:
                    //I->gen.push_back("rsp");
                    break;

                // call    
                case 8:

                    //I->gen.push_back("rsp");

                    if (I->registers[0] != "print" && 
                        I->registers[0] != "allocate" && 
                        I->registers[0] != "array_error" && I->registers[0][0] != ':') {
                        if(DEBUGGING) printf("Pushing the value found in a call inst: %s\n", I->registers[0].c_str());
                        I->gen.push_back(I->registers[0]);
                    }
                    //This will add the arguments to the gen set. Essentially it is a loop that will add registers in the arguments until it reaches the number in the instruction
                    for(int q = 0; q < atoi(I->registers[1].c_str()); q++){
                        I->gen.push_back(callInstGen[q]);
                    }

                    I->kill = callInstKill;
                    break;

                // lea
                case 9:

                    I->kill.push_back(I->registers[0]);
                    I->gen.push_back(I->registers[1]);
                    I->gen.push_back(I->registers[2]);
                    break;

                // compare assign
                case 10:

                    I->kill.push_back(I->registers[0]);
                    if (!std::isdigit(I->registers[1][0])) {
                        I->gen.push_back(I->registers[1]);
                    }
                    if (!std::isdigit(I->registers[2][0])) {
                        I->gen.push_back(I->registers[2]);
                    }
                    
                    break;

                // inc/dec
                case 12:

                    I->kill.push_back(I->registers[0]);
                    I->gen.push_back(I->registers[0]);
                    break;


                default:
                    break;
            }
        }

        bool changed = true;
        int debugIters = 1;
        //this will be used to set the next outset for an instruction
        std::vector<std::string> prevINSet = {"r12", "r13", "r14", "r15", "rax", "rbp", "rbx"};
        while (changed) {
            //This will determine if we are dealing with the very first instruction in order to correctly make the IN set {}
            int firstInst = 1;
            if(DEBUGGING){
                printf("Running Iteration %d\n", debugIters);
                debugIters++;
            }
            changed = false;
            for (Instruction* I: f->instructions) {
                if(DEBUGGING) printf("\n-------NEW INST--------\n%s\n", I->instruction.c_str());
                //Declare the vectortors that will be used for intermediate steps in IN computation
                //outKill is the  result of OUT[i] - KILL[i]
                std::vector<std::string> outKill = {};
                //genUoutKill is the Union of outKill and the GEN set. Begins by taking the current gen set
                std::vector<std::string> genUoutKill = I->gen;
                if(DEBUGGING){
                    printf("The out set is:\n");
                    for(std::string val : I->out){
                        printf("%s ", val.c_str());
                    }
                    printf("\n");
                    printf("The kill set is:\n");
                    for(std::string val : I->kill){
                        printf("%s ", val.c_str());
                    }
                    printf("\n");
                }
                //This will look at the out set and kill set and only add entrys to the outKill that are unique to the OUT set
                for (std::string o : I->out) {
                    bool match = false;
                    for (std::string k : I->kill) {
                        if (o == k) {
                            match = true;
                        }
                    }
                    if (!match) {
                        outKill.push_back(o);
                    }
                }
                if(DEBUGGING){
                    printf("The out set - kill set is:\n");
                    for(std::string val : outKill){
                        printf("%s ", val.c_str());
                    }
                    printf("\n");
                }

                //This will take the union of the outkill and genUoutKill set. The loop is so there are no duplicates, but isn't 100% necessary I suppose
                for (std::string oK : outKill) {
                    bool found = false;
                    for (std::string g : genUoutKill) {
                        //Is the entry in outKill currently in the gen set
                        if (oK == g) {
                            found = true;
                        }
                    }
                    //If it isn't in the current gen set then let us add it to the gen set.
                    if (!found) {
                        genUoutKill.push_back(oK);
                    }
                }

                //Now we will make a comparison of the newly generated set of genUoutKill to the current IN set, if they match then we won't set changed, otherwise we will.
                for(std::string curVal : genUoutKill){
                    bool found = false;
                    for(std::string compVal : I->in){
                        if(curVal == compVal){
                            found = true;
                        }
                    }
                    //There is a new entry, in should never really become smaller over time per each unique instruction. 
                    //This means the new IN set is going to be different. 
                    if(!found){
                        changed = true;
                        //Add the new variable to the IN set
                        I->in.push_back(curVal);
                    }
                }
                if(DEBUGGING){
                    printf("The gen set is:\n");
                    for(std::string val : I->gen){
                        printf("%s ", val.c_str());
                    }
                    printf("\n");
                    printf("The gen set unioned with the outKill set (aka the IN set) is:\n");
                    for(std::string val : genUoutKill){
                        printf("%s ", val.c_str());
                    }
                    printf("\n");
                }

                //The outset is going to be a little tricky,
                //it is normal if the instruction is anything but a goto or cjump instruction
                //because we just look at the instruction below it
                //Special insts will have to iterate through the instructions to see what it calls
                //and then Union their IN sets, won't be hard to do, but may be fun to explain
                std::vector<std::string> newOut = {};

                //if it is a special cjump or goto instruction, we need to do some shifty stuff
                if(I->type == 5 || I->type == 6){
                    if(DEBUG_S) printf("Found a cjump or goto instruction, now finding its labels\nThe inst: %s\nThe label(s): %s\n%s\n", I->instruction.c_str(), I->registers[0].c_str(), I->registers[1].c_str());
                    for(Instruction* ITemp : f->instructions){
                        //label instruction
                        if(ITemp->type == 11){
                            //if the label is present in the cjump/goto instruction
                            if(DEBUG_S) printf("Found a label inst: %s\n", ITemp->instruction.c_str());
                            if (ITemp->registers[0].find(I->registers[0]) != std::string::npos || ITemp->registers[0].find(I->registers[1]) != std::string::npos){
                                if(DEBUG_S) printf("Found one of its labels: %s\n", ITemp->registers[0].c_str());
                                for(std::string curVal : ITemp->in){
                                    bool found = false;
                                    for(std::string compVal : newOut){
                                        if(curVal == compVal){
                                            found = true;
                                        }
                                    }
                                    if(!found){
                                        //Add the new variable to the newOut set
                                        newOut.push_back(curVal);
                                    }
                                }
                            }  
                        }
                    }
                    // if(I->prevInst != NULL){
                    //   for(std::string curVal : prevINSet){
                    //         bool found = false;
                    //         for(std::string compVal : newOut){
                    //             if(curVal == compVal){
                    //                 found = true;
                    //             }
                    //         }
                    //         if(!found){
                    //             //Add the new variable to the newOut set
                    //             newOut.push_back(curVal);
                    //         }
                    //     }  
                    // }
                    //Is the first instruction
                    // else{
                    //     for(std::string curVal : I->prevInst->in){
                    //         bool found = false;
                    //         for(std::string compVal : newOut){
                    //             if(curVal == compVal){
                    //                 found = true;
                    //             }
                    //         }
                    //         if(!found){
                    //             //Add the new variable to the newOut set
                    //             newOut.push_back(curVal);
                    //         }
                    //     }
                    // }
                }
                //Otherwise we just need to take the sucessor IN set and pretend everything is ok. Also for correctness
                else{
                    if(I->nextInst != NULL){
                        newOut = I->nextInst->in;                        
                    }
                    else{
                        newOut = prevINSet;
                    }
                }

                

                //Attempting to strap the kill set into the out set

                // for(std::string curVal : I->kill){
                //     bool found = false;
                //     for(std::string compVal : newOut){
                //         if(curVal == compVal){
                //             found = true;
                //         }
                //     }
                //     if(!found){
                //         //Add the new variable to the IN set
                //         newOut.push_back(curVal);
                //     }
                // }

                if(DEBUGGING){
                    printf("The new OUT set is:\n");
                    for(std::string val : newOut){
                        printf("%s ", val.c_str());
                    }
                    printf("\n");
                }

                for(std::string curVal : newOut){
                    bool found = false;
                    for(std::string compVal : I->out){
                        if(curVal == compVal){
                            found = true;
                        }
                    }
                    if(!found){
                        changed = true;
                        //Add the new variable to the IN set
                        I->out.push_back(curVal);
                    }
                }
            }   
        }

        //Loop to add any new variables to the set of variables for interference graph
        //for(Instruction* I : f->instructions){
            //In set
          //  for(std::string curIn : I->in){
            //    f->vars.insert(curIn);
           // }
        //}

        //if(DEBUGGING){
          //  printf("The vars in this function are:\n");
            //std::set<std::string>::iterator iter;
            //for (iter = f->vars.begin(); iter != f->vars.end(); iter++){
            //for(std::string iter : f->vars){   
            //    printf("%s ", iter.c_str());
            //}
           // printf("\n");
       // }

        //Time to print to the string
        std::string inGlobal;
        std::string outGlobal;

        inGlobal.append("(\n(in\n");
        outGlobal.append("(out\n");

        for(Instruction* I : f->instructions){

            //Time to handle the return staement
            if(I->nextInst == NULL){
               I->out = {};
            }

            inGlobal.append("(");
            outGlobal.append("(");
            //In set first
            int inLen = 0;
            int outLen = 0;
            for(std::string cur : I->in){
                inGlobal.append(cur);
                inGlobal.append(" ");
                inLen++;
            }
            for(std::string cur : I->out){
                outGlobal.append(cur);
                outGlobal.append(" ");
                outLen++;
            }

            if(inLen){ 
                inGlobal.replace(inGlobal.length()-1, 1, ")");
            }
            else{
                inGlobal.append(")");
            }

            if(outLen){ 
                outGlobal.replace(outGlobal.length()-1, 1, ")");
            }
            else{
                outGlobal.append(")");
            }
            inGlobal.append("\n");
            outGlobal.append("\n");
        }
        inGlobal.append(")\n\n");
        outGlobal.append(")\n\n)");
        inGlobal.append(outGlobal);

        DataFlowResult* newDF = new L2::DataFlowResult();
        newDF->result = inGlobal;
        //if(DEBUGGING) printf("%s\n", newDF->result.c_str());
        return newDF;

    }
}
