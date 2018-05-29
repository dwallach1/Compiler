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

namespace LA {
	string UE = "uniqueEncodedDavidBrian";
	string UD = "uniqueDecodedDavidBrian";

	string decodeName(string name) {

		return UD + name + " <- " + name + " >> 1\n";
	}

	string encodeName(string name) {

		return UE + name + " <- " + name + " << 1\n" + UE + name + " <- " + UE + name + " + 1\n";
	}

	void check_memory_access(Instruction_Assignment* I) {
		Instruction_Load* i_load = dynamic_cast<Instruction_Load*>(I);
		Instruction_Store* i_store = dynamic_cast<Instruction_Store*>(I)
		int i = 0;
		string newInst = "";
		string legnthVar = "uniqueLengthHolderBrianDavid";
	
		// check if load 
		if (i_load) {
			
			// check if array is 0
			newInst.append(legnthVar + " <- " + i_load->src->name + " = 0\n");
			string trueLabel = ":" + legnthVar + "trueLabel" + to_string(i_load->num);
			string falseLabel = ":" + legnthVar + "falseLabel" + to_string(i_load->num);
			newInst.append("br " + legnthVar + trueLabel + " " + falseLabel + "\n");
			
			// if it is zero, call array-error
			newInst.append(trueLabel +  "\n");
			newInst.append("call array-error(0,0)\n");
			
			// if it isnt zero, continue execution
			newInst.append(falseLabel);
			



			for (Arg* idx : i_load->indexes) {

				// load the current dimensions length
				newInst.append(legnthVar + " <- length " + i_load->src->name + " " + idx->name + "\n");

				// check if less than length 
				newInst.append(legnthVar + to_string(i) + " <- " + idx->name + " < " + legnthVar + "\n");
				newInst.append("br " + legnthVar + to_string(i) + ":" + legnthVar + to_string(i) + "trueLabel" + " " + ":" + legnthVar + to_string(i) + "falseLabel\n");
				
				// jump for array error
				newInst.append(":" + legnthVar + to_string(i) + "falseLabel\n");
				newInst.append("call array-error(" + i->src->name + "," + idx->name);

				// jump for no error
				newInst.append(":" + legnthVar + to_string(i) + "trueLabel\n");

				i++;
			}
			i_load->instruction = newInst;
		}
		// otherwise its a store
		else {

			// check if array is 0
			newInst.append(legnthVar + " <- " + i_store->dst->name + " = 0\n");
			string trueLabel = ":" + legnthVar + "trueLabel" + to_string(i_store->num);
			string falseLabel = ":" + legnthVar + "falseLabel" + to_string(i_store->num);
			newInst.append("br " + legnthVar + trueLabel + " " + falseLabel + "\n");
			
			// if it is zero, call array-error
			newInst.append(trueLabel +  "\n");
			newInst.append("call array-error(0,0)\n");
			
			// if it isnt zero, continue execution
			newInst.append(falseLabel);


			for (Arg* idx : i_store->indexes) {

				// load the current dimensions length
				newInst.append(legnthVar + " <- length " + i_load->dst->name + " " + idx->name + "\n");

				// check if less than length 
				newInst.append(legnthVar + to_string(i) + " <- " + idx->name + " < " + legnthVar + "\n");
				newInst.append("br " + legnthVar + to_string(i) + ":" + legnthVar + to_string(i) + "trueLabel" + " " + ":" + legnthVar + to_string(i) + "falseLabel\n");
				
				// jump for array error
				newInst.append(":" + legnthVar + to_string(i) + "falseLabel\n");
				newInst.append("call array-error(" + i->dst->name + "," + idx->name);

				// jump for no error
				newInst.append(":" + legnthVar + to_string(i) + "trueLabel\n");

				i++;
			}

			i->store->instruction = newInst;

		}
	}


	void parse_instruction(Instruction* I) {
		string newInst = "";


		if (Instruction_Declaration* i = dynamic_cast<Instruction_Declaration*>(I)) {

			if (Tuple* t = dynamic_cast<Tuple*>(i->type)) {
				
				newInst.append(type->name + " " + arg->name + "\n");
				newInst.append(arg->name + " <- 0");
			}
			else if (Array* type = dynamic_cast<Array*>(i->type)) {

				newInst.append(i->type->name);

				for (int i = 0; i < i->dims; i++) {
					newInst.append("[]");
				}
				newInst.append(" " + arg->name + "\n");

				newInst.append(arg->name + " <- 0");
			}	
			else {
				newInst.append(type->name + " " + arg->name);
			}
		}
		else if (Instruction_BrCmp* i = dynamic_cast<Instruction_BrCmp*>(I)) {
			// decode t 
			newInst.append(decodeName(i->comparitor->name));
			newInst.append("br " + UD + i->comparitor->name + " " + i->trueLabel->name + " " + i->falseLabel->name);

			i->instruction = newInst;
		}
		else if (Instruction_Length* i = dynamic_cast<Instruction_Length*>(I)) {
			// decode var3 (dimension)
			newInst.append(decodeName(i->dimension->name));
			newInst.append(i->dst->name + " <- length " + i->array->name + " " + UD + i->dimension->name);
			i->instruction = newInst;
		}
		else if (Instruction_Load* i = dynamic_cast<Instruction_Load*>(I)) {

			// set the begining of the instruction to be the memory check
			check_memory_access(i);
			newInst = i->instruction;

			// decode vari(s)
			for (Arg* idx : i->indexes) {
				newInst.append(decodeName(idx->name));
			}

			newInst.append(i->dst->name + " <- " + i->src->name);

			for (Arg* idx : i->indexes) {
				newInst.append('[' + UD + idx->name + ']');
			}
		}
		else if (Instruction_Store* i = dynamic_cast<Instruction_Store*>(I)) {
			
			// set the begining of the instruction to be the memory check
			check_memory_access(i);
			newInst = i->instruction;

			// decode vari(s)
			for (Arg* idx : i->indexes) {
				newInst.append(decodeName(idx->name));
			}

			newInst.append(i->dst->name);

			for (Arg* idx : i->indexes) {
				newInst.append('[' + UD + idx->name + ']');
			}

			newInst.append(" <- " + i->src->name);
		}
		else if (Instruction_opAssignment* i = dynamic_cast<Instruction_opAssignment*>(I)) {
			// decode arg1, arg2 (t1, t2)

			newInst.append(decodeName(i->arg1->name));
			newInst.append(decodeName(i->arg2->name));
			newInst.append(encodeName(i->dst->name));
			newInst.append(UE + i->dst->name + " <- " + UD + i->arg1->name + " " + i->operation->name + " " + UD + i->arg2->name);
		}

		else {
			// instruction remains the same
			return;
		}
		return;
	}

	void number_instructions(Function* f) {
		int i = 0;
		for (Instruction* I : f->instructions) {
			I->num = i;
			i++;
		}
	}

    void LA_generate_code(Program p) {
        
        // set up file stream
        std::fstream fs;
        fs.open("prog.LA", std::fstream::in | std::fstream::out | std::fstream::app);

        for (Function* f : p.functions) {
        	number_instructions(f);

        	fs << f->name->name << "(";
            for(Arg* param : f->parameters){
                fs << param->name;
                if(param != f->parameters[f->parameters.size()-1]){
                    fs << ", ";
                }
            }
            fs << "){\n";
        	for (Instruction* I : f->instructions) {
        		parse_instruction(I);
        		fs << I->instruction << endl;
        	}
        	fs << "}\n";
        }
        
        fs.close();
      }        
}

 
