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

namespace L1{

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

    // std::ofstream outputFile;
    FILE *outputFile;

    //std::cout << "opening file" << std::endl;
    outputFile = fopen("prog.S", "w");
    // outputFile.open("prog.S");

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

        //Need to manually do some stack stuff in the main function if needed
        //if(F->name == p.entryPointLabel){
           // if(DEBUGGING) printf("This is the main function\n");
            if(F->locals > 0){
                if(DEBUGGING) printf("There are locals in this function so I am adding some stack space for it\n");
                fprintf(outputFile, "subq $%ld, %%rsp\n", F->locals*8);
            }
        //}
        if(DEBUGGING) std::cout << "Beginning to iterate through the instructions" << std::endl;

        for (Instruction* I: F->instructions) {
            int tempPos = 0;
            std:string space = " ";
            if(DEBUGGING) std::cout << "Working on instruction: " << I->instruction << std::endl;

            // for(char temp : I->instruction){
            //     if(temp == '+' | (temp == '-' && I->instruction[tempPos+1] == '=') | temp == '*' | temp == '<' | temp == '>' | temp == '=' | temp == '&'){
            //         //check space before
            //         if(I->instruction[tempPos-1] != ' '){
            //             printf("Augmenting string because: %s\n", I->instruction.c_str());
            //             I->instruction.insert(tempPos, space);
            //         }

            //         //check after
            //         //two character ops
            //         if(temp == '+' | temp == '-' | temp == '*' | temp == '&' | (temp == '>' && I->instruction[tempPos+1] == '=') | (temp == '<' && I->instruction[tempPos+1] == '=') | (temp == '<' && I->instruction[tempPos+1] == '-')){
            //             if (I->instruction[tempPos+2] != ' ')
            //             {
            //                 printf("Augmenting string because: %s\n", I->instruction.c_str());
            //                 I->instruction.insert(tempPos+2, space);
            //             }
            //         }
            //         //one char ops
            //         else if (temp == '=' | (temp == '>' && I->instruction[tempPos+1] != '>') | (temp == '<' && I->instruction[tempPos+1] != '<')){
            //             if (I->instruction[tempPos+1] != ' ')
            //             {
            //                 printf("Augmenting string because: %s\n", I->instruction.c_str());
            //                 I->instruction.insert(tempPos+1, space);
            //             }
            //         }
            //         else{
            //             if (I->instruction[tempPos+3] != ' ')
            //             {
            //                 printf("Augmenting string because: %s\n", I->instruction.c_str());
            //                 I->instruction.insert(tempPos+3, space);
            //             }
            //         }
            //         break;
            //     }
            //     else{
            //         tempPos++;
            //     }
            // }
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
            std::string label1;
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
                    //printf("%s\n", I->instruction.c_str());
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
		    label1 = result[4];
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
		        //extra_instruction = label1;
			//label1 = label2;
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
                    label1 = clean_label(label1);
		    label2 = clean_label(label2);
 
		    fprintf(outputFile, "\tcmpq %s, %s\n",  arg1.c_str(), arg2.c_str());
		    fprintf(outputFile, "\t%s %s\n", inst.c_str(), label1.c_str());
		    fprintf(outputFile, "\tjmp %s\n", label2.c_str());
		     
		   // if (DEBUG_S) printf("arg1: %s, arg2: %s, label1: %s, label2: %s, inst: %s\n", arg1.c_str(), arg2.c_str(), label1.c_str(), label2.c_str(), inst.c_str());

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

                    if (result[0][3] == '+') {
                        fprintf(outputFile, "\t%s %%%c%c%c\n", "inc", result[0][0], result[0][1], result[0][2]);
                    } else {
                        fprintf(outputFile, "\t%s %%%c%c%c\n", "dec", result[0][0], result[0][1], result[0][2]);
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

 
