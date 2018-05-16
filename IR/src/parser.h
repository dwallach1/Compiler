#pragma once

#include <IR.h>

namespace IR{
  Program parse_file (char *fileName);
  Arg* findVariable(Function* f, std::string name)
}
