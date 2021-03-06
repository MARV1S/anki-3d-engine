#!/bin/bash
set -v

PROFILE=1 ANKI_DATA_PATH=$PWD/assets valgrind --tool=callgrind --callgrind-out-file=callgrind.out.tmp --collect-jumps=yes --branch-sim=yes --cache-sim=yes --simulate-wb=yes --cacheuse=yes --simulate-hwpref=yes --separate-threads=yes --I1=32768,2,64 --D1=32768,2,64 --LL=1048576,16,64 $1
