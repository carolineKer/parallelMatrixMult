#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include "parallel_ctxt.h" 
#include "matrix.h"

int mod(int r,int  m)
{
    int res = r%m;
    if (res < 0)
        res += m;
    return res;
}

void initial_distrib(PAR_CTXT * parCtxt, Matrix * A,
        Matrix * B, Matrix * a, Matrix * b)
{
    //Process 0 sends matrices
    if (parCtxt->rank == 0)
    {
        int A_x = 0;
        int A_y = 0;
        int B_x = 0;
        int B_y = 0;
        for (int row = 0; row < parCtxt->P; row++)
        {
            for (int col = 0; col <parCtxt->P; col++)
            {
                //Size of block matrix A_row_col
                PAR_CTXT A_ctxt;
                A_ctxt.P = parCtxt->P;
                A_ctxt.p = row;
                A_ctxt.q = col;
                size_of_block(&A_ctxt, A->I, A->J);

                //Where to send matrix A_row_col (initial shift)
                int destArow = row;
                int destAcol = col;
                destAcol = mod((destAcol -row),parCtxt->P); //shift
                int destA = destArow * parCtxt->P +destAcol;

                //Size of block matrix B_row_col
                PAR_CTXT B_ctxt;
                B_ctxt.P = parCtxt->P;
                B_ctxt.p = row;
                B_ctxt.q = col;
                size_of_block(&B_ctxt, B->I, B->J);

                //Where to send matrix B_row_col (initial shift)
                int destBcol = col;
                int destBrow = row;
                destBrow = mod((destBrow - col),parCtxt->P); //shift
                int destB = destBrow * parCtxt->P + destBcol;

                ///////////////////////////////////////////////
                //distribute matrix A
                //////////////////////////////////////////////
                if (destA != 0)
                {
                    int dim[2];
                    dim[0]= A_ctxt.i;
                    dim[1]=A_ctxt.j;

                    MPI_Send(dim, 2, MPI_INTEGER, destA, 0xA, MPI_COMM_WORLD);

                    for (int i = 0; i < A_ctxt.i; i++)
                    {
                        printf("send A %d a %d %d to %d, %lf\n", 
(A_x + i)*A->J+A_y,
                                row, col, destA, A->ptr[(A_x + i)*A->J+A_y]);
                        MPI_Send(&(A->ptr[(A_x + i)*A->J+A_y]), A_ctxt.j,
                            MPI_DOUBLE, destA, 0xA, MPI_COMM_WORLD);
                    }
                }
                else
                {
                    assert(row == 0 && col == 0);
                    a->I = A_ctxt.i;
                    a->J = A_ctxt.j;
                    for (int i = 0; i<A_ctxt.i; i++)
                        for (int j = 0; j<A_ctxt.j; j++)
                            a->ptr[i*a->J+j] = A->ptr[i*A->J+j];
                }

                if (destB != 0) //Send block
                {
                    int dim[2];
                    dim[0] = B_ctxt.i;
                    dim[1] = B_ctxt.j;
                    MPI_Send(dim, 2, MPI_INTEGER, destB, 0xB, MPI_COMM_WORLD);

                    for (int i = 0; i <B_ctxt.i; i++)
                    {
                        MPI_Send(&(B->ptr[(B_x+i)*B->J+B_y]), B_ctxt.j,
                            MPI_DOUBLE, destB, 0xB, MPI_COMM_WORLD);
                    }
                }
                else //copy
                {
                    assert(row == 0 && col == 0);
                    b->I = B_ctxt.i;
                    b->J = B_ctxt.j;
                    for (int i = 0; i<B_ctxt.i; i++)
                        for (int j = 0; j<B_ctxt.j; j++)
                            b->ptr[i*b->J+j] = B->ptr[i*B->J+j];
                }

                //Next part of A/B to send
                B_y = (B_y+B_ctxt.j)%B->J;
                A_y = (A_y+A_ctxt.j)%A->J;

                if (col == parCtxt->P-1)
                {
                    B_x += B_ctxt.i;
                    A_x += A_ctxt.i;
                }
            }
        }

    } //Receive block matrices
    else
    {
        MPI_Status status;
        int dim[2];
        if (parCtxt->rank == 2)
        printf("process %d %d wait for a_dim\n", parCtxt->p, parCtxt->q);
        MPI_Recv(dim, 2, MPI_INTEGER, 0, 0xA, MPI_COMM_WORLD, &status);
        if (parCtxt->rank == 2)
        printf("process %d %d  a_dim\n", parCtxt->p, parCtxt->q);
        a->I = dim[0];
        a->J = dim[1];
        for (int i = 0; i<a->I; i++)
        {
            MPI_Recv(&(a->ptr[i*a->J]), a->J, MPI_DOUBLE,
                0, 0xA, MPI_COMM_WORLD, &status);
        }

        if (parCtxt->rank == 2)
        printf("process %d %d wait for b_dim\n", parCtxt->p, parCtxt->q);
        MPI_Recv(dim, 2, MPI_INTEGER, 0, 0xB, MPI_COMM_WORLD, &status);
        if (parCtxt->rank == 2)
        printf("process %d %d   b_dim\n", parCtxt->p, parCtxt->q);
        b->I = dim[0];
        b->J = dim[1];
        if (parCtxt->rank == 2)
        printf("process %d %d size b %d %d\n",parCtxt->p, parCtxt->q, b->I, b->J);
        for (int i = 0; i<b->I; i++)
        {
            MPI_Recv(&(b->ptr[i*b->J]), b->J, MPI_DOUBLE,
                0, 0xB, MPI_COMM_WORLD, &status);
        }

    }
        if (parCtxt->rank == 2)
        {
        printf("process %d %d a %lf\n",parCtxt->p, parCtxt->q, a->ptr[0]);
        printf("process %d %d size a %d %d\n",parCtxt->p, parCtxt->q, a->I, a->J);
        printf("process %d %d b %lf\n",parCtxt->p, parCtxt->q, 
                b->ptr[0 ]);
        }
}

