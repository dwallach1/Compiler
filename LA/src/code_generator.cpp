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
#include <map>
#define DEBUGGING 0
#define DEBUG_S 0



using namespace std;

map<string,string> renamedVars;
namespace LA {
	string UE = "%uniqueEncodedDavidBrian";
	string UD = "%uniqueDecodedDavidBrian";

	Arg* var_exists(Function* f, Arg* var) {
		for (Arg* arg :  f->declared_variables) {
			if (arg->name == var->name ) { return arg; }
		}
		return var;
	}

	string decodeArg(Function* f, Arg* arg, vector<Instruction *>* newInsts) {

		Instruction_Declaration* i_dec = new Instruction_Declaration();

		Int64* i64 = new Int64();

		i_dec->type = i64;
        i_dec->type->name = "int64";
		Arg* var = new Arg();
		if(Number* num = dynamic_cast<Number*>(arg)){
			var->name = UD + arg->name;
		}
		else{
			var->name = UD + arg->name.substr(1);
		}
		var->type = i64;

		i_dec->instruction = i64->name + " " + var->name;


		// we do not want to redeclare the same variables 
		Arg* var_checker = var_exists(f, var);

		if (var == var_checker) { 
			newInsts->push_back(i_dec); 
			f->declared_variables.insert(var);
		}

		

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

		return var->name;
	}

	string encodeArg(Function* f, Arg* arg, vector<Instruction *>* newInsts) {

		Instruction_Declaration* i_dec = new Instruction_Declaration();

		Int64* i64 = new Int64();

		i_dec->type = i64;
        i_dec->type->name = "int64";

		Arg* var = new Arg();
		var->name = arg->name;

		if(Number* num = dynamic_cast<Number*> (arg)){
			var->name = UE + arg->name;
		}
		else{
			//var->name = UE + arg->name.substr(1);
			var->name = arg->name;
		}

		var->type = i64;

		i_dec->instruction = i64->name + " " + var->name;

		// we do not want to redeclare the same variables 
		Arg* var_checker = var_exists(f, var);

		if (var == var_checker) { 
			newInsts->push_back(i_dec); 
			f->declared_variables.insert(var);
		}

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
		op2->name = "+";
		op2->op = ADD;

		i_opAssign2->operation = op2;

		i_opAssign2->instruction = var->name + " <- " + var->name + " " + op2->name + " " + one->name;

		newInsts->push_back(i_opAssign2);
		return var->name;
	}

