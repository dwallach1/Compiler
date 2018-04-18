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

    L2::DataFlowResult computeLivenessAnalysis(L2::Program p, L2::Function f) {
        //Iterate through each instruction and generate the instructions gen and kill sets
        for(Instruction* I : f.instructions) {
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
                        std::vector<std::string> result;

                        std::istringstream iss(I->registers[1]);
                        for(std::string s; iss >> s; )
                            result.push_back(s);

                        I->gen.push_back(result[1])
                    }

                    if(I->registers[0].substr(0, 4) == "mem ") {
                        std::vector<std::string> result;

                        std::istringstream iss(I->registers[0]);
                        for(std::string s; iss >> s; )
                            result.push_back(s);

                        I->gen.push_back(result[1])
                    }

                    break;


                //assignment
                case 1:
                    if(I->registers[1][0] != ':') {
                        I->gen.push_back(I->registers[1]);
                    }
                    I->kill.push_back(I->registers[0]);
                    break;


                // load
                case 2:
                    std::vector<std::string> result;

                    std::istringstream iss(I->instruction[1]);
                    for(std::string s; iss >> s; )
                        result.push_back(s);

                    I->kill.push_back(I->registers[0]);
                    I->gen.push_back(result[1]);
                    break;


                //store
                case 3:
                    if (I->registers[1][0] != ':' && !(std::isdigit(I->registers[1][0])) {
                        I->gen.push_back(I->registers[1]);
                    }

                    std::vector<std::string> result;

                    std::istringstream iss(I->instruction[1]);
                    for(std::string s; iss >> s; )
                        result.push_back(s);

                    I->gen.push_back(result[1]);

                    break;

                // cjump
                case 5:

                    // dest
                    if (!(std::isdigit(I->registers[3][0])) {
                        I->gen.push_back(I->registers[3]);
                    }

                    // source
                    if (!(std::isdigit(I->registers[2][0])) {
                        I->gen.push_back(I->registers[2]);
                    }

                    break;

                // goto    
                case 6:

                    break;

                // return 
                case 7:
                    I->gen.push_back("rsp");
                    break;

                // call    
                case 8:

                    I->gen.push_back("rsp");

                    if (I->registers[0] != "print" && 
                        I->registers[0] != "allocate" && 
                        I->registers[0] != "array_error" && I->registers[0][0] != ':') {
                        I->gen.push_back(I->registers[0])
                    }

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

        int changed = 1;
        //this will be used to set the next outset for an instruction
        std::vec<std::string> prevINSet = {};
        while (changed) {
            //This will determine if we are dealing with the very first instruction in order to correctly make the IN set {}
            int firstInst = 1;
            changed = 0;
            for (Instruction* I: f.instructions) {
                //Declare the vectors that will be used for intermediate steps in IN computation
                //outKill is the  result of OUT[i] - KILL[i]
                std::vec<std::string> outKill = {};
                //genUoutKill is the Union of outKill and the GEN set. Begins by taking the current gen set
                std::vec<std::string> genUoutKill = I->gen;

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
                //it is normal if the instruction is anything but a label instruction
                //because we just look at the instruction above it
                //Label insts will have to iterate through the instructions to see what calls it
                //and then Union their IN sets, won't be hard to do, but may be fun to explain


            }   
        }
    }


    std::string clean_label(std::string label) {
        if (label.at(0) == char(':')) {
            label[0] = '_';
        }
        return label;
    }

    std::string register_map(std::string r) {
        if (r == "%r10")
            return "%r10b";
        if (r == "%r11") 
            return "%r11b";
        if (r ==  "%r12")
            return "%r12b";
        if (r == "%r13")
            return "%r13b";
        if (r ==  "%r14")
            return "%r14b";
        if (r == "%r15")
            return "%r15b";
        if (r == "%r8")
            return "%r8b";
        if (r == "%r9")
            return "%r9b";
        if (r == "%rax")
            return "%al";
        if (r == "%rbp")
            return "%bpl";
        if (r == "%rbx")
            return "%bl";
        if ( r == "%rcx")
            return "%cl";
        if ( r == "%rdi")
            return "%dil";
        if (r == "%rdx")
            return "%dl";
        if (r == "%rsi")
            return "%sil";
        return "";
    }
  


  void generate_code(Program p) {

    if(DEBUGGING) std::cout << "Beginning Code Generation" << std::endl;

    FILE *outputFile;
    outputFile = fopen("prog.S", "w");

    if(DEBUGGING) std::cout << "Opened prog.S" << std::endl;
    p.entryPointLabel = clean_label(p.entryPointLabel);
    if(DEBUGGING) std::cout << "Retrieved entryPointLabel" << std::endl;

    // Hard coded begining 
    fprintf(outputFile, ".text\n\t.globl go\ngo:\n\tpushq %%rbx\n\tpushq %%rbp\n\tpushq %%r12\n\tpushq %%r13\n\tpushq %%r14\n\tpushq %%r15\n\tcall %s\n\tpopq %%r15\n\tpopq %%r14\n\tpopq %%r13\n\tpopq %%r12\n\tpopq %%rbp\n\tpopq %%rbx\n\tretq\n", p.entryPointLabel.c_str());
    
    if(DEBUGGING) std::cout << "Beginning to iterate through functions" << std::endl;

    for (Function* F: p.functions) {
        F->name = clean_label(F->name);

        if(DEBUGGING) std::cout << "Working on function " << F->name << std::endl;

        fprintf(outputFile, "%s:\n", F->name.c_str());
     
        if(F->locals > 0){
            if(DEBUGGING) printf("There are locals in this function so I am adding some stack space for it\n");
            fprintf(outputFile, "\tsubq $%ld, %%rsp\n", F->locals*8);
        }
    
        if(DEBUGGING) std::cout << "Beginning to iterate through the instructions" << std::endl;

        for (Instruction* I: F->instructions) {
            int tempPos = 0;
            std:string space = " ";
            if(DEBUGGING) std::cout << "Working on instruction: " << I->instruction << std::endl;

           
            // split instruction by words
            std::vector<std::string> result;

            std::istringstream iss(I->instruction);
            for(std::string s; iss >> s; )
                result.push_back(s);

            // initalize variables 
            std::string operation;
            std::string src;
            std::string dst;

            std::string arg1;
            std::string arg2;
            std::string labeL2;
	        std::string label2;
	        std::string extra_instruction;
            std::string inst;
            std::string operand;

            long offset;
            int idx;
            int r;


            switch (I->type) {

                //arithmetic
                case 0:
                   
                    // register/number to register
                    if (result.size() == 3) {

                        
                        if (result[1] == "+=") {
                            operation = "addq";
                        }
                        else if (result[1] == "-=") {
                            operation = "subq";
                        }
                        else if (result[1] == "*=") {
                            operation = "imulq";
                        }
                        else if (result[1] == "&=") {
                            operation = "andq";
                        }
                        else if (result[1] == ">>=") {
                            operation = "sarq";
                        }
                        else if (result[1] == "<<=") {
                            operation = "salq";
                        }

                        // determine if number of register
                        if (result[2][0] == 'r') {
                            src = '%' + result[2];
                        } else {
                            src = '$' + result[2];
                        }

                        dst = '%' + result[0];

                    }

                    else if (result.size() == 5) {

                        // 2 types of instructions: memory as source or as dest
                        idx = result[0] == "mem" ? 3 : 1;

                        if(DEBUGGING && idx == 3) printf("Found a memory arith operation where mem is source\n");
                        if(DEBUGGING && idx == 1) printf("Found a memory arith operation where mem is dest\n");


                        if (result[idx] == "+=") {
                            operation = "addq";
                        }
                        else if (result[idx] == "-=") {
                            operation = "subq";
                        }
                        else if (result[idx] == "*=") {
                            operation = "imulq";
                        }
                        else if (result[idx] == "&=") {
                            operation = "andq";
                        }
                        else if (result[idx] == ">>=") {
                            operation = "sarq";
                        }
                        else if (result[idx] == "<<=") {
                            operation = "salq";
                        }

                        // memory is destination
                        if (idx == 3) {
                            // check if source is number/register
                            if (result[4][0] == 'r') {
                                src = '%' + result[4];
                            } else {
                                src = '$' + result[4];
                            }
                            // build destination string
                            dst = result[2] + '(' + '%' + result[1] + ')'; 
                        }
                        else {
                            // memory is the source (second arg in instruction)
                            src = result[4] + '(' + '%' + result[3] + ')'; 

                            // check if destination is number/register
                            if (result[0][0] == 'r') {
                                dst = '%' + result[0];
                            } else {
                                dst = '$' + result[0];
                            }
                        }
                    } 
                    // special case to allocate small registers for shifting
                    // printf("Originally: %s\n", src.c_str());
                    if ((operation == "salq" || operation == "sarq") && (src[1] == 'r')) {
                        src = register_map(src);
                        //printf("Now: %s\n", src.c_str());
                    } 
                    // write instruction using predefined variables
                    fprintf(outputFile, "\t%s %s, %s\n", operation.c_str(), src.c_str(), dst.c_str());

                    break;

                // store (run assignment code)
                case 3:
                // load (run assignment code)
                case 2: 
                // assignment
                case 1:
                    // constant for all assignments 
                    operation = "movq";

                    if (result.size() == 3) {

                        // determine if number of register
                        if (result[2][0] == 'r') {
                            src = '%' + result[2];
                        } 
                        else if (result[2][0] == ':'){
                            result[2] = clean_label(result[2]);
                            src = '$' + result[2];
                        }
                        else {
                            src = '$' + result[2];
                        }

                        // always a register in this case
                        dst = '%' + result[0];
                    }
                    // we have size of 5 
                    else if (result.size() == 5) {
                        if (result[0] == "mem") {
                            // determine if number of register
                            if (result[4][0] == 'r') {
                                src = '%' + result[4];
                            } 
                            else if (result[4][0] == ':'){
                                result[4] = clean_label(result[4]);
                                src = '$' + result[4];
                            }
                            else {
                                src = '$' + result[4];
                            }

                            // always a register in this case
                            dst = result[2] + '(' + '%' + result[1] + ')';


                        } else {
                            // determine if number of register
                            src = result[4] + '(' + '%' + result[3] + ')';
                        
                            // always a register in this case
                            dst = '%' + result[0];
                        }
                        
                    }
                    else {
                        src = result[6] + '(' + '%' + result[5] + ')';
                        dst = result[2] + '(' + '%' + result[1] + ')';
                    }


                    fprintf(outputFile, "\t%s %s, %s\n", operation.c_str(), src.c_str(), dst.c_str());
                    break;

                // instruction DNE
                case 4:
                    break;

                // compare and jump (cjmp)
                case 5:
                   
                    operand = result[2];
		    arg1 = result[1];
		    arg2 = result[3];
		    labeL2 = result[4];
		    label2 = result[5];	    
	
                    if (result[3][0] != 'r') {

                        // both are numbers 
                        if (result[1][0] != 'r') {
                
                            if (operand == "<") {
                                r = atoi(result[1].c_str()) < atoi(result[3].c_str());
                            }
                            else if (operand == "<=") {
                                r = atoi(result[1].c_str()) <= atoi(result[3].c_str());
                            }
                            else if (operand == "=") {
                                r = atoi(result[1].c_str()) == atoi(result[3].c_str());
                            }

                            if (r) {
                                result[4] = clean_label(result[4]);
                                fprintf(outputFile, "%s %s\n", "jmp", result[4].c_str());
                            } else {
                                result[5] = clean_label(result[5]);
                                fprintf(outputFile, "%s %s\n", "jmp", result[5].c_str());
                            }
                            break;
                        }

                        // only last one is a number

                        // negate
                        //if (operand == "<") { operand = ">"; }
                        //if (operand == "<=") { operand = ">="; }


                        // swap args so that reg is last (necessary)
                        extra_instruction = arg1;
		 	arg1 = arg2;
			arg2 = extra_instruction;
		 
			// swap true false labels
		        //extra_instruction = labeL2;
			//labeL2 = label2;
			//label2 = extra_instruction;
			       
                    } 
		    
		    else if (arg1[0] != 'r' && arg2[0] == 'r') {
		    	
			// negate
			if (operand == "<") { operand = ">"; }
			if (operand == "<=") { operand = ">=";  }

		    }


		    // both are registers -- need to swap
	            if (arg1[0] == 'r' && arg2[0] == 'r') {
			extra_instruction = arg1;
			arg1 = arg2;
			arg2 = extra_instruction;
		     }


                    if (operand == "<") { inst = "jl"; }
                    if (operand == "<=") { inst = "jle"; }
                    if (operand == ">") { inst = "jg"; }
                    if (operand == ">=") { inst = "jge"; }
                    if (operand == "=") { inst = "je"; }

                    if (arg1[0] == 'r') {
                        arg1.insert(0, 1, '%');
                    } else {
                        arg1.insert(0, 1, '$');
                    }

                    
                    arg2.insert(0, 1, '%');
                    labeL2 = clean_label(labeL2);
		    label2 = clean_label(label2);
 
		    fprintf(outputFile, "\tcmpq %s, %s\n",  arg1.c_str(), arg2.c_str());
		    fprintf(outputFile, "\t%s %s\n", inst.c_str(), labeL2.c_str());
		    fprintf(outputFile, "\tjmp %s\n", label2.c_str());
		     
		   // if (DEBUG_S) printf("arg1: %s, arg2: %s, labeL2: %s, label2: %s, inst: %s\n", arg1.c_str(), arg2.c_str(), labeL2.c_str(), label2.c_str(), inst.c_str());

                    break;

                // goto
                case 6:
                    result[1] = clean_label(result[1]);
                    fprintf(outputFile, "%s %s\n", "jmp", result[1].c_str());
                    break;

                // returns
                case 7:
                    offset = F->locals * 8;
                    //potentially pop off stack variables as well
                    if(F->arguments > 6){
                        offset += (F->arguments - 6) * 8;
                    }
		    
		    if (DEBUGGING) printf("return adding to the stack: %ld\n", offset);
                    if (offset) {
                        if(DEBUGGING) printf("\t%s $%ld, %%%s\n", "addq", offset, "rsp");
                        fprintf(outputFile, "\t%s $%ld, %%%s\n", "addq", offset, "rsp");
                    }
                    fprintf(outputFile, "\t%s\n", "retq");
                    break;

                // calls
                case 8:

                    if (result[1][0] != 'r' && result[1][0] != ':') {
                        fprintf(outputFile, "\t%s %s %s\n", result[0].c_str(), result[1].c_str(), "# runtime system call");
                        break;
                    }

                  
                    if(atoi(result[2].c_str()) <= 6){
                        offset = 8;

                    }
                    else{
                    	offset = (( (atoi(result[2].c_str())) - 6 ) * 8) + 8;
                   //	offset += 8; 
		   }
		   

		  if (DEBUGGING) printf("The offeset is %ld -- subtracting this from stack\n", offset);

                    fprintf(outputFile, "\t%s $%ld, %%%s\n", "subq", offset, "rsp");

                    // special register jump instruction
		    if (result[1][0] == 'r') {
                        fprintf(outputFile, "\t%s *%%%s\n", "jmp", result[1].c_str());
                    } else {
			// otherwise jmp to function
                        result[1] = clean_label(result[1]);
                        fprintf(outputFile, "\t%s %s\n", "jmp", result[1].c_str());
                    }   

                    break;

                // lea 
                case 9:

                    fprintf(outputFile, "\t%s (%%%s, %%%s, %s), %%%s\n", "lea", result[2].c_str(), result[3].c_str(), result[4].c_str(), result[0].c_str());
                    break;
                // compare assign
                case 10:

               	    arg1 = result[2];
		    arg2 = result[4]; 
                    operand = result[3];

                    if (result[4][0] != 'r') {

                        // both are numbers 
                        if (result[2][0] != 'r') {
                
                            if (operand == "<") {
                                r = atoi(result[2].c_str()) < atoi(result[4].c_str());
                            }
                            else if (operand == "<=") {
                                r = atoi(result[2].c_str()) <= atoi(result[4].c_str());
                            }
                            else if (operand == "=") {
                                r = atoi(result[2].c_str()) == atoi(result[4].c_str());
                            }

                            if (r) {
                                fprintf(outputFile, "\t%s %s, %%%s\n", "movq", "$1", result[0].c_str());
                            } else {
                                fprintf(outputFile, "\t%s %s, %%%s\n", "movq", "$0", result[0].c_str());
                            }
                            break;
                        }

                        // only last one is a number

                        // negate operands
                        //if (operand == "<") { operand = ">="; }
                        //if (operand == "<=") { operand = ">="; }


                        // swap results
                        extra_instruction = arg1;
			arg1 = arg2;
			arg2 = extra_instruction;
                    }

		    else if (arg1[0] != 'r') {
		    
			//negate operands
			if (operand == "<") { operand = ">=";  }
			if (operand == "<=") { operand = ">="; }

		    }
		    
		    if (arg1[0] == 'r' && arg2[0] == 'r') {
			// swap args
			extra_instruction = arg1;
			arg1 = arg2;
			arg2 = extra_instruction;	
			
			
		    }

                    if (operand == "<") { inst = "setl"; }
                    if (operand == "<=") { inst = "setle"; }
                    if (operand == ">") { inst = "setg"; }
                    if (operand == ">=") { inst = "setge"; }
                    if (operand == "=") { inst = "sete"; }

                    if (arg1[0] == 'r') {
                        arg1.insert(0, 1, '%');
                    } else {
                        arg1.insert(0, 1, '$');
                    }

                    
                    arg2.insert(0, 1, '%');

                    fprintf(outputFile, "\tcmpq %s, %s\n", arg1.c_str(), arg2.c_str());
		    fprintf(outputFile, "\t%s %s\n", inst.c_str(), register_map('%' + result[0]).c_str());
		    fprintf(outputFile, "\tmovzbq %s, %%%s\n", register_map('%' + result[0]).c_str(), result[0].c_str());
                 
                
                    break;

                // label inst
                case 11:
                    result[0] = clean_label(result[0]);
                    fprintf(outputFile, "\t%s:\n", result[0].c_str());
                    break;

                // increment / decrement
                case 12:
	            
		    arg1 = '%' + result[0]; 
                    if (result[1][0] == '+') {
                        fprintf(outputFile, "\tinc %s\n", arg1.c_str());
                    } else {
                        fprintf(outputFile, "\tdec %s\n", arg1.c_str());
                    }
                    break;

                default:
                    printf("%s\n", "error");

            }
            if(DEBUGGING) std::cout << "Done with instruction" << std::endl;

        }
        if(DEBUGGING) std::cout << "Done with function" << std::endl;

    }
    //std::cout << "closing file" << std::endl;
    fclose(outputFile);
   
    return ;
  }
        
}

 
