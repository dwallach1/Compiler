#pragma once

#include <L2.h>

namespace L2{

  void generate_code(Program p);
  L2::DataFlowResult computeLivenessAnalysis(Program p, Function f);


}