	void check_memory_access(Function* f, Instruction_Assignment* I, vector<Instruction*>* newInsts) {
		Instruction_Load* i_load = dynamic_cast<Instruction_Load*>(I);
		Instruction_Store* i_store = dynamic_cast<Instruction_Store*>(I);
		string legnthVar = "uniqueLengthHolderBrianDavid" + to_string(I->num);
		int i = 0;
	
		// check if load 
		if (i_load) {
			
			Instruction_opAssignment* i_isZero = new Instruction_opAssignment();
			Arg* isZeroDest = new Arg();
			isZeroDest->name = "%" + legnthVar;
			Number* zero = new Number();
			zero->name = "0";
			zero->num = 0;
			i_isZero->instruction = isZeroDest->name + " <- " + i_load->src->name + " = " +  zero->name;
			
			Instruction_Declaration* i_dec = new Instruction_Declaration();
			i_dec->instruction =  "int64 " + isZeroDest->name;
	
			// we do not want to redeclare the same variables 
			Arg* var_checker = var_exists(f, isZeroDest);
			if (isZeroDest == var_checker) { 
				newInsts->push_back(i_dec); 
				f->declared_variables.insert(isZeroDest);
			}
			newInsts->push_back(i_isZero);

			// check if array is 0
			Instruction_brCmp* i_brcmp = new Instruction_brCmp();
			string trueLabel = ":" + legnthVar + "trueLabel" + to_string(i_load->num);
			string falseLabel = ":" + legnthVar+ "falseLabel" + to_string(i_load->num);
			
			i_brcmp->instruction = "br " +  isZeroDest->name + " " + trueLabel + " " + falseLabel;
			newInsts->push_back(i_brcmp);

			
			// if it is zero, call array-error
			Instruction_Label* i_lbl = new Instruction_Label();
			i_lbl->instruction = trueLabel;
			newInsts->push_back(i_lbl);

			Instruction_Call* i_call = new Instruction_Call();
			i_call->instruction = "call array-error(0,0)";
			newInsts->push_back(i_call);
			
			// if it isnt zero, continue execution
			Instruction_Label* i_lbl2 = new Instruction_Label();
			i_lbl2->instruction = falseLabel;
			newInsts->push_back(i_lbl2);			


			if (dynamic_cast<Tuple*>(i_load->src->type)) { return; }

			int indexNum = 0;
			int k = 0;
			for (Arg* idx : i_load->indexes) {

				// load the current dimensions length
				Arg* newLength = new Arg();
				newLength->name = "%" + legnthVar;

				int idxDecoded = 0;
				if (!dynamic_cast<Number*>(idx)){

					Instruction_opAssignment* decode_idx = new Instruction_opAssignment();
					decode_idx->instruction = idx->name + " <- " + idx->name + " >> 1";
					newInsts->push_back(decode_idx);
					idxDecoded = 1;
				}

				// get the length of the current "indexNum" dimension
				Instruction_Length* i_length = new Instruction_Length();
				i_length->instruction = newLength->name + " <- length " + i_load->src->name + " " + to_string(indexNum);
				newInsts->push_back(i_length);

				/*
				 *  this snippet is used for debugging, call array-error seems to be brokn
				 *
				 */
				Instruction_Call* debug_print = new Instruction_Call();
				debug_print->instruction = "call print(" + newLength->name + ")";
				//newInsts->push_back(debug_print);
				// end of debug snippet

				// decode the length value (its encoded by default)
				Instruction_opAssignment* decode_length = new Instruction_opAssignment();
				decode_length->instruction = newLength->name + " <- " + newLength->name + " >> 1";
				newInsts->push_back(decode_length);

				// create and declare instruction (if necessary) the result of if dimesnion < length 
				Arg* lengthResult = new Arg();
				lengthResult->name = newLength->name + to_string(i);

				Instruction_Declaration* i_dec = new Instruction_Declaration();
				i_dec->instruction =  "int64 " + lengthResult->name;
		
				// we do not want to redeclare the same variables 
				Arg* var_checker = var_exists(f, lengthResult);
				if (lengthResult == var_checker) { 
					newInsts->push_back(i_dec); 
					f->declared_variables.insert(lengthResult);
				}

				// check if less than length 
				Instruction_opAssignment* i_opAssign = new Instruction_opAssignment();
				if (dynamic_cast<Number*>(idx)) {
					//i_opAssign->instruction = lengthResult->name + " <- " + idx->name + " < " + newLength->name;
					 i_opAssign->instruction = lengthResult->name + " <- " + idx->name + " < " + newLength->name;
				}
				else {
					i_opAssign->instruction = lengthResult->name + " <- " + idx->name + " < " + newLength->name;
				}
				newInsts->push_back(i_opAssign);

				Instruction_brCmp* i_brcmp = new Instruction_brCmp();
				i_brcmp->instruction = "br " + lengthResult->name + " :" + lengthResult->name.substr(1) + "trueLabel" + " " + ":" + lengthResult->name.substr(1) + "falseLabel";
				newInsts->push_back(i_brcmp);
					
				// jump for array error
				Instruction_Label* i_lbl = new Instruction_Label();
				i_lbl->instruction = ":" + lengthResult->name.substr(1) + "falseLabel";
				newInsts->push_back(i_lbl);

				Instruction_Call* i_call = new Instruction_Call();
				if (dynamic_cast<Number*>(idx)) {
					i_call->instruction = "call array-error(" + i_load->src->name + "," + encodeArg(f, idx, newInsts) + ")";
				}
				else {
					i_call->instruction = "call array-error(" + i_load->src->name + "," + decodeArg(f, idx, newInsts) + ")";
				}
				// i_call->instruction = "call array-error(0,0)";
				
				/*
				 *  this snippet is used for debugging, call array-error seems to be brokn
				 *
				 */
				// k = (indexNum * 2) + 1;
				// i_call->instruction = "call print(" + to_string(k) + ")";
				// end of debug snippet
				
				newInsts->push_back(i_call);

				// jump for no error
				Instruction_Label* i_lbl8 = new Instruction_Label();
				i_lbl8->instruction = ":" + lengthResult->name.substr(1) + "trueLabel";
				newInsts->push_back(i_lbl8);

				indexNum++;
				i++;
				if(idxDecoded){
					Instruction_opAssignment* encode_idx = new Instruction_opAssignment();
					encode_idx->instruction = idx->name + " <- " + idx->name + " << 1";
					newInsts->push_back(encode_idx);
					Instruction_opAssignment* encode_idx1 = new Instruction_opAssignment();
					encode_idx1->instruction = idx->name + " <- " + idx->name + " + 1";
					newInsts->push_back(encode_idx1);
				}
			}

			if (DEBUG_S) cout << "done with i_load" << endl;
		}
		// otherwise its a store
		else if (i_store) {

			Instruction_opAssignment* i_isZero = new Instruction_opAssignment();
			Arg* isZeroDest = new Arg();
			isZeroDest->name = "%" + legnthVar;
			Number* zero = new Number();
			zero->name = "0";
			zero->num = 0;
			i_isZero->instruction = isZeroDest->name + " <- " + i_store->dst->name + " = " +  zero->name;
			
			Instruction_Declaration* i_dec = new Instruction_Declaration();
			i_dec->instruction =  "int64 " + isZeroDest->name;
	
			// we do not want to redeclare the same variables 
			Arg* var_checker = var_exists(f, isZeroDest);
			if (isZeroDest == var_checker) { 
				newInsts->push_back(i_dec); 
				f->declared_variables.insert(isZeroDest);
			}
			newInsts->push_back(i_isZero);

			// check if array is 0
			Instruction_brCmp* i_brcmp = new Instruction_brCmp();
			string trueLabel = ":" + legnthVar + "trueLabel" + to_string(i_store->num);
			string falseLabel = ":" + legnthVar+ "falseLabel" + to_string(i_store->num);
			
			i_brcmp->instruction = "br " +  isZeroDest->name + " " + trueLabel + " " + falseLabel;
			newInsts->push_back(i_brcmp);

			
			// if it is zero, call array-error
			Instruction_Label* i_lbl = new Instruction_Label();
			i_lbl->instruction = trueLabel;
			newInsts->push_back(i_lbl);

			Instruction_Call* i_call = new Instruction_Call();
			i_call->instruction = "call array-error(0,0)";
			newInsts->push_back(i_call);
			
			// if it isnt zero, continue execution
			Instruction_Label* i_lbl2 = new Instruction_Label();
			i_lbl2->instruction = falseLabel;
			newInsts->push_back(i_lbl2);			


			if (dynamic_cast<Tuple*>(i_store->dst->type)) { return; }

			int indexNum = 0;
			int k = 0;
			for (Arg* idx : i_store->indexes) {
				int idxDecoded = 0;
				if (!dynamic_cast<Number*>(idx)){

					Instruction_opAssignment* decode_idx = new Instruction_opAssignment();
					decode_idx->instruction = idx->name + " <- " + idx->name + " >> 1";
					newInsts->push_back(decode_idx);
					idxDecoded = 1;
				}
				// load the current dimensions length
				Arg* newLength = new Arg();
				newLength->name = "%" + legnthVar;

				// get the length of the current "indexNum" dimension
				Instruction_Length* i_length = new Instruction_Length();
				i_length->instruction = newLength->name + " <- length " + i_store->dst->name + " " + to_string(indexNum);
				newInsts->push_back(i_length);

				/*
				 *  this snippet is used for debugging, call array-error seems to be brokn
				 *
				 */
				Instruction_Call* debug_print = new Instruction_Call();
				debug_print->instruction = "call print(" + newLength->name + ")";
				//newInsts->push_back(debug_print);
				// end of debug snippet

				// decode the length value (its encoded by default)
				Instruction_opAssignment* decode_length = new Instruction_opAssignment();
				decode_length->instruction = newLength->name + " <- " + newLength->name + " >> 1";
				newInsts->push_back(decode_length);

				// create and declare instruction (if necessary) the result of if dimesnion < length 
				Arg* lengthResult = new Arg();
				lengthResult->name = newLength->name + to_string(i);

				Instruction_Declaration* i_dec = new Instruction_Declaration();
				i_dec->instruction =  "int64 " + lengthResult->name;
		
				// we do not want to redeclare the same variables 
				Arg* var_checker = var_exists(f, lengthResult);
				if (lengthResult == var_checker) { 
					newInsts->push_back(i_dec); 
					f->declared_variables.insert(lengthResult);
				}

				// check if less than length 
				Instruction_opAssignment* i_opAssign = new Instruction_opAssignment();
				if (dynamic_cast<Number*>(idx)) {
					//i_opAssign->instruction = lengthResult->name + " <- " + idx->name + " < " + newLength->name;
					 i_opAssign->instruction = lengthResult->name + " <- " + idx->name + " < " + newLength->name;
				}
				else {
					i_opAssign->instruction = lengthResult->name + " <- " + idx->name + " < " + newLength->name;
				}
				newInsts->push_back(i_opAssign);

				Instruction_brCmp* i_brcmp = new Instruction_brCmp();
				i_brcmp->instruction = "br " + lengthResult->name + " :" + lengthResult->name.substr(1) + "trueLabel" + " " + ":" + lengthResult->name.substr(1) + "falseLabel";
				newInsts->push_back(i_brcmp);
					
				// jump for array error
				Instruction_Label* i_lbl = new Instruction_Label();
				i_lbl->instruction = ":" + lengthResult->name.substr(1) + "falseLabel";
				newInsts->push_back(i_lbl);

				Instruction_Call* i_call = new Instruction_Call();
				if (dynamic_cast<Number*>(idx)) {
					i_call->instruction = "call array-error(" + i_store->dst->name + "," + encodeArg(f, idx, newInsts) + ")";
				}
				else {
					i_call->instruction = "call array-error(" + i_store->dst->name + "," + decodeArg(f, idx, newInsts) + ")";
				}
				// i_call->instruction = "call array-error(0,0)";
				
				/*
				 *  this snippet is used for debugging, call array-error seems to be brokn
				 *
				 */
				// k = (indexNum * 2) + 1;
				// i_call->instruction = "call print(" + to_string(k) + ")";
				// end of debug snippet
				
				newInsts->push_back(i_call);

				// jump for no error
				Instruction_Label* i_lbl8 = new Instruction_Label();
				i_lbl8->instruction = ":" + lengthResult->name.substr(1) + "trueLabel";
				newInsts->push_back(i_lbl8);

				indexNum++;
				i++;
				if(idxDecoded){
					Instruction_opAssignment* encode_idx = new Instruction_opAssignment();
					encode_idx->instruction = idx->name + " <- " + idx->name + " << 1";
					newInsts->push_back(encode_idx);
					Instruction_opAssignment* encode_idx1 = new Instruction_opAssignment();
					encode_idx1->instruction = idx->name + " <- " + idx->name + " + 1";
					newInsts->push_back(encode_idx1);
				}
			}

			if (DEBUG_S) cout << "done with i_store" << endl;
		}
		else { return; }
	}

