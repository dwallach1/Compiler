#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <cctype>
#include <cstdlib>
#include <stdint.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <stdlib.h>
#include <code_generator.h>
#define DEBUGGING 0
#define DEBUG_S 0

using namespace std;

namespace LA {


    void LA_generate_code(Program p) {
        
        // set up file stream
        std::fstream fs;
        fs.open("prog.LA", std::fstream::in | std::fstream::out | std::fstream::app);
        
        fs.close();
      }        
}

 
