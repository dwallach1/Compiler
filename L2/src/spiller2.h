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
#define DEBUG_S 0


namespace L2{

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

	void generateUsesAndVars(L2::Function* f){
		if(DEBUGGING) printf("Beginning generateInterferenceGraph\n");
        L2::InterferenceGraph* iG = new L2::InterferenceGraph();
        f->interferenceGraph = iG;
        //iG->variables = {};
        if(DEBUGGING) printf("instatiateVariables Time\n");
        instatiateVariables(f, iG);
		for(Instruction* I : f->instructions){
			for(int i = 0; i < I->registers.size(); i++){
				L2::Variable* V = findCorrespondingVar(I->registers[i], iG);
				//Found a variable
				if(V){
					//varFound = true;
					V->uses.push_back(I);
					if(I->type == 1){ 
						if(I->registers[0] == I->registers[1]){
							break;
						}
					}
				}
			}
		}
	}
	
	void generateInstNums(L2::Function* f){
		int i = 0;
		generatePrevInstPointers(f);
		for(Instruction* I : f->instructions){
			I->instNum = i;
			i++;
			//Handling inc/dec because we were doing bad programming before
		}

	}

	void removeIncDecSpaces(L2::Function* f){
		for(Instruction* I : f->instructions){
			if(I->type == 12){
				if(DEBUG_S) printf("Trying to remove spaces from inst: %s\n", I->instruction.c_str());
				I->instruction.erase(remove(I->instruction.begin(), I->instruction.end(), ' '), I->instruction.end());
			}
		}
	}

	bool callAhead(Instruction* I, Function* f){
		bool found = false;
		for(Instruction* Ii : f->instructions){
			if(Ii == I){
				found = true;
				continue;
			}
			if(found){
				if(Ii->type == 8){
					return true;
				}
				for(std::string compStr : Ii->registers){
					if(compStr == f->toSpill){
						return false;
					}
				}
			}
		}
		return false;
	}

	void printNewSpill(Function* f){

		printf("(%s\n", f->name.c_str());
		printf("\t%ld %ld\n", f->arguments, f->locals);

		for (Instruction* I : f->instructions) {
			printf("\t%s\n", I->instruction.c_str());
		}

		printf(")\n");
	}

