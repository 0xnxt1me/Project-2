#! /bin/bash
echo Running blab - see inputs dir for generated files
mkdir -p inputs
./blab -n 10 -s 231 -o inputs/blab%n.txt grammar.blab
