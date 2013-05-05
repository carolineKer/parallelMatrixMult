#!/usr/bin/env bash
set -e

N=1041
RES_DIR=result_$N

make
mkdir -p $RES_DIR

for P in 1 4 9 16 25 36
do
    echo "$P processors"
    mpirun -np $P ./matrix_mult.exe $RES_DIR $N $N $N
    echo "Done"
done

    ./plot_speedup.py $RES_DIR


