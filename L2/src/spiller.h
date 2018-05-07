#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <iterator>
#include <regex>
#include <stdlib.h>
#include <algorithm>

#define DEBUGGING 0
#define DEBUG_SP 0



namespace L2{

	//Deprecated function
	L2::Variable* findVarInFunction(std::string name, Function* f){
        for(Variable* var : f->interferenceGraph->variables){
        	if(DEBUGGING) printf("Comparing %s and %s\n", name.c_str(), var->name.c_str());
            if(name == var->name){
                return var;
            }
        }
        if(DEBUGGING) printf("Could not find the var %s\n", name.c_str());
        return NULL;
    }

    //Deprecated function
	void generateUsesAndVars(L2::Function* f){
		if(DEBUGGING) printf("Beginning generateInterferenceGraph\n");
        L2::InterferenceGraph* iG = new L2::InterferenceGraph();
        f->interferenceGraph = iG;
        //iG->variables = {};
        if(DEBUGGING) printf("instatiateVariables Time\n");
        instatiateVariables(f, iG);
		for(Instruction* I : f->instructions){
			for(int i = 0; i < I->arguments.size(); i++){
				L2::Variable* V = findCorrespondingVar(I->arguments[i]->name, iG);
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
	
	//Deprecated function
	void generateInstNums(L2::Function* f){
		int i = 0;
		linkInstructionPointers(f);
		for(Instruction* I : f->instructions){
			I->instNum = i;
			i++;
			//Handling inc/dec because we were doing bad programming before
		}

	}

	//Removes the spaces in increment/decrement instructions
	void removeIncDecSpaces(L2::Function* f){
		for(Instruction* I : f->instructions){
			if(I->type == INC_DEC){
				I->instruction.erase(remove(I->instruction.begin(), I->instruction.end(), ' '), I->instruction.end());
			}
		}
	}

	//Prints a function after spilling a variable
	void printNewSpill(Function* f){

		printf("(%s\n", f->name.c_str());
		printf("\t%ld %ld\n", f->arguments, f->locals);

		for (Instruction* I : f->instructions) {
			printf("\t%s\n", I->instruction.c_str());
		}

		printf(")\n");
	}

	//Will insert a load instruction before the idx instruction
	void insertLoad(Function* f, std::string replacementString, std::vector<Instruction*>::iterator idx, int stackLoc) {
		Instruction* newInst = new Instruction();
		//Load inst
		newInst->type = LOAD;
		newInst->instruction = replacementString + " <- "+ "mem rsp " + std::to_string(stackLoc);

		if (DEBUGGING) printf("inserting load: %s\n", newInst->instruction.c_str());

		L2::Arg* arg = new L2::Arg();
		arg->name = replacementString;
		arg->type = MEM;

		L2::Arg* arg2 = new L2::Arg();
		arg2->name = "mem rsp " + std::to_string(stackLoc);
		arg2->type = MEM;

		newInst->arguments.push_back(arg);
		newInst->arguments.push_back(arg2);
		newInst->operation.push_back("<-");

		f->instructions.insert(idx, newInst);
	}

	//Same as insertLoad, but with a store
	void insertStore(Function* f, std::string replacementString, std::vector<Instruction*>::iterator idx, int stackLoc) {
		Instruction* newInst = new Instruction();
		//Store inst
		newInst->instruction = "mem rsp " + std::to_string(stackLoc) + " <- "+ replacementString;
		newInst->type = STORE;

		if (DEBUGGING) printf("inserting store: %s\n", newInst->instruction.c_str());

		L2::Arg* arg = new L2::Arg();
		arg->name = "mem rsp " + std::to_string(stackLoc);
		arg->type = MEM;

		L2::Arg* arg2 = new L2::Arg();
		arg2->name = replacementString;
		arg2->type = MEM;

		newInst->arguments.push_back(arg);
		newInst->arguments.push_back(arg2);
		newInst->operation.push_back("<-");

		f->instructions.insert(idx, newInst);
	}

	//Deprecated Function
	void replaceInstructionVarWithVar(Instruction* I, std::string varToReplace, std::string replacementString){
		std::vector<std::string> result;
		std::string newInst = "";
		std::istringstream iss(I->instruction);		
		for(std::string s; iss >> s;){
			result.push_back(s);
		}
		int i = 0;
		for(std::string s : result){
			if(s == varToReplace){
				result[i] = varToReplace;
			}
			i++;
		}
		for(std::string s : result){
			newInst = newInst + s + " ";
		}
		I->instruction = newInst;
	}


	//This function will spill a variable
	void spillVar(L2::Function* f){
		
		if (DEBUGGING) printf("doing preliminary functions\n");
		removeIncDecSpaces(f);
		generateUsesAndVars(f);
		linkInstructionPointers(f);
		
		Variable* V = findVarInFunction(f->toSpill, f);
		
		int i = 0;
		if(V){ 
			if (DEBUGGING) printf("found var %s\n", V->name.c_str());
			f->locals++;
			int stackLoc = (f->locals * 8) - 8;
			// if (f->arguments > 6) {
			// 	stackLoc += (f->arguments - 6) * 8;
			// }
			int numUses = V->uses.size();

		
			while(V->uses.size() > 0){

				std::vector<Instruction*>::iterator iter = V->uses.begin();
				Instruction* I = *iter;

				int j = 0;
				std::string replacementString = f->replaceSpill + std::to_string(i);

				if (DEBUG_SP) printf("replacementString is now %s\n", replacementString.c_str());

				//replaceInstructionVarWithVar(I, f->toSpill, replacementString);

				I->instruction = std::regex_replace(I->instruction, std::regex(f->toSpill), replacementString);

				for(Arg* curArg : I->arguments){
					if(curArg->name == f->toSpill){
						I->arguments[j]->name = replacementString;
						curArg->name = replacementString;
						if (DEBUG_SP) printf("updated args w replacementString\n");
					}

					//I->instruction = std::regex_replace(I->instruction, std::regex(f->toSpill), replacementString);



					if (DEBUGGING) printf("updated instruction w replacementString\n");
					j++;
				}

				// for(Variable* replaceV : f->interferenceGraph->variables) {
				// 	if (replaceV->name == f->toSpill) {
				// 		replaceV->name = replacementString;
				// 	}
				// 	std::set<std::string>::iterator it;
				// 	it = replaceV->edges.begin();
				// 	for (std::string E : replaceV->edges) {

						
				// 		if (E == f->toSpill) {
				// 			it = replaceV->edges.find(f->toSpill);
				// 			replaceV->edges.erase(it);
				// 			replaceV->edges.insert(replacementString);
				// 		}
				// 	}
				// }

				if (DEBUGGING) printf("done w  arguments\n");
				std::vector<Instruction*>::iterator iter2;
				iter2 = f->instructions.begin();
				
				for(std::string g : I->gen){
					if(g == V->name){
						if (DEBUG_SP) printf("attempting load\n");
						insertLoad(f, replacementString, iter2 + I->instNum, stackLoc);
						linkInstructionPointers(f);
						iter2 = f->instructions.begin();
						if (DEBUG_SP) printf("inserted load replacementString\n");
						break;
					}
				}

				if (DEBUGGING) printf("passed through genset\n");
				iter2 = f->instructions.begin();
				if (DEBUGGING) printf("attempting kill[0]\n");
				if(I->type == LOAD || I->type == ASSIGN || I->type == AOP || I->type == INC_DEC || I->type == CMP_ASSIGN || I->type == LEA){
					if (DEBUGGING) printf("special instruction\n");
					if(I->kill.size() && I->kill[0].compare(V->name) == 0){
						if (DEBUG_SP) printf("attempting store\n");
						insertStore(f, replacementString, iter2 + I->instNum + 1, stackLoc);
						linkInstructionPointers(f);
						iter2 = f->instructions.begin();
						if (DEBUG_SP) printf("inserted store w replacementString\n");
					}
					else {
						if (DEBUGGING) printf("kill[0] not equal\n");
					}
				}
				if (DEBUGGING) printf("passed thorugh kill[0]\n");


				// pop off front use
				int k = 0;
				std::vector<Instruction*> newUses = {};
				for(Instruction* I : V->uses){
					if (k) {   newUses.push_back(I);   }
					else   {   k++;				 	   }
				}
				
				V->uses = newUses;
				linkInstructionPointers(f);
				i++;
			}
		}
		else{
			if(DEBUGGING) printf("Not a valid VAR\n");
		}

	}


}
