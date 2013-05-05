#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>
#include "parallel_ctxt.h" 
#include "parallel_print.h"
#include "matrix.h"

int mod(int r,int  m)
{
    int res = r%m;
    if (res < 0)
        res += m;
    return res;
}

void usage(char ** argv)
{
    printf("%s directory [ I K J ]\n", argv[0]);
    printf("If I K and J are given, the matrices are randomly picked (A (I,K), B(K,J)), and the result is not saved\n");
    printf("Otherwise, the program tries to read matrixA and matrixB from dir and saves the resulting matrix\n.");
    printf("In both cases, the timing performances are stored in directory\n");
}


void initial_distrib(PAR_CTXT * parCtxt, Matrix * A,
        Matrix * B, Matrix * a, Matrix * b)
{
    //Process 0 sends matrices
    if (parCtxt->rank == 0)
    {
        MPI_Request *reqs = calloc(parCtxt->P * parCtxt->P, sizeof(MPI_Request));
        int reqIndex = 0;

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
                        /*printf("PROC 0: destA: %d\n", destA);*/
                    int dim[2];
                    dim[0]= A_ctxt.i;
                    dim[1]=A_ctxt.j;

                    MPI_Send(dim, 2, MPI_INTEGER, destA, 0xA, MPI_COMM_WORLD);

                    for (int i = 0; i < A_ctxt.i; i++)
                    {
                        MPI_Send(&(A->ptr[(A_x + i)*A->J+A_y]), A_ctxt.j,
                            MPI_DOUBLE, destA, 0xA, MPI_COMM_WORLD);
                    }
                }
                else
                {
                        /*printf("PROC 0: destA: 0\n");*/
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
                        /*printf("PROC 0: destB: %d\n", destB);*/
                    MPI_Send(dim, 2, MPI_INTEGER, destB, 0xB, MPI_COMM_WORLD);

                    for (int i = 0; i <B_ctxt.i; i++)
                    {
                        MPI_Send(&(B->ptr[(B_x+i)*B->J+B_y]), B_ctxt.j,
                            MPI_DOUBLE, destB, 0xB, MPI_COMM_WORLD);
                    }
                }
                else //copy
                {
                        /*printf("PROC 0: destB: %d\n", 0);*/
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
        /*printf("PROC 0: termine initial distrib\n");*/

    } //Receive block matrices
    else
    {
        MPI_Status status;
        int dim[2];
                /*printf("PROC: %d, INITIAL DISTRIB: avant recv taille a\n", parCtxt->rank);*/
        MPI_Recv(dim, 2, MPI_INTEGER, 0, 0xA, MPI_COMM_WORLD, &status);
        if (parCtxt->rank == 2)
        a->I = dim[0];
        a->J = dim[1];
                /*printf("PROC: %d, INITIAL DISTRIB: avant recv contenu a\n", parCtxt->rank);*/
        for (int i = 0; i<a->I; i++)
        {
            MPI_Recv(&(a->ptr[i*a->J]), a->J, MPI_DOUBLE,
                0, 0xA, MPI_COMM_WORLD, &status);
        }

                /*printf("PROC: %d, INITIAL DISTRIB: avant recv taille b\n", parCtxt->rank);*/
        MPI_Recv(dim, 2, MPI_INTEGER, 0, 0xB, MPI_COMM_WORLD, &status);
        b->I = dim[0];
        b->J = dim[1];
                /*printf("PROC: %d, INITIAL DISTRIB: avant recv contenu b\n", parCtxt->rank);*/
        for (int i = 0; i<b->I; i++)
        {
            MPI_Recv(&(b->ptr[i*b->J]), b->J, MPI_DOUBLE,
                0, 0xB, MPI_COMM_WORLD, &status);
        }

    }
}




