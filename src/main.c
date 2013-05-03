#include <stdio.h>
#include <mpi.h>
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
        for (int row = 0; row < parCtxt->P; row++)
        {
            for (int col = 0; col <parCtxt->P; col++)
            {
                int destBcol = col;
                int destBrow = row;
                destBrow = mod((destBrow - col),parCtxt->P); //shift
                int destB = destBrow * parCtxt->P + destBcol;

                int destArow = row;
                int destAcol = col;
                destAcol = mod((destAcol -row),parCtxt->P); //shift
                int destA = destArow * parCtxt->P +destAcol;

                printf("Send to destA %d destB %d\n", destA,
                        destB);

                if (destA != 0)
                    for (int i = 0; i < parCtxt->i; i++)
                    {
                        MPI_Send(&(A->ptr[(row*parCtxt->i + i)*A->J+col*parCtxt->j]), parCtxt->j,
                            MPI_DOUBLE, destA, 0xA, MPI_COMM_WORLD);
                    }
                if (destB != 0)
                {
                    for (int i = 0; i <parCtxt->i; i++)
                    {
                        MPI_Send(&(B->ptr[(row*parCtxt->i+i)*B->J+col*parCtxt->j]), parCtxt->j,
                            MPI_DOUBLE, destB, 0xB, MPI_COMM_WORLD);
                    }
                }
            }
        }

        for (int i = 0; i<parCtxt->i; i++)
        {
            for (int j = 0; j<parCtxt->j;j++)
            {
                a->ptr[i*a->J+j] = A->ptr[i*A->J+j];
                b->ptr[i*b->J+j] = B->ptr[i*B->J+j];
            }
        }

    }
    else
    {
        MPI_Status status;
        for (int i = 0; i<parCtxt->i; i++)
        {
            MPI_Recv(&(a->ptr[i*parCtxt->j]), parCtxt->j, MPI_DOUBLE,
                0, 0xA, MPI_COMM_WORLD, &status);
        }
        for (int i = 0; i<parCtxt->i; i++)
        {
            MPI_Recv(&(b->ptr[i*parCtxt->j]), parCtxt->j, MPI_DOUBLE,
                0, 0xB, MPI_COMM_WORLD, &status);
        }
    }
}

int main(int argc, char** argv)
{
    PAR_CTXT  * parCtxt;
    parCtxt =  parallel_context_init(argc, argv);
    Matrix * a = alloc_block_matrix(parCtxt);
    Matrix * b = alloc_block_matrix(parCtxt);
    Matrix * c = alloc_block_matrix(parCtxt);

    //TODO
    char * A_filename = argv[1];
    char * B_filename = argv[2];

    Matrix * A;
    Matrix * B;
    if (parCtxt->rank == 0)
    {
        A = create_simple_matrix(parCtxt);
        B = create_id_matrix(parCtxt);
        /*A = read_matrix(A_filename);*/
        /*B = read_matrix(B_filename);*/
    }

    initial_distrib(parCtxt, A, B,a ,b);

    matrix_mult_add(a,b,c);
    MPI_Status status;
    for (int shift = 1; shift < parCtxt->P; shift++)
    {
        int destArow = parCtxt->p;
        int destAcol = mod((parCtxt->q-1),parCtxt->P);
        int rcvAcol = mod((parCtxt->q+1),parCtxt->P);
        int destA = destArow * parCtxt->P + destAcol;
        int rcvA = destArow * parCtxt->P + rcvAcol;


        MPI_Sendrecv_replace(a->ptr, parCtxt->i*parCtxt->j, MPI_DOUBLE,
                        destA, shift, rcvA, shift, MPI_COMM_WORLD,
                            &status);

        int Bcol = parCtxt->q;
        int destBrow = mod((parCtxt->p-1),parCtxt->P);
        int rcvBrow = mod((parCtxt->p+1),parCtxt->P);
        int destB = destBrow *parCtxt->P + Bcol;
        int rcvB = rcvBrow *parCtxt->P + Bcol;

        MPI_Sendrecv_replace(b->ptr, parCtxt->i*parCtxt->j, MPI_DOUBLE,
                destB, shift, rcvB, shift, MPI_COMM_WORLD,
                &status);

        matrix_mult_add(a,b,c);
    }

    for (int i = 0; i <parCtxt->i; i++)
    {
        for (int j = 0; j<parCtxt->j; j++)
            printf("processor (line %d, col %d) : results %lf\n", parCtxt->p*parCtxt->i+i, parCtxt->q*parCtxt->j+j, c->ptr[i*c->J+j]);
    }

    parallel_context_finalize(parCtxt);
}


