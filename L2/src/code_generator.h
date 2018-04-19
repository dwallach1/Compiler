#pragma once

#include <L2.h>

namespace L2{

  void L2_generate_code(Program p);
  L2::DataFlowResult* computeLivenessAnalysis(Program p, Function f);
  void generatePrevInstPointers(L2::Function f);


}