int main(int argc, char** argv)
{
    PAR_CTXT  * parCtxt;
    parCtxt =  parallel_context_init(argc, argv);
    int dim[2];

    Matrix * A = NULL;
    Matrix * B = NULL;

    if (parCtxt->rank == 0)
    {
        if (argc != 4)
        {
            printf("%s matrixA matrixB\n", argv[0]);
            MPI_Finalize();
            exit(-1);
        }
        char * A_filename = argv[1];
        char * B_filename = argv[2];

        /*A = create_simple_matrix(parCtxt->i*parCtxt->P, */
                /*parCtxt->k*parCtxt->P);*/
        /*B = create_id_matrix(parCtxt->k*parCtxt->P,*/
                /*parCtxt->j*parCtxt->P);*/
        A = read_matrix(A_filename);
        B = read_matrix(B_filename);


        if (A->I < parCtxt->P || A->J < parCtxt->P ||
                B->I < parCtxt->P || B->J < parCtxt->P)
        {
            printf("Matrices are too small\n");
        }

        if (A->J != B->I)
        {
            printf("Matrices of incompatible sizes\n");
            MPI_Finalize();
            exit(-1);
        }

        dim[0] = A->I;
        dim[1] = A->J;
        dim[2] = B->J;

    }



    //Process 0 broadcasts max size of block matrices
    MPI_Bcast(dim, 3, MPI_INTEGER, 0, MPI_COMM_WORLD);
    //Compute max size of block matrices
    //--> a is i*k
    //--> b is k*j
    //--> c is i*j
    //a and b are shifted but c does not move
    //--> i and j are fixed, k can vary when the matrices
    //are shifted

    size_of_block(parCtxt, dim[0], dim[2]);

    int max_k = dim[1]/parCtxt->P;
    if (dim[1]%parCtxt->P)
        max_k++;

    //Allocate block matrices
    Matrix * a = alloc_block_matrix(parCtxt->i, max_k);
    Matrix * b = alloc_block_matrix(max_k, parCtxt->j);
    Matrix * c = alloc_block_matrix(parCtxt->i, parCtxt->j);

    initial_distrib(parCtxt, A, B,a ,b);
    printf("init done %d\n", parCtxt->rank);

    matrix_mult_add_cblas(a,b,c);
    printf("first mult done\n");
    MPI_Status status;
    for (int shift = 1; shift < parCtxt->P; shift++)
    {
        int destArow = parCtxt->p;
        int destAcol = mod((parCtxt->q-1),parCtxt->P);
        int rcvAcol = mod((parCtxt->q+1),parCtxt->P);
        int destA = destArow * parCtxt->P + destAcol;
        int rcvA = destArow * parCtxt->P + rcvAcol;

        printf("process %d %d shift %d a\n",parCtxt->p, parCtxt->q, shift);
        shift_matrices(a, parCtxt->i*max_k, rcvA, destA);
        printf("process %d %d shift %d a Done\n", parCtxt->p, parCtxt->q, shift);

        int Bcol = parCtxt->q;
        int destBrow = mod((parCtxt->p-1),parCtxt->P);
        int rcvBrow = mod((parCtxt->p+1),parCtxt->P);
        int destB = destBrow *parCtxt->P + Bcol;
        int rcvB = rcvBrow *parCtxt->P + Bcol;

        printf("process %d %d shift %d b\n" ,parCtxt->p, parCtxt->q, shift);
        shift_matrices(b, parCtxt->j*max_k, rcvB, destB);
        printf("process %d %d shift %d b Done\n", parCtxt->p, parCtxt->q, shift);

        matrix_mult_add_cblas(a,b,c);
    }

    char filename[100];
    sprintf(filename, "%s/MatrixC_%d_%d", argv[3], parCtxt->p, parCtxt->q);

    FILE * fp = fopen(filename,"w");
    if (fp == NULL)
    {
        printf("Unable to open %s\n", filename);
        exit(-1);
    }

    for (int i = 0; i < c->I; i++)
    {
        for (int j = 0; j<c->J; j++)
        {
            fprintf(fp, "%lf", c->ptr[i*c->J+j]);
            if (j != c->J-1) fprintf(fp, ",");
        }
        if (i!= c->I-1) fprintf(fp, ";");
    }
    fclose(fp);
    
    MPI_Barrier(MPI_COMM_WORLD);

    parallel_context_finalize(parCtxt);
}


