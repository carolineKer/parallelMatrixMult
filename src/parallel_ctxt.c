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
    parCtxt->i = 2; //rows per processor
    parCtxt->j = 2; //columns per processor
    parCtxt->k = 3;

    return parCtxt;
}

void parallel_context_finalize(PAR_CTXT * parCtxt)
{
    free(parCtxt);
    MPI_Finalize();
}
