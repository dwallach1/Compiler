#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <set>
#include <iterator>
#include <iostream>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <unistd.h>
#include <iostream>

#include <parser.h>
#include <code_generator.h>

using namespace std;

void print_help (char *progName){
  std::cerr << "Usage: " << progName << " [-v] [-g 0|1] [-O 0|1|2] [-s] [-l 1|2] SOURCE" << std::endl;
  return ;
}

int main(
  int argc, 
  char **argv
  ){
  bool enable_code_generator = true;
  bool spill_only = false;
  int32_t liveness_only = 0;
  int32_t optLevel = 0;

  /* Check the input.
   */
  Utils::verbose = false;
  if( argc < 2 ) {
    print_help(argv[0]);
    return 1;
  }
  int32_t opt;
  while ((opt = getopt(argc, argv, "vg:O:sl:")) != -1) {
    switch (opt){
      case 'l':
        liveness_only = strtoul(optarg, NULL, 0);
        break ;

      case 's':
        spill_only = true;
        break ;

      case 'O':
        optLevel = strtoul(optarg, NULL, 0);
        break ;

      case 'g':
        enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true ;
        break ;

      case 'v':
      //TODO
        break ;

      default:
        print_help(argv[0]);
        return 1;
    }
  }

  /*
   * Parse the input file.
   */
  L2::Program p;
  if (spill_only){

    /* 
     * Parse an L2 function and the spill arguments.
     */
    p = L2::parse_spill_file(argv[optind]);
 
  } else if (liveness_only){

    /*
     * Parse an L2 function.
     */
    p = L2::parse_function_file(argv[optind]);

  } else {

    /* 
     * Parse the L2 program.
     */
    p = L2::parse_file(argv[optind]);
  }

  /*
   * Special cases.
   */
  if (spill_only){

    /*
     * Spill.
     */
    L2::REG_spill(p, p.functions[0], p.spill.varToSpill, p.spill.prefixSpilledVars);
  }

  if (liveness_only){
    for (auto f : p.functions){

      /*
       * Compute the liveness analysis.
       */
      L2::DataFlowResult *liveness = L2::computeLivenessAnalysis(p, f);

      /*
       * Print the liveness.
       */
      cout << liveness->toString(f, liveness_only > 1) << endl;

      /*
       * Free the memory.
       */
      delete liveness;
    }

    return 0;
  }

  /*
   * Generate the code.
   */
  if (enable_code_generator){
    L2::L2_generate_code(p);
  }

  return 0;
}