#!/usr/bin/env bash
set -e

N=5000
RES_DIR=result_$N

make
mkdir -p $RES_DIR

for P in 1 4 9 16 25 
do
    echo "$P processors"
    esubmit -n 3 -t 1 ./mympirun -np $P ./matrix_mult.exe $RES_DIR $N $N $N
    echo "Done"
done
