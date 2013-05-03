#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>

#include "matrix.h"
#include "cblas.h"

void matrix_mult_add_cblas(Matrix * a, Matrix *b, Matrix *c)
{
   assert(a->J == b->I);
   cblas_dgemm(CblasRowMajor , CblasNoTrans,CblasNoTrans, a->I, b->J, a->J, 
           1, a->ptr, a->J, b->ptr, b->J ,1, c->ptr, c->J);
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
    m->ptr = (double *)malloc( m->I*m->J*sizeof(double));
    for (int i = 0 ; i<m->I; i++)
    {
        for (int j = 0; j<m->J; j++)
        {
            m->ptr[i*m->J+j] = 0;
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
                c->ptr[i*c->J+j] += 
                    a->ptr[i*a->J+k]*b->ptr[k*b->J+j];
            }
        }
    }
}
