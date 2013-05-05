#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <mpi.h>

#include "matrix.h"

//Comment on ferlin
#ifdef ON_FERLIN
#include "mkl.h"
#else
#include "cblas.h"
#endif

void matrix_mult_add_cblas(Matrix * a, Matrix *b, Matrix *c)
{
   assert(a->J == b->I);
   cblas_dgemm(CblasRowMajor , CblasNoTrans,CblasNoTrans, a->I, b->J, a->J, 
           1, a->ptr, a->J, b->ptr, b->J ,1, c->ptr, c->J);
}

//Create random matrix
Matrix * create_random_matrix(int nb_row, int nb_col)
{
    Matrix * m = (Matrix *)malloc(sizeof(Matrix));
    m->I = nb_row;
    m->J = nb_col;
    m->ptr = (double *)malloc( m->I*m->J*sizeof(double));
    for (int i = 0 ; i<m->I; i++)
    {
        for (int j = 0; j<m->J; j++)
        {
            //Random number between -100 and 100
            m->ptr[i*m->J+j]  = 200.0*((float)rand()/(float)RAND_MAX)-100.0;
        }
    }
    return m;
}

void shift_matrices(Matrix * m, int max_size,  int source , int dest )
{
    MPI_Status status;
    
    //Shift matrix
    MPI_Sendrecv_replace(m->ptr, max_size, MPI_DOUBLE, dest, 17,
            source, 17, MPI_COMM_WORLD, &status);

    //Shift matrix dimensions
    int dim[2];
    dim[0] = m->I;
    dim[1] = m->J;
    MPI_Sendrecv_replace(dim, 2, MPI_INTEGER,
                    dest, 0xcafe, source, 0xcafe, MPI_COMM_WORLD, &status);

    m->I = dim[0];
    m->J = dim[1];
}


Matrix * read_matrix(char * filename)
{
    FILE * fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Unable to open %s\n",filename);
        exit(-1);
    }

    //Get file size
    struct stat st;
    stat(filename, &st);
    long size = st.st_size;

    char * str = (char *)malloc(size + 1);

    fgets(str, size+1, fp);


    char * p = str;
    int nb_rows = 1;
    int nb_cols = 1;

    int first_line = 1;

    while (3.14)
    {
        //get size of the first line
        while (first_line)
        {
            p = strpbrk(p, ",;");
            if (p== NULL || *p== ';') 
            {
                first_line = 0;
                break;
            }
            nb_cols++;
            p++;
        }

        p = strchr(p, ';');

        if (p == NULL) break;
        nb_rows++;
        p++;
    }

    p = str;
    char * tok = strtok(p, ",;");

    Matrix * m = (Matrix *)malloc(sizeof(Matrix));
    m->I = nb_rows;
    m->J = nb_cols;
    m->ptr = (double *)malloc( m->I*m->J*sizeof(double));
    for (int i = 0 ; i<m->I; i++)
    {
        for (int j = 0; j<m->J; j++)
        {
            assert(tok != NULL);
            m->ptr[i*m->J+j] = atof(tok);
            tok = strtok(NULL, ",;");
        }
    }

    assert(tok == NULL);

    fclose(fp);
    return m;
}

//Create a matrix with one data per processor
Matrix * create_simple_matrix(int nb_row, int nb_col)
{
    Matrix * m = (Matrix *)malloc(sizeof(Matrix));
    m->I = nb_row;
    m->J = nb_col;
    m->ptr = (double *)malloc( m->I*m->J*sizeof(double));
    for (int i = 0 ; i<m->I; i++)
    {
        for (int j = 0; j<m->J; j++)
        {
            m->ptr[i*m->J+j] = i*m->I + j;
        }
    }
    return m;
}

Matrix * create_id_matrix(int nb_row, int nb_col)
{
    Matrix * m = (Matrix *)malloc(sizeof(Matrix));
    m->I = nb_row;
    m->J = nb_col;
    m->ptr = (double *)malloc( m->I*m->J*sizeof(double));
    for (int i = 0 ; i<m->I; i++)
    {
        for (int j = 0; j<m->J; j++)
        {
            if (i != j)
                m->ptr[i*m->J+j] = 0;
            else
                m->ptr[i*m->J+j] = 1;
        }
    }
    return m;
}

Matrix * alloc_block_matrix(int nb_row, int nb_col)
{
    Matrix * m = (Matrix *)malloc(sizeof(Matrix));
    m->I = nb_row;
    m->J = nb_col;
    m->ptr = (double *)calloc(m->I*m->J, sizeof(double));

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
                c->ptr[i*c->J+j] += 
                    a->ptr[i*a->J+k]*b->ptr[k*b->J+j];
            }
        }
    }
}
