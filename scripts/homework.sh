#!/bin/bash

dirName="322_submission" ;
origDir=`pwd` ;

# Copy all files to an empty directory
if test -d ../$dirName ; then
  echo "ERROR: Please delete ../$dirName" ;
  exit 1 ;
fi
mkdir ../$dirName ;
cd ../$dirName ;
cp -r "${origDir}"/* ./ ;

# Remove unnecessary files
rm -r lib Makefile scripts bin ;
rm -r */tests ;
rm -fr */obj ;
rm -fr */bin ;
rm -r */scripts ;
rm -rf `find ./ -name .DS_Store`
rm  */Makefile ;
rm -f `find ./ -iname *.swp`
for i in `ls` ; do
  if ! test -d $i ; then
    continue ;
  fi
  pushd ./ ;
  cd $i ;
  for j in `ls` ; do
    if test -d $j ; then
      continue ;
    fi
    rm -f $j ;
  done
  popd ;
done

# Create the package
tar cfj ../${dirName}.tar.bz2 ./ ;
cd ../ ;
mv ${dirName}.tar.bz2 "${origDir}"/ ;
rm -r ${dirName} ;