int main(int argc, char** argv)
{
    PAR_CTXT  * parCtxt;
    parCtxt =  parallel_context_init(argc, argv);
    int dim[3];

    Matrix * A = NULL;
    Matrix * B = NULL;
    char * directory;

    if (parCtxt->rank == 0)
    {
        if (argc == 5)
        { 
            int I,J,K;
            directory = argv[1];
            I = atoi(argv[2]);
            K = atoi(argv[3]);
            J = atoi(argv[4]);
            if (I==0 || J == 0 || K == 0)
            {
                /*usage(argv);*/
                MPI_Finalize();
                exit(-1);
            }
            A = create_random_matrix(I,K);
            B = create_random_matrix(K,J);
        }
        else if (argc == 2)
        {
            directory = argv[1];
            char filename[100];
            sprintf(filename, "%s/matrixA", directory);
            A = read_matrix(filename);

            sprintf(filename, "%s/matrixB", directory);
            B = read_matrix(filename);

            printf("%d %d\n",A->I, A->J);
            printf("%d %d\n",B->I, B->J);
        }
        else
        {
            /*usage(argv);*/
            MPI_Finalize();
            exit(-1);
        }



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


    /////////////////////////////////
    // Start
    ////////////////////////////////
    double time[2];
    time[0] = MPI_Wtime();



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

    double *a_buf_tmp = calloc(parCtxt->i * max_k, sizeof(double));
    double *b_buf_tmp = calloc(parCtxt->j * max_k, sizeof(double));

    initial_distrib(parCtxt, A, B,a ,b);

    matrix_mult_add_cblas(a,b,c);
    MPI_Status status;

#define TAG_SIZE_A 1
#define TAG_CONTENT_A 2
#define TAG_SIZE_B 3
#define TAG_CONTENT_B 4

    MPI_Request reqs[8];

    /*printf("Taille tu mesh: %d\n", parCtxt->P);*/
    for (int shift = 1; shift < parCtxt->P; shift++)
    {
        int destArow = parCtxt->p;
        int destAcol = mod((parCtxt->q-1),parCtxt->P);
        int rcvAcol = mod((parCtxt->q+1),parCtxt->P);
        int destA = destArow * parCtxt->P + destAcol;
        int rcvA = destArow * parCtxt->P + rcvAcol;

        // Initiate the communications of a and its size
        /*printf("tour = %d, avant premier shift\n", shift);*/
        /*shift_matrices(a, parCtxt->i*max_k, rcvA, destA);*/

        int dimSendA[2] = {a->I, a->J};
        MPI_Isend(dimSendA, 2, MPI_INTEGER, destA, TAG_SIZE_A, MPI_COMM_WORLD, &reqs[0]);

        int dimRecvA[2];
        MPI_Irecv(dimRecvA, 2, MPI_INTEGER, rcvA, TAG_SIZE_A, MPI_COMM_WORLD, &reqs[1]);

        MPI_Isend(a->ptr, parCtxt->i * max_k, MPI_DOUBLE, destA, TAG_CONTENT_A, MPI_COMM_WORLD, &reqs[2]);

        MPI_Irecv(a_buf_tmp, parCtxt->i * max_k, MPI_DOUBLE, rcvA, TAG_CONTENT_A, MPI_COMM_WORLD, &reqs[3]);

        int Bcol = parCtxt->q;
        int destBrow = mod((parCtxt->p-1),parCtxt->P);
        int rcvBrow = mod((parCtxt->p+1),parCtxt->P);
        int destB = destBrow *parCtxt->P + Bcol;
        int rcvB = rcvBrow *parCtxt->P + Bcol;

        /*printf("tour = %d, avant second shift\n", shift);*/
        /*shift_matrices(b, parCtxt->j*max_k, rcvB, destB);*/

        int dimSendB[2] = {b->I, b->J};
        MPI_Isend(dimSendB, 2, MPI_INTEGER, destB, TAG_SIZE_B, MPI_COMM_WORLD, &reqs[4]);

        int dimRecvB[2];
        MPI_Irecv(dimRecvB, 2, MPI_INTEGER, rcvB, TAG_SIZE_B, MPI_COMM_WORLD, &reqs[5]);

        MPI_Isend(b->ptr, parCtxt->j * max_k, MPI_DOUBLE, destB, TAG_CONTENT_B, MPI_COMM_WORLD, &reqs[6]);

        MPI_Irecv(b_buf_tmp, parCtxt->j * max_k, MPI_DOUBLE, rcvB, TAG_CONTENT_B, MPI_COMM_WORLD, &reqs[7]);


        // Wait for all the communications to terminate
        int rc = MPI_Waitall(8, reqs, MPI_STATUSES_IGNORE);
        if (rc != MPI_SUCCESS) {
                printf("FAILED to wait for communications\n");
        }

        // Now do the swapping we need
        a->I = dimRecvA[0];
        a->J = dimRecvA[1];
        double *tmp = a->ptr;
        a->ptr = a_buf_tmp;
        a_buf_tmp = tmp;

        b->I = dimRecvB[0];
        b->J = dimRecvB[1];
        tmp = b->ptr;
        b->ptr = b_buf_tmp;
        b_buf_tmp = tmp;

<<<<<<< HEAD
        matrix_mult_add(a,b,c);
=======
        /*printf("tour = %d, avant multiplication\n", shift);*/
        matrix_mult_add_cblas(a,b,c);
        /*printf("tour = %d, après multiplication\n", shift);*/
>>>>>>> Using non-blocking communications for the main part
    }

    time[1] = MPI_Wtime();
    ///////////////////////////
    // END: Save results
    ///////////////////////////
    char time_file[100];
    sprintf(time_file, "%s/time_%d.txt", argv[1], parCtxt->P*
            parCtxt->P);
    
    print_data(time_file, print_time,  (void*)time, 0, 
            parCtxt->rank, parCtxt->P*parCtxt->P);

    if (argc == 2)//If matrices were read from file
    {
        char filename[100];
        sprintf(filename, "%s/matrixC/MatrixC_%d_%d", argv[1], parCtxt->p, parCtxt->q);

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
    }
    
    MPI_Barrier(MPI_COMM_WORLD);

    parallel_context_finalize(parCtxt);
}

