#pragma once

#include <L3.h>


namespace L3{

  void L3_generate_code(Program p);
  void updateArgumentsAndLocals(Function* f);
  std::string generate_unique_var(Function* f);
  std::string convert_function(Function* f);
  std::string convert_instruction(Function* f, Instruction* I);
  void linkCallsToFunctions(Program* p);
  void gatherAllSpecialCalls(Program* p);
  void addFunctionArgumentLoadAndStore(Program* p);
  void generateContextBlocks(Function* f);
  void generateTrees(ContextBlock* cb, std::vector<Tree *>* trees);
  void renameAllLabels(Program* p);
  bool isFunctionLabel(std::string labelName, Program* p);
  //void replaceKeywordLabels(char* fileName);
  void mergeTrees(std::vector<Tree *>* trees);
  void tileTree(Tree* tree);

}
