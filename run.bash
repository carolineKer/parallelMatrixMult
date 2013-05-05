#!/usr/bin/env bash
set -e

I=1000
J=1000
K=1000
RES_DIR=result_$I_$K_$J

make
mkdir -p $RES_DIR

for P in 1 4 9 16 25 36
do
    mpirun -np $P ./matrix_mult.exe $RES_DIR $I $K $J
done

    ./plot_speedup.py $RES_DIR


