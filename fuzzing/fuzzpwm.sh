#! /bin/bash

if [ $# -eq 0 ]; then
  echo Usage: ./fuzzpwm.sh input_file1 ... input_filen
  exit 0
fi
N=0
crashes=0
mkdir -p coverage outputs
echo --- Cleaning up previous execution files ...
rm -f ../*.gcda coverage/* outputs/*
echo --- Fuzzing ...
for f in $*
do
  ofile="outputs/$(basename $f).out"
  efile="outputs/$(basename $f).err"
  echo Executing pwm for input file $f
  ../pwm $f > $ofile 2> $efile
  if [ $? -ne 0 ]; then
    echo Possible program crash for $f - see $ofile for more information
    crashes=$((crashes + 1))
  fi
  N=$((N+1))
done
echo --- Done ... $crashes possible program crashes in $N executions
echo --- Deriving coverage information ...
cd ..; gcov -b *.c; cd fuzzing; mv ../*gcov coverage; ls coverage/*.gcov

