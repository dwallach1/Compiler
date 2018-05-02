#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <fstream>
#include <cstdio>
#include <regex>
#include <stdlib.h>
#define DEBUGGING 0
#define DEBUG_S 1


std::vector<std::string> allRegs = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi", "r12", "r13", "r14", "r15", "rbp", "rbx"};
std::vector<std::string> callInstKill = {"r10", "r11", "r8", "r9", "rax", "rcx", "rdi", "rdx", "rsi"};
std::vector<std::string> callInstGen = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
std::vector<std::string> calleeSaveRegs = {"r12", "r13", "r14", "r15", "rbx", "rbp"};

using namespace std;

namespace L2{    
std::map<L2::Color, int> colorToRegMap = {{R10, 0}, {R11, 1}, {R8, 2}, {R9, 3}, {RAX, 4}, {RCX, 5}, {RDI, 6}, {RDX, 7}, {RSI, 8}, {R12, 9}, {R13, 10}, {R14, 11}, {R15, 12}, {RBP, 13}, {RBX, 14}, {NO_COLOR, 15} };
std::map<int, L2::Color> regToColorMap = {{0, R10}, {1, R11}, {2, R8}, {3, R9}, {4, RAX}, {5, RCX}, {6, RDI}, {7, RDX}, {8, RSI}, {9, R12}, {10, R13}, {11, R14}, {12, R15}, {13, RBP}, {14, RBX}, {15, NO_COLOR} };

void printInterferenceGraph(L2::InterferenceGraph* iG);
void linkInstructionPointers(L2::Function* f);
L2::Variable* findCorrespondingVar(std::string name, L2::InterferenceGraph* iG);
void spillVar(L2::Function* f);
void generateUsesAndVars(L2::Function* f);
void insertLoad(Function* f, std::string replacementString, std::vector<Instruction*>::iterator idx, int stackLoc);
void insertStore(Function* f, std::string replacementString, std::vector<Instruction*>::iterator idx, int stackLoc);
void removeIncDecSpaces(L2::Function* f);

    /*
     *
     *
     *
     *  COLOR VARIABLES 
     *
     *
     *
     *
     *
     */

    void handleStackArgs(Function* f) {

        for (Instruction* I : f->instructions) {
            if (I->type == STACKARG) {
                int bytes = f->locals * 8;
                int a = atoi(I->arguments[2]->name.c_str());
                bytes += a;
                if (DEBUG_S) printf("a is %d\n", a);
                I->instruction = std::regex_replace(I->instruction, std::regex("stack-arg [0-9]+"), "mem rsp " + std::to_string(bytes));
            }
        }
    }


    void colorRegisters(Function* f){
        //if (DEBUG_S) printf("Coloring registers:\n");
        for(std::string curReg : allRegs){
            Variable* curVar = findCorrespondingVar(curReg, f->interferenceGraph);
            if(curVar){
                for(int i = 0; i < 15; i++){
                    if(curReg == allRegs[i]){
                        curVar->color = regToColorMap[i];
                        //if (DEBUG_S) printf("%s -> %s\n", curReg.c_str(), allRegs[i].c_str());
                    }
                }
            }
        }
        printf("\n");
    }

    void generateStack(Function* f, std::vector<Variable*>* stack){
        for(Variable* v : f->interferenceGraph->variables){
            //Not a register
            if(std::find(allRegs.begin(), allRegs.end(), v->name) == allRegs.end()){
                v->color = NO_COLOR;
                for(int i = 0; i < 15; i++){
                    v->aliveColors[i] = true;
                }
                v->aliveColors[15] = false;
                stack->push_back(v);
            }
        }
    }

