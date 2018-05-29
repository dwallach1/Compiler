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

	string decodeName(string name) {

		return name + " <- " + name + " >> 1\n";
	}

	string encodeName(string name) {

		return name + " <- " + name + " << 1\n" + name + " <- " + name + " + 1\n";
	}

	void parse_instruction(Instruction* I) {
		string newInst = "";
		if (Instruction_BrCmp* i = dynamic_cast<Instruction_BrCmp*>(I)) {
			// decode t 
			newInst.append(decodeName(i->comparitor->name));
			newInst.append("br " + i->comparitor->name + " " + i->trueLabel->name + " " + i->falseLabel->name);

			i->instruction = newInst;
		}
		else if (Instruction_Length* i = dynamic_cast<Instruction_Length*>(I)) {
			// decode var3 (dimension)
			newInst.append(decodeName(i->dimension->name));
			newInst.append(i->dst->name + " <- length " + i->array->name + " " + i->dimension->name);
			i->instruction = newInst;
		}
		else if (Instruction_Load* i = dynamic_cast<Instruction_Load*>(I)) {
			// decode vari(s)
			for (Arg* idx : i->indexes) {
				newInst.append(decodeName(idx->name));
			}

			newInst.append(i->dst->name + " <- " + i->src->name);

			for (Arg* idx : i->indexes) {
				newInst.append('[' + idx->name + ']');
			}
		}
		else if (Instruction_Store* i = dynamic_cast<Instruction_Store*>(I)) {
			// decode vari(s)
			for (Arg* idx : i->indexes) {
				newInst.append(decodeName(idx->name));
			}

			newInst.append(i->dst->name);

			for (Arg* idx : i->indexes) {
				newInst.append('[' + idx->name + ']');
			}

			newInst.append(" <- " + i->src->name);
		}
		else if (Instruction_opAssignment* i = dynamic_cast<Instruction_opAssignment*>(I)) {
			// decode arg1, arg2 (t1, t2)

			newInst.append(decodeName(i->arg1->name));
			newInst.append(decodeName(i->arg2->name));
			newInst.append(encodeName(i->dst->name));
			newInst.append(i->dst->name + " <- " + i->arg1->name + " " + i->operation->name + " " + i->arg2->name);
		}

		else {
			// instruction remains the same
			return;
		}
		return;
	}

    void LA_generate_code(Program p) {
        
        // set up file stream
        std::fstream fs;
        fs.open("prog.LA", std::fstream::in | std::fstream::out | std::fstream::app);

        for (Function* f : p.functions) {
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

 
