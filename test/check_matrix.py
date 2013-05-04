#!/usr/bin/python
import sys
import os
import numpy as np

def usage():
    print "%s matrixA matrixB matrixC_dir"%sys.argv[0]
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
    dir = sys.argv[3]

    i = A.shape[0]
    j = B.shape[1]
    
    C = np.matrix(np.zeros((i,j)))
    matrix_dict = {}

    max_p = -1
    max_q = -1
    files = os.listdir(dir)
    for filename in files:
        if "MatrixC" in filename:
            l = filename.split("_")
            p = int(l[1])
            q = int(l[2])
            if p > max_p:
                p = max_p
            if q >max_q:
                q = max_q
            filename = os.path.join(dir, filename)
            matrix_dict[(p,q)] = matrix_from_file(filename)

    x = 0
    y = 0

    for p in range(0,max_p+1):
        delta = matrix_dict[(p,0)].shape[1]
        for q in range(0, max_q+1):
            print (p, q)
            assert(delta == matrix_dict[(p,q)].shape[1])
            m = matrix_dict[(p,q)]
            C[x:x+m.shape[0],y:y+m.shape[1]]
            x += m.shape[0]
        y+= delta

    print C

    D = A*B
    if (C == D).all():
        print "OK"
    else:
        print "KO"