    bool assignColor(Variable* v, Function* f){
        for(std::string e : v->edges){
            Variable* curVar = findCorrespondingVar(e, f->interferenceGraph);
            if(curVar){
                if (DEBUG_S) printf("Marking bit %d false for %s -> %s\n", colorToRegMap[curVar->color], v->name.c_str(), curVar->name.c_str());
                v->aliveColors[colorToRegMap[curVar->color]] = false;
            }
        }
        if(DEBUG_S){
            printf("The available colors for (%s) are: \n", v->name.c_str());
            for(int i = 0 ; i < 15; i++){
                if(v->aliveColors[i]){
                    printf("%s ", allRegs[i].c_str());
                }
            }
            printf("\n");
        }
        for(int i = 0; i < 15; i++){
            if(v->aliveColors[i]){
                v->color = regToColorMap[i];
                if(DEBUG_S) printf("Assigning color %s to var %s\n", allRegs[colorToRegMap[v->color]].c_str(), v->name.c_str());
                return true;
            }
        }
        return false;
    }

    void submitColorChanges(Function* f){
        for(Variable* V : f->interferenceGraph->variables){
            if(std::find(allRegs.begin(), allRegs.end(), V->name) == allRegs.end()){

                for(Instruction* I : V->uses){
                    I->instruction.append(" ");
                        if(DEBUG_S) printf("Changing Instruction (Replacing %s with %s): %s\n", V->name.c_str(), allRegs[colorToRegMap[V->color] ].c_str() ,I->instruction.c_str());
                        //std::size_t found = I->instruction.find(V->name);
                        std::string i = " " + I->instruction;
                        I->instruction = std::regex_replace(i, std::regex(" " + V->name + " "), " " + allRegs[colorToRegMap[V->color]] + " ");
                        if(DEBUG_S) printf("Instruction is now: %s\n", I->instruction.c_str());
                }
            }
        }
    }

    void generateUses(L2::Function* f){
        for(Instruction* I : f->instructions){
            for(int i = 0; i < I->arguments.size(); i++){
                L2::Variable* V = findCorrespondingVar(I->arguments[i]->name, f->interferenceGraph);
                //Found a variable
                if(V){
                    //varFound = true;
                    V->uses.push_back(I);
                    if(I->type == ASSIGN){ 
                        if(I->arguments[0]->name == I->arguments[1]->name){
                            break;
                        }
                    }
                }
            }
        }
    }

