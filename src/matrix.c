#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "matrix.h"

Matrix * read_matrix(char * filename)
{
    /*FILE * fp = fopen(filename, "r");*/
    /*if (fp == NULL)*/
    /*{*/
        /*printf("Unable to open %s\n",filename);*/
        /*exit(-1);*/
    /*}*/

    /*int I, J;*/
    /*fscanf(fp, "%d %d", &I, &J);*/
    Matrix * m = (Matrix *)malloc(sizeof(Matrix));
    m->I = 2;
    m->J = 2;

    m->ptr = (double **)malloc(m->I* sizeof(double *));

    for (int i = 0; i< m->I; i++)
    {
        m->ptr[i] = (double *)malloc(m->J *sizeof(double));
        for (int j = 0; j< m->J; j++)
        {
            m->ptr[i][j] = i*m->I +j;
        }
    }

    return m;
    /*fclose(fp);*/
}

//Create a matrix with one data per processor
Matrix * create_simple_matrix(PAR_CTXT * parCtxt)
{
    Matrix * m = (Matrix *)malloc(sizeof(Matrix));
    m->I = parCtxt->P * parCtxt->i;
    m->J = parCtxt->P * parCtxt->j;
    m->ptr = (double **)malloc(m->I*sizeof(double*));
    for (int i = 0 ; i<m->I; i++)
    {
        m->ptr[i] = (double *)malloc(m->J*sizeof(double));
        for (int j = 0; j<m->J; j++)
        {
            m->ptr[i][j] = i*m->I + j;
        }
    }
    return m;
}

Matrix * create_id_matrix(PAR_CTXT * parCtxt)
{
    Matrix * m = (Matrix *)malloc(sizeof(Matrix));
    m->I = parCtxt->P * parCtxt->i;
    m->J = parCtxt->P * parCtxt->j;
    m->ptr = (double **)malloc(m->I*sizeof(double*));
    for (int i = 0 ; i<m->I; i++)
    {
        m->ptr[i] = (double *)malloc(m->J*sizeof(double));
        for (int j = 0; j<m->J; j++)
        {
            if (i != j)
                m->ptr[i][j] = 0;
            else
                m->ptr[i][j] = 2;
        }
    }
    return m;
}

Matrix * alloc_block_matrix(PAR_CTXT * parCtxt)
{
    Matrix * m = (Matrix *)malloc(sizeof(Matrix));
    m->I = parCtxt->i;
    m->J = parCtxt->j;
    m->ptr = (double **)malloc(m->I*sizeof(double*));
    for (int i = 0 ; i<m->I; i++)
    {
        m->ptr[i] = (double *)malloc(m->J*sizeof(double));
        for (int j = 0; j<m->J; j++)
        {
            m->ptr[i][j] = 0;
        }
    }
    return m;
}

void matrix_mult_add(Matrix * a, Matrix *b, Matrix *c)
{
    assert(a->J == b->I);
    for (int i = 0; i<c->I; i++)
    {
        for (int j = 0; j<c->J; j++)
        {
            for (int k = 0; k <a->J; k++)
            {
                c->ptr[i][j] += a->ptr[i][k]*b->ptr[k][j];
            }
        }
    }
}