	void parse_instruction(Function* f, Instruction* I, vector<Instruction *>* newInsts) {

		if (Instruction_Declaration* i = dynamic_cast<Instruction_Declaration*>(I)) {
            
			if (Tuple* t = dynamic_cast<Tuple*>(i->type)) {
                				
				i->instruction = "tuple " + i->var->name;

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

				if (DEBUGGING) cout << "found a new array and initing to zero" << endl;
				
				i->instruction = "";

				i->instruction = "int64";
				for (int j = 0; j < type->dims; j++) {
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
				string name = "";
                if (Code* code = dynamic_cast<Code*>(i->type)) { name = "code"; }
                else { name = "int64"; }
                i->instruction = name + " " + i->var->name;
				newInsts->push_back(i);
			}
		}
		else if (Instruction_brCmp* i = dynamic_cast<Instruction_brCmp*>(I)) {
			// decode t 
			
			i->instruction = "br " + decodeArg(f, i->comparitor, newInsts) + " " + i->trueLabel->name + " " + i->falseLabel->name;
		    newInsts->push_back(i);
        }
		else if (Instruction_Length* i = dynamic_cast<Instruction_Length*>(I)) {
			// decode var3 (dimension)
			if(dynamic_cast<Number*>(i->dimension)){
				i->instruction = i->dst->name + " <- length " + i->array->name + " " + i->dimension->name;

			}
			else{
				i->instruction = i->dst->name + " <- length " + i->array->name + " " + decodeArg(f, i->dimension, newInsts);
			}
		    newInsts->push_back(i);
        }
		else if (Instruction_Load* i = dynamic_cast<Instruction_Load*>(I)) {

			// set the begining of the instruction to be the memory check
			check_memory_access(f, i, newInsts);


			i->instruction = "";
			i->instruction = i->dst->name + " <- " + i->src->name;

			for (Arg* idx : i->indexes) {
				if(dynamic_cast<Number*>(idx)){
					i->instruction.append('[' + idx->name + ']');
				}
				else{
					i->instruction.append('[' + decodeArg(f, idx, newInsts) + ']');

				}
			}
		    newInsts->push_back(i);
        }
		else if (Instruction_Store* i = dynamic_cast<Instruction_Store*>(I)) {
			
			// set the begining of the instruction to be the memory check
			check_memory_access(f, i, newInsts);

			i->instruction = "";
			i->instruction = i->dst->name;

			for (Arg* idx : i->indexes) {
				if(dynamic_cast<Number*>(idx)){
					i->instruction.append('[' + idx->name + ']');
				}
				else{
					i->instruction.append('[' + decodeArg(f, idx, newInsts) + ']');

				}
			}

			// size_t found = f->declaration.find(i->src->name);

			// if (found == std::string::npos){
			if (dynamic_cast<Number*>(i->src)) {
				i->instruction.append(" <- " + encodeArg(f, i->src, newInsts));
			}
			else {
		    	i->instruction.append(" <- " + i->src->name);
			}
		    newInsts->push_back(i);
        }
		else if (Instruction_opAssignment* i = dynamic_cast<Instruction_opAssignment*>(I)) {
			// decode arg1, arg2 (t1, t2)

			i->instruction = "";
			Number* num1 = dynamic_cast<Number*> (i->arg1);
			Number* num2 = dynamic_cast<Number*>(i->arg2);

			if(!num1 && !num2){
				i->instruction = i->dst->name + " <- " + decodeArg(f, i->arg1, newInsts) + " " + i->operation->name + " " + decodeArg(f, i->arg2, newInsts);
			}
			else if(!num1){
				i->instruction = i->dst->name + " <- " + decodeArg(f, i->arg1, newInsts) + " " + i->operation->name + " " + i->arg2->name;
			}
			else if(!num2){
				i->instruction = i->dst->name + " <- " + i->arg1->name + " " + i->operation->name + " " + decodeArg(f, i->arg2, newInsts);
			}
			else{
				i->instruction = i->dst->name + " <- " + i->arg1->name + " " + i->operation->name + " " + i->arg2->name;
			}

		    newInsts->push_back(i);
			encodeArg(f, i->dst, newInsts);
		}    
        else if (Instruction_TupleInit* i_tuple = dynamic_cast<Instruction_TupleInit*>(I)) {
        	i_tuple->instruction = i_tuple->dst->name + " <- new Tuple(" + encodeArg(f, i_tuple->src[0], newInsts) + ")";
        	newInsts->push_back(i_tuple);
        } 
        else if (Instruction_ArrayInit* i_array = dynamic_cast<Instruction_ArrayInit*>(I)) {
        	i_array->instruction = i_array->dst->name + " <- new Array(";
        	reverse(i_array->src.begin(), i_array->src.end());
        	for (Arg* idx : i_array->src) { i_array->instruction.append(encodeArg(f, idx, newInsts) + ","); }
        	i_array->instruction.append(")"); 
        	newInsts->push_back(i_array);
        }
        else if (Instruction_Return* i_ret = dynamic_cast<Instruction_Return*>(I)) {
        	i_ret->instruction = "return";
        	newInsts->push_back(i_ret);
        }
        else if (Instruction_CallAssign* i_callAssign = dynamic_cast<Instruction_CallAssign*>(I)) {
        	if(i_callAssign->callee->name[0] == '%'){
        		i_callAssign->instruction = i_callAssign->dst->name + " <- call " + i_callAssign->callee->name + "(";
        	}
        	else{
        		i_callAssign->instruction = i_callAssign->dst->name + " <- call :" + i_callAssign->callee->name + "(";
        	}
    		for(int i = 0; i < i_callAssign->parameters.size(); i++){
    			Number* num = dynamic_cast<Number*>(i_callAssign->parameters[i]);
    			if (num) {  
    				i_callAssign->instruction.append(encodeArg(f, i_callAssign->parameters[i], newInsts) + ", ");
    			} else {
    				i_callAssign->instruction.append(i_callAssign->parameters[i]->name + ", ");
    			}
    		}
    		i_callAssign->instruction.append(")");
    		newInsts->push_back(i_callAssign);
        }
        else if (Instruction_Call* i_call = dynamic_cast<Instruction_Call*>(I)){
        	if(PA* isPA = dynamic_cast<PA*>(i_call->callee)){
        		i_call->instruction = "call " + i_call->callee->name + "(";
        	}
        	else{
        		if(i_callAssign->callee->name[0] == '%'){
        			i_callAssign->instruction = i_callAssign->dst->name + " <- call " + i_callAssign->callee->name + "(";
        		}
        		else{
        			i_callAssign->instruction = i_callAssign->dst->name + " <- call :" + i_callAssign->callee->name + "(";
        		}
        	}
        		for(int i = 0; i < i_call->parameters.size(); i++){
        			Int64* int64 = dynamic_cast<Int64*>(i_call->parameters[i]->type);
        			Number* num = dynamic_cast<Number*>(i_call->parameters[i]);
        			if (num) {  
        				i_call->instruction.append(encodeArg(f, i_call->parameters[i], newInsts) + ", ");
        			} else {
        				i_call->instruction.append(i_call->parameters[i]->name + ", ");
        			}
        		}
        		i_call->instruction.append(")");
        		newInsts->push_back(i_call);
    //     	else{
				// i_call->instruction = "call :" + i_call->callee->name + "(";
    //     		for(int i = 0; i < i_call->parameters.size(); i++){
    //     			i_call->instruction.append(i_call->parameters[i]->name + ", ");
    //     		}
    //     		i_call->instruction.append(")");
    //     		newInsts->push_back(i_call);
    //     	}
        }
        else if (Instruction_Assignment* i_assign = dynamic_cast<Instruction_Assignment*>(I)) {
        	newInsts->push_back(i_assign);
        	encodeArg(f, i_assign->dst, newInsts);
        }
		else {
			// instruction remains the same
			newInsts->push_back(I);
		}
		return;
	}

	void generate_basic_blocks(Function* f, vector<Instruction *>* newInsts) {
		Instruction* inst = f->instructions[0];
		bool startBB = true;
		string uniqueLabel = ":uniqueLabelDavidAndBrian";
		int i = 0;
		
		if (DEBUGGING) cout << "length of functions instructions are: " << f->instructions.size() << endl;
        
        while (inst) {

            Instruction_Label* i_lbl = dynamic_cast<Instruction_Label*>(inst);
			if (startBB) {
				if (!i_lbl) {
					if(DEBUGGING) cout << "Found the beginning of a basic block without a label\n The beginning inst is: " << inst->instruction << endl;
                    Instruction_Label* new_label = new Instruction_Label();
					new_label->instruction = uniqueLabel + to_string(i);
					newInsts->push_back(new_label);
				}
				startBB = false;
			}
			else if (i_lbl) {
				if(DEBUGGING) cout << "Found a label: " << i_lbl->instruction << "\n";
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
     			if (DEBUGGING) cout << "found a terminator -- setting startBB flag to true" << endl;
				startBB = true;
			}

			i++;
      
			if (i > f->instructions.size() - 1) { inst = NULL; }
			//if (DEBUGGING) cout << " i is " << i << " and inst size is " << f->instructions.size() << endl;
		    else { inst = f->instructions[i]; }
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
        fs.open("prog.IR", std::fstream::in | std::fstream::out | std::fstream::app);
        
       
        for (Function* f : p.functions) {
        	
        	if (DEBUGGING) cout << "Function: " << f->name->name << " has " << f->instructions.size() << " instructions" << endl;
        	if (DEBUGGING) cout << "--> last instruction is: " << f->instructions.back()->instruction << endl;
        	
        	number_instructions(f);
        
            fs << "define " << f->returnType->name << " :" << f->declaration;

            
            fs << "{\n";
            vector<Instruction *> newInsts = {};
        	
           
            for (Instruction* I : f->instructions) {
            	if (DEBUGGING) cout << "parsing instruction: " << I->instruction << endl;
        		parse_instruction(f, I, &newInsts);
        	}

            f->instructions = newInsts;

            if (DEBUGGING) {
            	cout << "Before BB generation, the instructions are as follows:" << endl;
            	for (Instruction* inst : f->instructions) {  cout << inst->instruction << endl; }
            }
        
            newInsts = {};
        	generate_basic_blocks(f, &newInsts);
            
            if(DEBUGGING) cout << "last instruction of basic block is: " << newInsts.back()->instruction << endl;
                	
        	f->instructions = newInsts;
        	for (Instruction* I : f->instructions) {
        		fs << "\t" << I->instruction << endl;
        	}
            
        	fs << "}\n";
        }
        
        fs.close();
        if (DEBUGGING) cout << "file closed, LA code generator completed" << endl;
      }        
}

 
