#ifndef __MATRIX__H
#define __MATRIX__H

#include "parallel_ctxt.h"
typedef struct _Matrix
{
    double ** ptr;
    int I;
    int J;
} Matrix;

Matrix * read_matrix(char * filename);
Matrix * create_simple_matrix(PAR_CTXT * parCtxt);
#endif
