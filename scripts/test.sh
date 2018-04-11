#!/bin/bash

if test $# -lt 2 ; then
  echo "USAGE: `basename $0` EXTENSION_FILE COMPILER" ;
  exit 1;
fi
extFile=$1 ;
compiler=$2 ;

passed=0 ;
failed=0 ;
cd tests ; 
for i in *.${extFile} ; do

  # If the output already exists, skip the current test
  if ! test -f ${i}.out ; then
    continue ;
  fi
  echo $i ;

  # Generate the binary
  pushd ./ ;
  cd ../ ;
  didSucceed=0 ;
  ./${compiler} tests/${i} ;
  if test $? -eq 0 ; then
    ./a.out &> tests/${i}.out.tmp ;
    cmp tests/${i}.out.tmp tests/${i}.out ;
    if test $? -eq 0 ; then
      didSucceed=1 ;
    fi
  fi
  if test $didSucceed == "1" ; then
    echo "  Passed" ;
    let passed=$passed+1 ;
  else
    echo "  Failed" ;
    let failed=$failed+1 ;
  fi
  popd ; 
done
let total=$passed+$failed ;

echo "########## SUMMARY" ;
echo "Test passed: $passed out of $total"