    bool colorVariables(Function* f){
        //Color the registers because it won't change
        colorRegisters(f);
        std::vector<Variable*> stack;
        bool done = false;
        bool assigned = false;
        std::vector<std::string> calleeSavesInUse = {};
       
         calleeSavesInUse = {};
         stack = {};
         generateStack(f, &stack);
         for(Variable* V : stack){
             assigned = assignColor(V, f);
             if(assigned){
                 //Callee Save
                 if(colorToRegMap[V->color] > colorToRegMap[RSI]){
                    if (DEBUG_S) printf("adding callee saved: %s\n", allRegs[colorToRegMap[V->color]].c_str());
                     calleeSavesInUse.push_back(allRegs[colorToRegMap[V->color] ]);
                 }
             }
             //spill
             else{
                 f->toSpill = V->name;
                 f->replaceSpill = V->name + "S_P_I_L_L";
                 if(DEBUG_S) printf("Attempting to spill %s\n", V->name.c_str());
                 spillVar(f);
                 return false;
             }
         }
        
        int offset = f->locals * 8;
        for(std::string str : calleeSavesInUse){
            if (DEBUG_S) printf("adding load and store instructions for a callee saved reg\n");
            f->locals++;
            offset += 8;
            
            std::vector<Instruction*>::iterator iter;

            iter = f->instructions.begin();
            insertStore(f, str, iter, offset);

            iter = f->instructions.end();
            insertLoad(f, str, iter, offset);

            linkInstructionPointers(f);


        }
        if(DEBUG_S) printf("Submitting Color Changes\n");
        generateUses(f);
        submitColorChanges(f);
        removeIncDecSpaces(f);
        handleStackArgs(f);
        return true;

    }

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
                //printf("from in %s\n", curIn.c_str());
                vars.insert(curIn);
            }
            // add all instruction variables from out set
            for(std::string curOut : I->out){
               // printf("from out %s\n", curOut.c_str());
                vars.insert(curOut);
            }
            // add all instruction variables from kill set
            for(std::string curKill : I->kill){
                //printf("from kill %s\n", curKill.c_str());
                vars.insert(curKill);
            } 
        }
        
        // now we have all variable names, instiate new Variable objects for them
        for(std::string curVar : vars){
            //printf("instatiating newVar: %s\n", curVar.c_str());
            L2::Variable* newVar = new L2::Variable();
            newVar->name = curVar;
            newVar->edges = {};
            iG->variables.insert(newVar); 
        }
    }

    void addToEdgeSet(Variable* V, std::vector<std::string> vec){
        for (std::string s : vec) {
            if (s == V->name){
                for (std::string curVal : vec) {
                    if (curVal != V->name) {
                        V->edges.insert(curVal);
                    }
                }
            }

        }
        
    }

    void addToEdgeSetOneWay(Variable* V, std::vector<std::string> vec){
        for (std::string s : vec) {
                for (std::string curVal : vec) {
                    if (curVal != V->name) {
                        V->edges.insert(curVal);
                    }
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
            

            //Make clique of all registers
            std::set<L2::Variable*> regVars = {};
            for (std::string r : allRegs) {
                Variable* V = findCorrespondingVar(r, f->interferenceGraph);
                if (V) { regVars.insert(V); }
            }
            makeClique(&regVars);
            

            for(Instruction* I : f->instructions){
                // addToEdgeSet checks to see if the variable is in the given vector
                // if it is, then it adds its elements to its edge set
                addToEdgeSet(V, I->in);
                addToEdgeSet(V, I->out);

       
            }
        }


        //Link the kill sets and out sets
        for(Instruction* I : f->instructions){

            // check if x <- y condition           
            if ((I->type != ASSIGN) || (I->type == ASSIGN && (I->arguments[1]->type == LBL || I->arguments[1]->type == NUM))) { 
                
                std::vector<std::string> r;            
  
                // for each variable in the kill sets, link to variables in the out sets
                for(std::string curVar : I->kill){
                    r = {};
                    L2::Variable* V = findCorrespondingVar(curVar, f->interferenceGraph);

                    for(std::string curOut : I->out){
                        r.push_back(curOut);
                    }
                    
                    addToEdgeSetOneWay(V, r);


                }     
            }
                
            //Shift
            if(I->type == AOP && (I->operation[0] == "<<=" || I->operation[0] == ">>=")){

                //If not a digit, then add all registers except for rcx to the interence graph
                if(I->arguments[1]->type != NUM) {
                    std::set<L2::Variable*> result = {};
                    L2::Variable* V = findCorrespondingVar(I->arguments[1]->name, iG);
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
            printf("genkill for %s\n", I->instruction.c_str());
            switch(I->type){
                //arithmetic
                case AOP:
                    if (DEBUG_S) printf("added a AOP: %s\n", I->instruction.c_str());  
                    if(I->arguments[1]->type != MEM && I->arguments[1]->type != NUM) {
                        I->gen.push_back(I->arguments[1]->name);
                    }
                    if(I->arguments[0]->type != NUM && I->arguments[0]->type != NUM) {
                        I->kill.push_back(I->arguments[0]->name);
                        I->gen.push_back(I->arguments[0]->name);
                    }

                    if(I->arguments[1]->type == MEM) {
                        std::istringstream iss(I->arguments[1]->name);
                        for(std::string s; iss >> s; )
                            result.push_back(s);

                        I->gen.push_back(result[1]);
                    }

                    if(I->arguments[0]->type == MEM) {
                        std::istringstream iss(I->arguments[0]->name);
                        for(std::string s; iss >> s; )
                            result.push_back(s);

                        I->gen.push_back(result[1]);
                    }

                    break;


                //assignment
                case ASSIGN:
                    
                    if(I->arguments[1]->type != LBL && I->arguments[1]->type != NUM) {
                        I->gen.push_back(I->arguments[1]->name);
                    }

                    I->kill.push_back(I->arguments[0]->name);      
                    if (DEBUG_S) printf("added a assignmentn: %s\n", I->instruction.c_str());              
                    break;



                // load
                case LOAD:
                    for(std::string s; iss >> s; )
                        result.push_back(s);

                    I->kill.push_back(I->arguments[0]->name);

                    if(result[3] != "rsp"){
                        I->gen.push_back(result[3]);
                    }

                    if (DEBUG_S) printf("added a load: %s\n", I->instruction.c_str());
                    
                    break;

               
                case STACKARG:
                    I->kill.push_back(I->arguments[0]->name);
                    if (DEBUG_S) printf("added %s to kill set (STACKARG)\n", I->kill[0].c_str());
                    break;

                //store
                case STORE:
                    if (DEBUG_S) printf("added a STORE: %s\n", I->instruction.c_str());  
                    if (I->arguments[1]->type != LBL && I->arguments[1]->type != NUM) {
                        I->gen.push_back(I->arguments[1]->name);
                    }

                    for(std::string s; iss >> s; )
                        result.push_back(s);

                    if(result[1] != "rsp"){
                        I->gen.push_back(result[1]);
                    }

                    break;

                // cjump
                case CJUMP:
                    if (DEBUG_S) printf("added a CJUMP: %s\n", I->instruction.c_str());  
                    // dest
                    if (I->arguments[3]->type != NUM) {
                        I->gen.push_back(I->arguments[3]->name);
                    }

                    // source
                    if (I->arguments[2]->type != NUM) {
                        I->gen.push_back(I->arguments[2]->name);
                    }

                    break;

                // goto    
                case GOTO:
                    if (DEBUG_S) printf("added a GOTO: %s\n", I->instruction.c_str());  

                    break;

                // return 
                case RET:
                    break;

                // call    
                case CALL:
                    if (DEBUG_S) printf("added a CALL: %s\n", I->instruction.c_str());  
                    // I->gen = callInstGen;
                
                    if (I->arguments[0]->name != "print" && 
                        I->arguments[0]->name != "allocate" && 
                        I->arguments[0]->name != "array_error" && 
                        I->arguments[0]->type != LBL) {

                        I->gen.push_back(I->arguments[0]->name);
                    }
                    //This will add the arguments to the gen set. Essentially it is a loop that will add registers in the arguments until it reaches the number in the instruction
                    for(int q = 0; q < atoi(I->arguments[1]->name.c_str()); q++){
                        if (q == 6) break;
                        I->gen.push_back(callInstGen[q]);
                    }

                    I->kill = callInstKill;
                    break;

                // lea
                case LEA:
                    if (DEBUG_S) printf("added a LEA: %s\n", I->instruction.c_str());  
                    I->kill.push_back(I->arguments[0]->name);
                    I->gen.push_back(I->arguments[1]->name);
                    I->gen.push_back(I->arguments[2]->name);
                    break;

                // compare assign
                case CMP_ASSIGN:
                    if (DEBUG_S) printf("added a CMP_ASSIGN: %s\n", I->instruction.c_str());  
                    I->kill.push_back(I->arguments[0]->name);
                    if (I->arguments[1]->type != NUM) {
                        I->gen.push_back(I->arguments[1]->name);
                    }
                    if (I->arguments[2]->type != NUM) {
                        I->gen.push_back(I->arguments[2]->name);
                    }
                    
                    break;

                // inc/dec
                case INC_DEC:
                    if (DEBUG_S) printf("added a INC_DEC: %s\n", I->instruction.c_str());  
                    I->kill.push_back(I->arguments[0]->name);
                    I->gen.push_back(I->arguments[0]->name);
                    break;


                default:
                    if (DEBUG_S) printf("going to default for %s\n", I->instruction.c_str());
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

        //this will be used to set the next outset for an instruction
        std::vector<std::string> prevINSet = {"r12", "r13", "r14", "r15", "rax", "rbp", "rbx"};
        while (changed) {
            

            changed = false;
            for (Instruction* I: f->instructions) {
               

                std::vector<std::string> outKill = {};
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

                    for(Instruction* ITemp : f->instructions){
                        //label instruction
                        if(ITemp->type == LABEL){
                            //if the label is present in the cjump/goto instruction
                            if (ITemp->arguments[0]->name.find(I->arguments[0]->name) != std::string::npos || ITemp->arguments[0]->name.find(I->arguments[1]->name) != std::string::npos){

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
