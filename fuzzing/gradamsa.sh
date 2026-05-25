#! /bin/bash

echo Transforming given files using radamsa
for file in $@; do
	./radamsa -s $(wc -c $file) $file > $file.rad
  echo $file.rad generated
done

