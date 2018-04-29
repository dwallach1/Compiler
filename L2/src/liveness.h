#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdlib.h>
#define DEBUGGING 0
#define DEBUG_S 0


std::vector<std::string> allRegs = {"r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "rax", "rbx", "rbp", "rcx", "rdi", "rdx", "rsi"};
std::vector<std::string> callInstKill = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi"};
std::vector<std::string> callInstGen = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
std::vector<std::string> calleeSaveRegs = {"r12", "r13", "r14", "r15", "rbx", "rbp"};


using namespace std;

namespace L2{    

    /*
     *
     *
     *
     *  GENERATE INTERFERENCE GRAPH 
     *
     *
     *
     *
     *
     */
    
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
            
            // add all instruction variables from in set
            for(std::string curIn : I->in){
                vars.insert(curIn);
            }
            // add all instruction variables from out set
            for(std::string curOut : I->out){
                vars.insert(curOut);
            }
            // add all instruction variables from kill set
            for(std::string curKill : I->kill){
                vars.insert(curKill);
            } 
        }
        
        // now we have all variable names, instiate new Variable objects for them
        for(std::string curVar : vars){
            L2::Variable* newVar = new L2::Variable();
            newVar->name = curVar;
            newVar->edges = {};
            iG->variables.insert(newVar); 
        }
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


    void getCallInstructionIntersection(int instNum, L2::Function* f, std::set<L2::Variable*>* result, int numArgs){
        std::set<L2::Variable*> beforeSet = {};
        std::set<L2::Variable*> afterSet = {};
        *result = {};

        
        for(int i = 0; i < f->instructions.size(); i++){
            // build the before set
            if(i < instNum){
                // find corresponding variable and add it into the before set
                for(int j = 0; j < f->instructions[i]->registers.size(); j++){
                    Variable* V = findCorrespondingVar(f->instructions[i]->registers[j], f->interferenceGraph);
                    if(V != NULL){
                       beforeSet.insert(V); 
                    } 
                }
                // if theres another call instruction before instNum, we restart b/c we were out of scope
                if(f->instructions[i]->type == CALL){
                    beforeSet = {};
                }
            }
            // build the after set
            else if(i > instNum){
                // if theres another call instruction after instNum, break out --> we're done
                if(f->instructions[i]->type == CALL){
                    break;
                }
                // otherwise find the corresponding variable and insert into the after set
                for(int j = 0; j < f->instructions[i]->registers.size(); j++){
                    Variable* V = findCorrespondingVar(f->instructions[i]->registers[j], f->interferenceGraph);
                    if(V != NULL){
                       afterSet.insert(V); 
                    }
                }
            }
        }
        
        // find the intersection of the before and after call instruction sets    
        set_intersection(beforeSet.begin(), beforeSet.end(), afterSet.begin(), afterSet.end(), std::inserter(*result, result->begin()));
       
    
        std::vector<std::string> regsToAdd = {};
        // depending on the intersection of the before and after set, we build the set that we want to make a clique 
        if (result->size() == 0) {
            regsToAdd = calleeSaveRegs;
            for(L2::Variable* V : beforeSet){
                bool found = 0;
                for(std::string curStr : allRegs){
                    if(curStr == V->name){
                        found = true;
                    }
                }
                // only add it to the callee saved registers if the var itself is not a register
                if(!found){
                    regsToAdd.push_back(V->name);
                }
            }
        }
        // otherwise, we need to add all registers
        else { regsToAdd = allRegs; }
        
        // remove argument registers for each one used in a call 
        for(int i = 0; i < numArgs; i++){
            std::vector<std::string>::iterator position = std::find(regsToAdd.begin(), regsToAdd.end(), callInstGen[i]);
            if(position != regsToAdd.end()){
                regsToAdd.erase(position);
            }
        }
        // remove callee-saved registers for each one used in function's local variables
        for(int i= 0; i < f->locals; i++){
            std::vector<std::string>::iterator position = std::find(regsToAdd.begin(), regsToAdd.end(), calleeSaveRegs[i]);
            if(position != regsToAdd.end()){
                regsToAdd.erase(position);
            }
        }
        // insert all necessary variables and registers to make a clique
        for(std::string r : regsToAdd){
            Variable* V = findCorrespondingVar(r, f->interferenceGraph);
            if(V != NULL){
                result->insert(V);
            } 
        } 
    }

    void makeClique(std::set<L2::Variable*>* variables) {
        
        for (L2::Variable* V0 : *variables) { 
            for (L2::Variable* V1 : *variables) {
                if (V0 != V1)
                    V0->edges.insert(V1->name);
            }
        }
    
    }

    void generateInterferenceGraph(L2::Function* f){
   
        L2::InterferenceGraph* iG = new L2::InterferenceGraph();
        f->interferenceGraph = iG;
      
        instatiateVariables(f, iG);

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


        //Link the kill sets and out sets
        int instNum = 0;
        for(Instruction* I : f->instructions){

            // check if x <- y condition           
            if((I->type != ASSIGN) || (I->type == ASSIGN && (I->registers[1][0] == ':' || std::isdigit(I->registers[1][0])))){ 
                
                std::set<L2::Variable*> result = {};
                
                // for each variable in the kill sets, link to variables in the out sets
                for(std::string curVar : I->kill){
                    //Grab the correpsonding variable
                    L2::Variable* V = findCorrespondingVar(curVar, iG);
                    if (V){
                        result.insert(V);
                    }
                }
                for(std::string curOut : I->out){
                    L2::Variable* V = findCorrespondingVar(curOut, iG);
                    if (V){
                        result.insert(V);
                    }
                }
                makeClique(&result);
            }     
            
            //Call 
            if(I->type == CALL){
                std::set<L2::Variable*> result = {};
                int numArgs = atoi(I->registers[1].c_str());
                getCallInstructionIntersection(instNum, f, &result, numArgs);
                makeClique(&result);        
            }
    
            //Shift
            if(I->type == AOP && (I->operation[0] == "<<=" || I->operation[0] == ">>=")){

                //If not a digit, then add all registers except for rcx to the interence graph
                if(!(std::isdigit(I->registers[1][0]))){
                    std::set<L2::Variable*> result = {};
                    L2::Variable* V = findCorrespondingVar(I->registers[1], iG);
                    result.insert(V);
                    for(std::string s : allRegs){
                        if(s != "rcx"){
                           L2:Variable* V1 = findCorrespondingVar(s, iG);
                            result.insert(V1); 
                        }
                        
                    }
                    makeClique(&result); 
                }
            }
            instNum++;
        }        
    }

    /*
     *
     *
     *
     *
     *
     *
     *      GENERATE IN AND OUT SETS FOR LIVENESS ANALYSIS
     *
     *
     *
     *
     *
     *
     *
     *
     *
     *
     */


    void linkInstructionPointers(L2::Function* f){
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
                f->instructions[i]->nextInst = f->instructions[i+1];
                f->instructions[i]->prevInst = NULL;
            }
            else if(i != (size -1)){
                //set previous instruction to be above it
                f->instructions[i]->prevInst = f->instructions[i-1];
                f->instructions[i]->nextInst = f->instructions[i+1];

            }
            //Last instruction doesn't have a next inst
            else{
                f->instructions[i]->prevInst = f->instructions[i-1];
                f->instructions[i]->nextInst = NULL;
            }
        }
    }


    void computeGenKillSet(L2::Function* f) {
        //Iterate through each instruction and generate the instructions gen and kill sets
        for(Instruction* I : f->instructions) {
            std::istringstream iss(I->instruction);
            std::vector<std::string> result;

            switch(I->type){
                //arithmetic
                case AOP:

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
                case ASSIGN:
                    if(I->registers[1][0] != ':' && !(std::isdigit(I->registers[1][0]))) {
                        I->gen.push_back(I->registers[1] );
                        if(DEBUGGING) printf("I->reg[1] = %s is going to gen\n", I->registers[1].c_str());
                    }
                    I->kill.push_back(I->registers[0]);
                    if(DEBUGGING) printf("I->Reg[0] = %s is goign to kill\n", I->registers[0].c_str());
                    
                    break;


                // load
                case LOAD:
                    for(std::string s; iss >> s; )
                        result.push_back(s);

                    I->kill.push_back(I->registers[0]);

                    if(result[3] != "rsp"){
                        I->gen.push_back(result[3]);
                    }
                    
                    break;


                //store
                case STORE:
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
                case CJUMP:

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
                case GOTO:

                    break;

                // return 
                case RET:
                    //I->gen.push_back("rsp");
                    break;

                // call    
                case CALL:

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
                case LEA:

                    I->kill.push_back(I->registers[0]);
                    I->gen.push_back(I->registers[1]);
                    I->gen.push_back(I->registers[2]);
                    break;

                // compare assign
                case CMP_ASSIGN:

                    I->kill.push_back(I->registers[0]);
                    if (!std::isdigit(I->registers[1][0])) {
                        I->gen.push_back(I->registers[1]);
                    }
                    if (!std::isdigit(I->registers[2][0])) {
                        I->gen.push_back(I->registers[2]);
                    }
                    
                    break;

                // inc/dec
                case INC_DEC:

                    I->kill.push_back(I->registers[0]);
                    I->gen.push_back(I->registers[0]);
                    break;


                default:
                    break;
            }
        }
    }


    std::string buildDFResult(L2::Function* f) {

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

        return inGlobal;

    }


    L2::DataFlowResult* computeLivenessAnalysis(L2::Program* p, L2::Function* f) { 

        linkInstructionPointers(f);
        computeGenKillSet(f);
        

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
                

                //The outset is going to be a little tricky,
                //it is normal if the instruction is anything but a goto or cjump instruction
                //because we just look at the instruction below it
                //Special insts will have to iterate through the instructions to see what it calls
                //and then Union their IN sets, won't be hard to do, but may be fun to explain
                std::vector<std::string> newOut = {};

                //if it is a special cjump or goto instruction, we need to do some shifty stuff
                if(I->type == CJUMP || I->type == GOTO){
                    if(DEBUGGING) printf("Found a cjump or goto instruction, now finding its labels\nThe inst: %s\nThe label(s): %s\n%s\n", I->instruction.c_str(), I->registers[0].c_str(), I->registers[1].c_str());
                    for(Instruction* ITemp : f->instructions){
                        //label instruction
                        if(ITemp->type == LABEL){
                            //if the label is present in the cjump/goto instruction
                            if(DEBUGGING) printf("Found a label inst: %s\n", ITemp->instruction.c_str());
                            if (ITemp->registers[0].find(I->registers[0]) != std::string::npos || ITemp->registers[0].find(I->registers[1]) != std::string::npos){
                                if(DEBUGGING) printf("Found one of its labels: %s\n", ITemp->registers[0].c_str());
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

        std::string result = buildDFResult(f);

        DataFlowResult* newDF = new L2::DataFlowResult();
        newDF->result = result;
        return newDF;

    }
}
