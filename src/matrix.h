#ifndef __MATRIX__H
#define __MATRIX__H

#include "parallel_ctxt.h"
typedef struct _Matrix
{
    double * ptr;
    int I;
    int J;
} Matrix;

Matrix * read_matrix(char * filename);
Matrix * create_simple_matrix(PAR_CTXT * parCtxt);
Matrix * create_id_matrix(PAR_CTXT * parCtxt);
Matrix * alloc_block_matrix(PAR_CTXT * parCtxt);
void matrix_mult_add(Matrix * a, Matrix *b, Matrix *c);
#endif
