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

		Instruction_Declaration* i_dec = new Instruction_Declaration();

		Int64* i64 = new Int64();

		i_dec->type = i64;

		Arg* var = new Arg();
		var->name = UD + arg->name;
		var->type = i64;

		i_dec->instruction = i64->name + " " + var->name;

		newInsts->push_back(i_dec);

		Instruction_opAssignment* i_opAssign = new Instruction_opAssignment();
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

		i_opAssign->instruction = var->name + " <- " + arg->name + " " + op->name + " " + one->name;

		newInsts->push_back(i_opAssign);
	}

	void encodeArg(Arg* arg, vector<Instruction *>* newInsts) {

		Instruction_Declaration* i_dec = new Instruction_Declaration();

		Int64* i64 = new Int64();

		i_dec->type = i64;

		Arg* var = new Arg();
		var->name = UE + arg->name;
		var->type = i64;

		i_dec->instruction = i64->name + " " + var->name;

		newInsts->push_back(i_dec);

		Instruction_opAssignment* i_opAssign = new Instruction_opAssignment();
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


		Instruction_opAssignment* i_opAssign2 = new Instruction_opAssignment();
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
		Instruction_Store* i_store = dynamic_cast<Instruction_Store*>(I);
		int i = 0;
		string legnthVar = "uniqueLengthHolderBrianDavid" + to_string(I->num);
	
		// check if load 
		if (i_load) {
			
			// check if array is 0

			Instruction_brCmp* i_brcmp = new Instruction_brCmp();
			string trueLabel = ":" + legnthVar + "trueLabel" + to_string(i_load->num);
			string falseLabel = ":" + legnthVar + "falseLabel" + to_string(i_load->num);
			i_brcmp->instruction = "br " + i_load->src->name + " " + trueLabel + " " + falseLabel;
			newInsts->push_back(i_brcmp);
			
			// if it is zero, call array-error
			Instruction_Label* i_lbl = new Instruction_Label();
			i_lbl->instruction = falseLabel;
			newInsts->push_back(i_lbl);

			Instruction_Call* i_call = new Instruction_Call();
			i_call->instruction = "call array-error(0,0)";
			newInsts->push_back(i_call);
			
			// if it isnt zero, continue execution
			Instruction_Label* i_lbl2 = new Instruction_Label();
			i_lbl2->instruction = trueLabel;
			newInsts->push_back(i_lbl2);			



			for (Arg* idx : i_load->indexes) {

				// load the current dimensions length
				Instruction_Length* i_length = new Instruction_Length();
				i_length->instruction = legnthVar + " <- length " + i_load->src->name + " " + idx->name;
				newInsts->push_back(i_length);

				// check if less than length 
				Instruction_opAssignment* i_opAssign = new Instruction_opAssignment();
				i_opAssign->instruction = legnthVar + to_string(i) + " <- " + idx->name + " < " + legnthVar;
				newInsts->push_back(i_opAssign);

				Instruction_brCmp* i_brcmp2 = new Instruction_brCmp();
				i_brcmp2->instruction = "br " + legnthVar + to_string(i) + ":" + legnthVar + to_string(i) + "trueLabel" + " " + ":" + legnthVar + to_string(i) + "falseLabel";
				newInsts->push_back(i_brcmp2);
				
				// jump for array error
				Instruction_Label* i_lbl3 = new Instruction_Label();
				i_lbl3->instruction = ":" + legnthVar + to_string(i) + "falseLabel";
				newInsts->push_back(i_lbl3);

				Instruction_Call* i_call2 = new Instruction_Call();
				i_call2->instruction = "call array-error(" + i_load->src->name + "," + idx->name + ")";
				newInsts->push_back(i_call2);

				// jump for no error
				Instruction_Label* i_lbl4 = new Instruction_Label();
				i_lbl4->instruction = ":" + legnthVar + to_string(i) + "trueLabel";
				newInsts->push_back(i_lbl4);

				i++;
			}
		}
		// otherwise its a store
		else {

			// check if array is 0
			Instruction_brCmp* i_brcmp3 = new Instruction_brCmp();
			string trueLabel = ":" + legnthVar + "trueLabel" + to_string(i_store->num);
			string falseLabel = ":" + legnthVar + "falseLabel" + to_string(i_store->num);
			i_brcmp3->instruction = "br " + i_store->dst->name + " " + trueLabel + " " + falseLabel;
			newInsts->push_back(i_brcmp3);
			
			// if it is zero, call array-error
			Instruction_Label* i_lbl5 = new Instruction_Label();
			i_lbl5->instruction = falseLabel;
			newInsts->push_back(i_lbl5);

			Instruction_Call* i_call3 = new Instruction_Call();
			i_call3->instruction = "call array-error(0,0)";
			newInsts->push_back(i_call3);
			
			// if it isnt zero, continue execution
			Instruction_Label* i_lbl6 = new Instruction_Label();
			i_lbl6->instruction = trueLabel;
			newInsts->push_back(i_lbl6);


			for (Arg* idx : i_store->indexes) {

				// load the current dimensions length
				Instruction_Length* i_length2 = new Instruction_Length();
				i_length2->instruction = legnthVar + " <- length " + i_load->dst->name + " " + idx->name;
				newInsts->push_back(i_length2);

				// check if less than length 
				Instruction_opAssignment* i_opAssign3 = new Instruction_opAssignment();
				i_opAssign3->instruction = legnthVar + to_string(i) + " <- " + idx->name + " < " + legnthVar;
				newInsts->push_back(i_opAssign3);

				Instruction_brCmp* i_brcmp4 = new Instruction_brCmp();
				i_brcmp4->instruction = "br " + legnthVar + to_string(i) + ":" + legnthVar + to_string(i) + "trueLabel" + " " + ":" + legnthVar + to_string(i) + "falseLabel";
				newInsts->push_back(i_brcmp4);
				
				// jump for array error
				Instruction_Label* i_lbl7 = new Instruction_Label();
				i_lbl7->instruction = ":" + legnthVar + to_string(i) + "falseLabel";
				newInsts->push_back(i_lbl7);

				Instruction_Call* i_call4 = new Instruction_Call();
				i_call4->instruction = "call array-error(" + i_store->dst->name + "," + idx->name + ")";
				newInsts->push_back(i_call4);

				// jump for no error
				Instruction_Label* i_lbl8 = new Instruction_Label();
				i_lbl8->instruction = ":" + legnthVar + to_string(i) + "trueLabel\n";
				newInsts->push_back(i_lbl8);

				i++;
			}
		}
	}

	void parse_instruction(Instruction* I, vector<Instruction *>* newInsts) {

		if (Instruction_Declaration* i = dynamic_cast<Instruction_Declaration*>(I)) {

			if (Tuple* t = dynamic_cast<Tuple*>(i->type)) {
				
				i->instruction = i->type->name + " " + i->var->name;

				newInsts->push_back(i);

				Instruction_Assignment* i_assign = new Instruction_Assignment();
				i_assign->dst = i->var;

				Number* zero = new Number();
				zero->name = "0";
				zero->num = 0;

				i_assign->src = zero;
				i_assign->instruction = i_assign->dst->name + " <- " + zero->name;

				newInsts->push_back(i_assign);
			}
			else if (Array* type = dynamic_cast<Array*>(i->type)) {

				i->instruction = "";

				i->instruction = i->type->name;
				Array* array = dynamic_cast<Array*>(i->type);
				for (int j = 0; j < array->dims; j++) {
					i->instruction.append("[]");
				}
				i->instruction.append(" " + i->var->name);

				newInsts->push_back(i);

				Instruction_Assignment* i_assign = new Instruction_Assignment();
				i_assign->dst = i->var;

				Number* zero = new Number();
				zero->name = "0";
				zero->num = 0;

				i_assign->src = zero;
				i_assign->instruction = i_assign->dst->name + " <- " + zero->name;

				newInsts->push_back(i_assign);
			}	
			else {
				i->instruction = i->type->name + " " + i->var->name;
				newInsts->push_back(i);
			}
		}
		else if (Instruction_brCmp* i = dynamic_cast<Instruction_brCmp*>(I)) {
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
			check_memory_access(i, newInsts);

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
			check_memory_access(i, newInsts);

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

	void generate_basic_blocks(Function* f, vector<Instruction *>* newInsts) {
		Instruction* inst = f->instructions[0];
		bool startBB = true;
		string uniqueLabel = "uniqueLabelDavidAndBrian";
		int i = 0;
		while (inst) {
			Instruction_Label* i_lbl = dynamic_cast<Instruction_Label*>(inst);
			if (startBB) {
				if (!i_lbl) {
					Instruction_Label* new_label = new Instruction_Label();
					new_label->instruction = uniqueLabel + to_string(i);
					newInsts->push_back(new_label);
				}
				startBB = false;
			}
			else if (i_lbl) {
				Instruction_br* i_br = new Instruction_br();
				i_br->instruction = uniqueLabel + to_string(i);
				newInsts->push_back(i_br);
			}
			newInsts->push_back(inst);

			Instruction_br* i_brr = dynamic_cast<Instruction_br*>(inst);
			Instruction_brCmp* i_brcmp = dynamic_cast<Instruction_brCmp*>(inst);
			Instruction_Return* i_ret = dynamic_cast<Instruction_Return*>(inst);
			Instruction_ReturnVal* i_retVal = dynamic_cast<Instruction_ReturnVal*>(inst);
			bool terminator = i_brr || i_brcmp || i_ret || i_retVal;
			
			if (terminator) {
				startBB = true;
			}

			i++;

			if (i > f->instructions.size() - 1) { inst = NULL; }
		}
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
        	f->instructions = newInsts;

        	newInsts = {};
        	generate_basic_blocks(f, &newInsts);
        	
        	f->instructions = newInsts;
        	for (Instruction* I : f->instructions) {
        		fs << I->instruction << endl;
        	}
        	fs << "}\n";
        }
        
        fs.close();
      }        
}

 
