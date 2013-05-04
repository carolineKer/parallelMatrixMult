#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "parallel_ctxt.h"

PAR_CTXT * parallel_context_init(int argc, char** argv)
{
    int rank, total_nb_proc;
    PAR_CTXT *parCtxt  = (PAR_CTXT * )malloc(sizeof(PAR_CTXT));

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD,&total_nb_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    //Check that total_nb_proc is a square number
    double q = sqrt((double)total_nb_proc);
    double intpart;
    double fracpart = modf(q, &intpart);
    if (fracpart != 0.0)
    {
        printf("The number of processors must be a square number\n");
        exit(-1);
    }

    //Initialize parCtxt
    parCtxt->P = (int)intpart;
    parCtxt->p = rank/parCtxt->P;
    parCtxt->q = rank%parCtxt->P;
    parCtxt->rank = rank;

    return parCtxt;
}

//(i, j) = size of block matrix on this process with no shift
void size_of_block(PAR_CTXT *parCtxt, int I, int J)
{
    parCtxt->i = I/parCtxt->P;
    if (parCtxt->p < I%parCtxt-> P)
        (parCtxt->i)++;
    parCtxt->j = J/parCtxt->P;
    if (parCtxt->q < J%parCtxt->P)
        (parCtxt->j)++;
}

void parallel_context_finalize(PAR_CTXT * parCtxt)
{
    free(parCtxt);
    MPI_Finalize();
}