	void spillVar(L2::Function* f){
		//Is it a cJump Goto or call
		bool cJGC;
		removeIncDecSpaces(f);
		if(DEBUGGING){
			printf("Spilling the variable %s\n", f->toSpill.c_str());
		}
		
		if(DEBUGGING) printf("Generating the Uses for every var\n");
		generateUsesAndVars(f);
		generateInstNums(f);
		if(DEBUGGING) printf("Finding var in the function\n");
		Variable* V = findVarInFunction(f->toSpill, f);
		int i = 0;
		if(V){ 
			if(DEBUG_S) printf("The Variable is a valid variable\n");
			f->locals++;
			if(DEBUG_S) printf("Incrementing F->locals\n");
			int stackLoc = (f->locals * 8) - 8;
			int numUses = V->uses.size();

			if(DEBUG_S){
				printf("Printing Uses in order\n");
				for(Instruction* str : V->uses){
					printf("%s\n", str->instruction.c_str());
				}
			} 
			while(V->uses.size() > 0){
				if(DEBUG_S) printf("The variable currently has %ld uses\n", V->uses.size());
				std::vector<Instruction*>::iterator iter = V->uses.begin();
				Instruction* I = *iter;
				cJGC = I->type == 8 || I->type == 5 || I->type == 6;
				if(DEBUG_S) printf("Dealing with instruction: %s\n", I->instruction.c_str());
				int j = 0;
				std::string replacementString = f->replaceSpill + std::to_string(i);
				if(DEBUG_S) printf("replacemnet string is: %s", replacementString.c_str());
				for(std::string curStr : I->registers){
					if(curStr == f->toSpill){
						I->registers[j] = replacementString;
					}
					
					I->instruction = std::regex_replace(I->instruction, std::regex(f->toSpill), replacementString);
					
					j++;
				}
				if(DEBUG_S) printf("The instruction is now: %s\n", I->instruction.c_str());
				std::vector<Instruction*>::iterator iter2;
				iter2 = f->instructions.begin();
				bool callInstAhead = callAhead(I, f);
				//Last inst
				if(i == numUses-1 && I->type != 2){
					Instruction* newInst = new Instruction();
					//Load inst
					newInst->type = 2;
					newInst->instruction = replacementString + " <- "+ "mem rsp " + std::to_string(stackLoc);
					newInst->registers.push_back(replacementString);
					newInst->registers.push_back("mem rsp " + std::to_string(stackLoc));
					newInst->operation.push_back("<-");
					if(DEBUG_S) printf("Adding load inst at location %ld\n", I->instNum);
					f->instructions.insert(iter2 + I->instNum, newInst);
					if(!callInstAhead && !cJGC ){
						generateInstNums(f);
						iter2 = f->instructions.begin();
						Instruction* newInst1 = new Instruction();
						newInst1->type = 3;
						newInst1->instruction = "mem rsp " + std::to_string(stackLoc) + " <- "+ replacementString;
						newInst1->registers.push_back("mem rsp " + std::to_string(stackLoc));
						newInst1->registers.push_back(replacementString);
						newInst1->operation.push_back("<-");
						if(DEBUG_S) printf("Adding store inst at location %ld\n",  I->instNum + 1);
						f->instructions.insert(iter2 + I->instNum + 1, newInst1);
					}
				}
				//First Inst
				else if(i == 0 && I->type != 3 && !cJGC){
					Instruction* newInst = new Instruction();
					//Store inst
					newInst->type = 3;
					newInst->instruction = "mem rsp " + std::to_string(stackLoc) + " <- "+ replacementString;
					newInst->registers.push_back("mem rsp " + std::to_string(stackLoc));
					newInst->registers.push_back(replacementString);
					newInst->operation.push_back("<-");
					if(DEBUG_S) printf("Adding store inst at location %ld\n",  I->instNum + 1);
					f->instructions.insert(iter2 + I->instNum + 1, newInst);
				}

				//Middle case
				else{
				
					if(I->type !=3 && !cJGC){ 
						Instruction* newInst1 = new Instruction();
						//Store inst
						newInst1->type = 3;
						newInst1->instruction = "mem rsp " + std::to_string(stackLoc) + " <- "+ replacementString;
						newInst1->registers.push_back("mem rsp " + std::to_string(stackLoc));
						newInst1->registers.push_back(replacementString);
						newInst1->operation.push_back("<-");
						if(DEBUG_S) printf("Adding store inst at location %ld\n",  I->instNum + 1);
						f->instructions.insert(iter2 + I->instNum + 1, newInst1);
					}
					

					generateInstNums(f);

					iter2 = f->instructions.begin();

					if(I->type != 2){ 
						Instruction* newInst = new Instruction();
						//Load inst
						newInst->type = 2;
						newInst->instruction = replacementString + " <- "+ "mem rsp " + std::to_string(stackLoc);
						newInst->registers.push_back(replacementString);
						newInst->registers.push_back("mem rsp " + std::to_string(stackLoc));
						newInst->operation.push_back("<-");
						if(DEBUG_S) printf("Adding load inst at location %ld\n", I->instNum);
						f->instructions.insert(iter2 + I->instNum, newInst);
					}
				}
				int k = 0;
				std::vector<Instruction*> newUses = {};
				for(Instruction* I : V->uses){
					if (k){
						newUses.push_back(I);
					}
					else{
						k++;
					}
				}
				
				V->uses = newUses;

				generateInstNums(f);
				i++;
			}
		}
		else{
			if(DEBUGGING) printf("Not a valid VAR\n");
		}

	}


}
