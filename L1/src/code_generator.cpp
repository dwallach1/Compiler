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
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:

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
