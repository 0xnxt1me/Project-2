#! /bin/bash

if [ $# -ne 1 ]; then
  echo Usage: $0 file
  exit 1
fi

f=$(dirname $1)/$(basename --suffix=.s $1)

echo Compiling assembly code in $f.s

as $f.s -o $f.elf

if [ $? -ne 0 ]; then
  echo Error compiling assembly code
  exit 1
fi

echo Extracting shell code to $f.bin
objcopy -O binary --only-section=.text $f.elf $f.bin

rm -f $f.elf

echo Hex dump
hexdump  -e "$(stat -c '%s' $f.bin)/1 \"%02x\"" -e '"\n"' $f.bin

