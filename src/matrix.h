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
Matrix * create_simple_matrix(int row, int col);
Matrix * create_id_matrix(int row, int col);
Matrix * create_random_matrix(int row, int col);
Matrix * alloc_block_matrix(int row, int col);
void matrix_mult_add(Matrix * a, Matrix *b, Matrix *c);
void matrix_mult_add_cblas(Matrix * a, Matrix *b, Matrix *c);
void shift_matrices(Matrix * m,int  max_size,  int source , int dest);
#endif
