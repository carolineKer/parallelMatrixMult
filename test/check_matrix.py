#!/usr/bin/python
import sys
import numpy as np

def usage():
    print "%s matrixA matrixB matrixC"%sys.argv[0]
    print "Check if A*B == C"

def matrix_from_file(path):
    with open(path, "r") as f:
        l = f.readlines()[0].replace("\n","")
        return np.matrix(l)

if __name__=="__main__":
    if len(sys.argv) != 4:
        usage()
        sys.exit(-1)

    A = matrix_from_file(sys.argv[1])
    B = matrix_from_file(sys.argv[2])
    C = matrix_from_file(sys.argv[3])

    D = A*B
    if (C == D).all():
        print "OK"
    else:
        print "KO"



