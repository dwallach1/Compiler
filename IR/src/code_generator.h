#pragma once

#include <IR.h>


namespace IR {

  void IR_generate_code(Program p);
  void orderBasicBlocks(Function* f);
  BasicBlock* findBasicBlock(Function* f, std::string label);
}
