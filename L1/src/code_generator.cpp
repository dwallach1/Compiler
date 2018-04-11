#include <string>
#include <iostream>
#include <fstream>

#include <code_generator.h>

using namespace std;

namespace L1{
  void generate_code(Program p){

    /* 
     * Open the output file.
     */ 
    std::ofstream outputFile;
    outputFile.open("prog.S");
   
    /* 
     * Generate target code
     */ 

    clean_label(p.entryPointLabel)

    // Hard coded begining 
    fprintf(outputFile, ".text\n
                        \t.globl go\n
                        go:\n
                        \tpushq %rbx\n
                        \tpushq %rbp\n
                        \tpushq %r12\n
                        \tpushq %r13\n
                        \tpushq %r14\n
                        \tpushq %r15\n
                        \tcall %s \n
                        \tpopq %r15\n
                        \tpopq %r14\n
                        \tpopq %r13\n
                        \tpopq %r12\n
                        \tpopq %rbp\n
                        \tpopq %rbx\n
                        \tretq\n", p.entryPointLabel);

    for (Function* F: p.functions) {
        clean_label(F->name);
        fprintf(outputFile, "%s:\n", F->name);
        for (Instruction* I: F->instructions) {

            // split instruction by words
            std::vector<std::string> result;
            std::istringstream iss(I->instruction);
            for(std::string s; iss >> s; )
                result.push_back(s);

            // initalize variables 
            std::string operation;
            std::string src;
            std::string dst;

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

                        dst = '%' + result[2];

                    }

                    else if (result.size() == 5) {

                        // 2 types of instructions: memory as source or as dest
                        int idx = result[0] == "mem" ? 3 : 1;

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
                            if (result[4][0] == 'r') {
                                dst = '%' + result[4];
                            } else {
                                dst = '$' + result[4];
                            }
                        }
                    } 
                    // special case to allocate small registers
                    if (operation == "salq" || operation == "sarq") {
                        src = register_map(src);
                    } 
                    // write instruction using predefined variables
                    fprintf(outputFile, "\t%s %s, %s\n", operation, src, dst);

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
                            src = '$' + clean_label(result[2])
                        }
                        else {
                            src = '$' + result[2];
                        }

                        // always a register in this case
                        dst = '%' + result[2];
                    }
                    // we have size of 5 
                    else if (result.size() == 5) {
                        if (result[0] == "mem") {
                            // determine if number of register
                            if (result[4][0] == 'r') {
                                src = '%' + result[4];
                            } 
                            else if (result[4][0] == ':'){
                                src = '$' + clean_label(result[4])
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


                    fprintf(stderr, "\t%s %s, %s\n", operation, src, dst);
                    break;

                // instruction DNE
                case 4:
                    break;

                // compare and jump (cjmp)
                case 5:
                    operation = "cmpq";
                    std::string arg1;
                    std::string arg2;
                    std::string extra_instruction;

                    std::string operand = result[2];
                    int r;
                    if (result[3][0] != 'r') {

                        // both are numbers 
                        if (result[1][0] != 'r') {
                
                            if (operand == "<") {
                                r = atoi(result[1]) < atoi(result[3]);
                            }
                            else if (operand == "<=") {
                                r = atoi(result[1]) <= atoi(result[3]);
                            }
                            else if (operand == "=") {
                                r = atoi(result[1]) == atoi(result[3]);
                            }

                            if (r) {
                                clean_label(result[4]);
                                fprintf(outputFile, "%s %s\n", "jmp", result[4]);
                            } else {
                                clean_label(result[5]);
                                fprintf(outputFile, "%s %s\n", "jmp", result[5]);
                            }
                            break;
                        }

                        // only last one is a number

                        // negate
                        if (operand == "<") { operand = ">="; }
                        if (operand == "<=") { operand = ">="; }


                        // swap
                        arg1 = result[1];
                        arg2 = result[3];
                        result[1] = arg2;
                        result[3] = arg1;
                    }

                    std::string inst;

                    if (operand == "<") { inst = "jl"; }
                    if (operand == "<=") { inst = "jle"; }
                    if (operand == ">") { inst = "jg"; }
                    if (operand == ">=") { inst = "jge"; }
                    if (operand == "=") { inst = "je"; }

                    if (result[1][0] == 'r') {
                        arg1 = '%' + result[1];
                    } else {
                        arg1 = '$' + result[1];
                    }

                    
                    arg2 = '%' + result[3];
                    clean_label(result[4]);
                    clean_label(result[5]);

                    fprintf(outputFile, "\t%s %s, %s\n
                                         \t%s %s\n
                                         \t%s %s\n", operation, arg1, arg2, inst, result[4], "jmp", result[5]);

                    break;

                // goto
                case 6:
                    clean_label(result[1])
                    fprintf(outputFile, "%s %s\n", "jmp", result[1]);
                    break;

                // return
                case 7:
                    uint64_t offset = F->locals * 8;
                    if (offset) {
                        fprintf(outputFile, "\t%s $%d, %%%s\n", "addq", offset, "rsp");
                    }
                    fprintf(outputFile, "\t%s\n", "retq");
                    break;

                // call
                case 8:

                    if (result[1][0] != 'r' || result[1][0] != ':') {
                        fprintf(outputFile, "\t%s %s %s\n", result[0], result[1], "# runtime system call");
                        break;
                    }

                    uint64_t offset = ((atoi(result[2]) - 6 ) * 8) + 8;
                    if (offset < 8) { offset = 8; }

                    fprintf(outputFile, "\t%s $%d, %%%s\n", "subq", offset, "rsp");

                    if (result[1][0] == 'r') {
                        fprintf(outputFile, "\t%s *%%%s\n", "jmp", "rdi");
                    } else {
                        clean_label(result[1])
                        fprintf(outputFile, "\t%s %s\n", "jmp", result[1]);
                    }   

                    break;

                // lea 
                case 9:
                    fprintf(outputFile, "\t%s (%%%s, %%%s, %s), %%%s\n", "lea", result[2], result[3], result[4], result[0]);

                // compare assign
                case 10:

                    operation = "cmpq";
                    std::string arg1;
                    std::string arg2;
                    std::string extra_instruction;

                    std::string operand = result[3];
                    int r;
                    if (result[4][0] != 'r') {

                        // both are numbers 
                        if (result[2][0] != 'r') {
                
                            if (operand == "<") {
                                r = atoi(result[2]) < atoi(result[4]);
                            }
                            else if (operand == "<=") {
                                r = atoi(result[2]) <= atoi(result[4]);
                            }
                            else if (operand == "=") {
                                r = atoi(result[2]) == atoi(result[4]);
                            }

                            if (r) {
                                fprintf(outputFile, "\t%s %s, %%%s\n", "movq", "$1", result[0]);
                            } else {
                                fprintf(outputFile, "\t%s %s, %%%s\n", "movq", "$0", result[0]);
                            }
                            break;
                        }

                        // only last one is a number

                        // negate operands
                        if (operand == "<") { operand = ">="; }
                        if (operand == "<=") { operand = ">="; }


                        // swap results
                        arg1 = result[2];
                        arg2 = result[4];
                        result[2] = arg2;
                        result[4] = arg1;
                    }

                    std::string inst;

                    if (operand == "<") { inst = "setl"; }
                    if (operand == "<=") { inst = "setle"; }
                    if (operand == ">") { inst = "setg"; }
                    if (operand == ">=") { inst = "setge"; }
                    if (operand == "=") { inst = "sete"; }

                    if (result[2][0] == 'r') {
                        arg1 = '%' + result[2];
                    } else {
                        arg1 = '$' + result[2];
                    }

                    
                    arg2 = '%' + result[4];

            
                    fprintf(outputFile, "\t%s %s, %s\n
                                         \t%s %%%s\n
                                         \t%s %%%s, %%%s\n", operation, arg1, arg2, inst, register_map(result[0]), "movzbq", register_map(result[0]), result[0]);

                    break;

                // label inst
                case 11:
                    clean_label(result[0]);
                    fprintf(outputFile, "%s:\n", result[0]);
                    break;

                // increment / decrement
                case 12:

                    if (result[0][3] == '+') {
                        fprintf(outputFile, "\t%s %%%c%c%c\n", "inc", result[0][0], result[0][1], result[0][2]);
                    } else {
                        fprintf(outputFile, "\t%s %%%c%c%c\n", "dec", result[0][0], result[0][1], result[0][2]);
                    }
                    break;



            }
        }
    }



    /* 
     * Close the output file.
     */ 
    outputFile.close();
   
    return ;
  }

  void clean_label(String* label) {
    if (label[0] == ':') {
        label[0] = '_';
    }
  }

  std::string register_map(std::string r) {
    switch (r) {
        case "r10":
            return "r10b"
        case "r11":
            return "r11b"
        case "r12":
            return "r12b"
        case "r13":
            return "r13b"
        case "r14":
            return "r14b"
        case "r15":
            return "r15b"
        case "r8":
            return "r8b"
        case "r9":
            return "r9b"
        case "rax":
            return "al"
        case "rbp":
            return "bpl"
        case "rbx":
            return "bl"
        case "rcx":
            return "cl"
        case "rdi":
            return "dil"
        case "rdx":
            return "dl"
        case "rsi":
            return "sil"
    }
  }
}
