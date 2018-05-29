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

	void decodeArg(Arg* arg, vector<Instruction *>* newInsts) {

		Instruction_Declaration i_dec = new Instruction_Declaration();

		Int64* i64 = new Int64();

		i_dec->type = i64;

		Arg* var = new Arg();
		var->name = UD + arg->name;
		var->type = i64;

		i_dec->instruction = i64->name + " " + var->name;

		newInsts->push_back(i_dec);

		Instruction_opAssignment i_opAssign = new Instruction_opAssignment();
		i_opAssign->dst = var;
		i_opAssign->arg1 = arg;

		Number* one = new Number();
		one->name = "1";
		one->num = 1;

		i_opAssign->arg2 = one;

		Operation* op = new Operation();
		op->name = ">>";
		op->op = SHR;

		i_opAssign->operation = op;

		i_opAssign->instruction = dst->name + " <- " + arg1->name + " " + op->name + " " + arg2->name;

		newInsts->push_back(i_opAssign);
	}

	void encodeArg(Arg* arg, vector<Instruction *>* newInsts) {

		Instruction_Declaration i_dec = new Instruction_Declaration();

		Int64* i64 = new Int64();

		i_dec->type = i64;

		Arg* var = new Arg();
		var->name = UE + arg->name;
		var->type = i64;

		i_dec->instruction = i64->name + " " + var->name;

		newInsts->push_back(i_dec);

		Instruction_opAssignment i_opAssign = new Instruction_opAssignment();
		i_opAssign->dst = var;
		i_opAssign->arg1 = arg;

		Number* one = new Number();
		one->name = "1";
		one->num = 1;

		i_opAssign->arg2 = one;

		Operation* op = new Operation();
		op->name = "<<";
		op->op = SHL;

		i_opAssign->operation = op;

		i_opAssign->instruction = var->name + " <- " + arg->name + " " + op->name + " " + one->name;

		newInsts->push_back(i_opAssign);


		Instruction_opAssignment i_opAssign2 = new Instruction_opAssignment();
		i_opAssign2->dst = var;
		i_opAssign2->arg1 = var;
		
		i_opAssign2->arg2 = one;

		Operation* op2 = new Operation();
		op->name = "+";
		op->op = ADD;

		i_opAssign2->operation = op2;

		i_opAssign2->instruction = var->name + " <- " + var->name + " " + op2->name + " " + one->name;

		newInsts->push_back(i_opAssign);

	}

	void check_memory_access(Instruction_Assignment* I, vector<Instruction*>* newInsts) {
		Instruction_Load* i_load = dynamic_cast<Instruction_Load*>(I);
		Instruction_Store* i_store = dynamic_cast<Instruction_Store*>(I)
		int i = 0;
		string newInst = "";
		string legnthVar = "uniqueLengthHolderBrianDavid" + to_string(I->num);
	
		// check if load 
		if (i_load) {
			
			// check if array is 0

			Instruction_BrCmp* i_brcmp = new Instruction_BrCmp();
			string trueLabel = ":" + legnthVar + "trueLabel" + to_string(i_load->num);
			string falseLabel = ":" + legnthVar + "falseLabel" + to_string(i_load->num);
			i_brcmp->instruction = "br " + i_load->src->name + " " + trueLabel + " " + falseLabel;
			newInsts->push_back(i_brcmp)
			
			// if it is zero, call array-error
			Instruction_Label i_lbl = new Instruction_Label();
			i_lbl->instruction = falseLabel;
			newInsts->push_back(i_lbl);

			Instruction_Call i_call = new Instruction_Call();
			i_call->instruction = "call array-error(0,0)";
			newInsts->push_back(i_call);
			
			// if it isnt zero, continue execution
			Instruction_Label* i_lbl2 = new Instruction_Label();
			i_lbl2->instruction = trueLabel;
			newInsts->push_back(i_lbl2);			



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

	void parse_instruction(Instruction* I, vector<Instruction *>* newInsts) {

		if (Instruction_Declaration* i = dynamic_cast<Instruction_Declaration*>(I)) {

			if (Tuple* t = dynamic_cast<Tuple*>(i->type)) {
				
				i->instruction = type->name + " " + vale->name;

				newInsts->push_back(i);

				Instruction_Assignment* i_assign = new Instruction_Assignment();
				i_assign->dst = i->val;

				Number* zero = new Number();
				Number->name = "0";
				Number->num = 0;

				i_assign->src = zero;
				i_assign->instruction = i_assign->dst->name + " <- " + zero->name;

				newInsts->push_back(i_assign);
			}
			else if (Array* type = dynamic_cast<Array*>(i->type)) {

				i->instruction = "";

				i->instruction = i->type->name;

				for (int i = 0; i < i->dims; i++) {
					i->instruction.append("[]");
				}
				i->instruction.append(" " + var->name);

				newInsts->push_back(i);

				Instruction_Assignment* i_assign = new Instruction_Assignment();
				i_assign->dst = i->val;

				Number* zero = new Number();
				Number->name = "0";
				Number->num = 0;

				i_assign->src = zero;
				i_assign->instruction = i_assign->dst->name + " <- " + zero->name;

				newInsts->push_back(i_assign);
			}	
			else {
				i->instruction = type->name + " " + var->name;
				newInsts->push_back(i);
			}
		}
		else if (Instruction_BrCmp* i = dynamic_cast<Instruction_BrCmp*>(I)) {
			// decode t 
			decodeArg(i->comparitor, newInsts);
			i->instruction = "br " + UD + i->comparitor->name + " " + i->trueLabel->name + " " + i->falseLabel->name;

		}
		else if (Instruction_Length* i = dynamic_cast<Instruction_Length*>(I)) {
			// decode var3 (dimension)
			decodeArg(i->dimension, newInsts);
			i->instruction = i->dst->name + " <- length " + i->array->name + " " + UD + i->dimension->name;
		}
		else if (Instruction_Load* i = dynamic_cast<Instruction_Load*>(I)) {

			// set the begining of the instruction to be the memory check
			check_memory_access(i);
			newInst = i->instruction;

			// decode vari(s)
			for (Arg* idx : i->indexes) {
				decodeArg(idx, newInsts);
			}

			i->instruction = "";
			i->instruction = i->dst->name + " <- " + i->src->name;

			for (Arg* idx : i->indexes) {
				i->instruction.append('[' + UD + idx->name + ']');
			}
		}
		else if (Instruction_Store* i = dynamic_cast<Instruction_Store*>(I)) {
			
			// set the begining of the instruction to be the memory check
			check_memory_access(i);
			newInst = i->instruction;

			// decode vari(s)
			for (Arg* idx : i->indexes) {
				decodeArg(idx, newInsts);
			}

			i->instruction = "";
			i->instruction = i->dst->name;

			for (Arg* idx : i->indexes) {
				i->instruction.append('[' + UD + idx->name + ']');
			}

			i->instruction.append(" <- " + i->src->name);
		}
		else if (Instruction_opAssignment* i = dynamic_cast<Instruction_opAssignment*>(I)) {
			// decode arg1, arg2 (t1, t2)

			decodeArg(i->arg1, newInsts);
			decodeArg(i->arg2, newInsts);
			encodeArg(i->dst, newInsts);
			i->instruction = "";
			i->instruction = UE + i->dst->name + " <- " + UD + i->arg1->name + " " + i->operation->name + " " + UD + i->arg2->name;
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

        	fs << f->returnType->name << " " << f->name->name << "(";
            for(Arg* param : f->parameters){
                fs << param->name;
                if(param != f->parameters[f->parameters.size()-1]){
                    fs << ", ";
                }
            }
            fs << "){\n";
            vector<Instruction *> newInsts = {};
        	for (Instruction* I : f->instructions) {
        		parse_instruction(I, &newInsts);
        	}
        	// generate_basic_blocks(f);
        	fs << "}\n";
        }
        
        fs.close();
      }        
}

 
