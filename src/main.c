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
        Matrix * B, double  * a_ptr, double * b_ptr)
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
                    MPI_Send(&(A->ptr[row][col]), 1,
                        MPI_DOUBLE, destA, 0xA, MPI_COMM_WORLD);
                if (destB != 0)
                {
                    MPI_Send(&(B->ptr[row][col]), 1,
                        MPI_DOUBLE, destB, 0xB, MPI_COMM_WORLD);
                }
            }
        }
        *a_ptr = A->ptr[0][0];
        *b_ptr = B->ptr[0][0];

    }
    else
    {
        MPI_Status status;
        MPI_Recv(a_ptr, 1, MPI_DOUBLE,
            0, 0xA, MPI_COMM_WORLD, &status);
        MPI_Recv(b_ptr, 1, MPI_DOUBLE,
            0, 0xB, MPI_COMM_WORLD, &status);
    }
}

int main(int argc, char** argv)
{
    PAR_CTXT  * parCtxt;
    parCtxt =  parallel_context_init(argc, argv);
    double a;
    double b;
    double c;

    Matrix * A;
    Matrix * B;
    if (parCtxt->rank == 0)
    {
        char A_filename[100] = "Nom bidon1";
        char B_filename[100] = "Nom bidon2";
        printf("Going to create matrices\n");
        A = create_simple_matrix(parCtxt);
        B = create_simple_matrix(parCtxt);
        /*A = read_matrix(A_filename, &A);*/
        /*B = read_matrix(B_filename, &B);*/
        printf("Matrix created\n");
    }

    printf("Initial distribution\n");
    initial_distrib(parCtxt, A, B,&a ,& b);
    printf("After initial distribution\n");

    c = a*b;
    MPI_Status status;
    for (int shift = 1; shift < parCtxt->P; shift++)
    {
        printf("p %d shift %d\n", parCtxt->rank, shift);
        int destArow = parCtxt->p;
        int destAcol = mod((parCtxt->q-1),parCtxt->P);
        int rcvAcol = mod((parCtxt->q+1),parCtxt->P);
        int destA = destArow * parCtxt->P + destAcol;
        int rcvA = destArow * parCtxt->P + rcvAcol;

        printf("proc %d Send replace A dest %d rec %d\n",
                parCtxt->rank, destA, rcvA);
        MPI_Sendrecv_replace(&a, 1, MPI_DOUBLE,
                        destA, shift, rcvA, shift, MPI_COMM_WORLD,
                            &status);
        printf("proc %d Send replace A dest %d rec %d Done\n", 
                parCtxt->rank, destA, rcvA);


        int Bcol = parCtxt->q;
        int destBrow = mod((parCtxt->p-1),parCtxt->P);
        int rcvBrow = mod((parCtxt->p+1),parCtxt->P);
        int destB = destBrow *parCtxt->P + Bcol;
        int rcvB = rcvBrow *parCtxt->P + Bcol;
        printf("proc %d Send replace B dest %d rec %d\n", 
                parCtxt->rank ,destB, rcvB);
        MPI_Sendrecv_replace(&b, 1, MPI_DOUBLE,
                        destB, shift, rcvB, shift, MPI_COMM_WORLD,
                            &status);
        printf("proc %d Send replace B dest %d rec %d Done\n", 
                parCtxt->rank ,destB, rcvB);

        c+= a*b;
    }

    printf("processor (line %d, col %d): results %lf\n", parCtxt->p, parCtxt->q, c);

    parallel_context_finalize(parCtxt);
}


