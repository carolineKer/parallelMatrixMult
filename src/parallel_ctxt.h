#ifndef __PAR_CTXT__
#define __PAR_CTXT__
typedef struct _PAR_CTXT
{
    int P; //P*P mesh
    int p; //row of the processor in the mesh
    int q; //line of the processor in the mesh
    int rank;
    int i;
    int j;
    int k;
} PAR_CTXT;

PAR_CTXT * parallel_context_init(int argc, char** argv);
void parallel_context_finalize(PAR_CTXT * parCtxt);

#endif /* __PAR_CTXT__ */
