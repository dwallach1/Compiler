#pragma once

#include <LA.h>

namespace LA {
  Program parse_file (char *fileName);
  Arg* findVariable(Function* f, std::string name);
}
