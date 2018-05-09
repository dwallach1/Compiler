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


#define DEBUGGING 0
#define DEBUG_S 0

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
  int32_t optLevel = 0;

  /* Check the input.
   */
  //Utils::verbose = false;
  if( argc < 2 ) {
    print_help(argv[0]);
    return 1;
  }
  int32_t opt;
  while ((opt = getopt(argc, argv, "vg:O:sli:")) != -1) {
    switch (opt){
      case 'l':
        break ;
      case 'i':
        break;
      case 's':
        break ;

      case 'O':
        optLevel = strtoul(optarg, NULL, 0);
        if(DEBUGGING) printf("Setting optLevel to be %d\n", optLevel);

        break ;

      case 'g':
        enable_code_generator = (strtoul(optarg, NULL, 0) == 0) ? false : true ;
        if(DEBUGGING) printf("Enabling code generator");

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
  L3::Program p;
  p = L3::parse_file(argv[optind]);
    

  /*
   * Generate the code.
   */
  if (enable_code_generator){
    if(DEBUGGING) std::cout << "Linking calls to functions\n";
    renameAllLabels(&p);
    linkCallsToFunctions(&p);
    if(DEBUGGING) std::cout << "Generating new code\n";
    L3::L3_generate_code(p);
  }

  return 0;
}
