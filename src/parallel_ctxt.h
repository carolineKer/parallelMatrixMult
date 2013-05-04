#ifndef __PAR_CTXT__
#define __PAR_CTXT__
typedef struct _PAR_CTXT
{
    int P; //P*P mesh
    int p; //row of the processor in the mesh
    int q; //line of the processor in the mesh
    int rank;
    int i; //number of rows of block matrix a and c
    int j; //number of columns of block matrix b and c
    int k; //number of col of a and row of b --> change during computations if P does not divide K (col of A = row of B)
} PAR_CTXT;

PAR_CTXT * parallel_context_init(int argc, char** argv);
void parallel_context_finalize(PAR_CTXT * parCtxt);
void size_of_block(PAR_CTXT * parCtxt, int I, int J);

#endif /* __PAR_CTXT__ */
