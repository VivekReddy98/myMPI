#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"
#include <string.h>
#include <unistd.h>
#include <limits.h>

int *global, *local;

int numproc, rank;

void print_matrix();

int MPI_Init( int *argc, char ***argv ){

    int retValue = PMPI_Init(argc, argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Comm_size(MPI_COMM_WORLD, &numproc);

    local = (int *)calloc(numproc, sizeof(int));

    return retValue;
}


int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    local[dest]++;

    int retValue = PMPI_Send(buf, count, datatype, dest, tag, comm);

    return retValue;
}

int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request * request){

    local[dest]++;

    int retValue = PMPI_Isend(buf, count, datatype, dest, tag, comm, request);

    return retValue;
}

int MPI_Finalize(){

    if (rank == 0){
       global = (int *)calloc(numproc*numproc, sizeof(int));

       int rk;

       int displ[numproc];
       int rcvCounts[numproc];
       for (rk = 0; rk < numproc; rk++){
            rcvCounts[rk] = numproc;
            displ[rk] = rk*numproc;
       }

       MPI_Gatherv(local, numproc, MPI_INT, global, (const int *)rcvCounts, (const int *)displ, MPI_INT, 0, MPI_COMM_WORLD);

       //printf("-----------------Gather Completed-------------------------------------------\n");
       print_matrix();
    }

    else{
       MPI_Gatherv(local, numproc, MPI_INT, NULL, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0){
      free(global);
    }

    free(local);

    int retVal = PMPI_Finalize();

    return retVal;

}

void print_matrix()
{
    int   i, j, idx;

    char filename[] = "matrix.data";

    FILE *fp = fopen(filename, "w");

    for(i = 0;  i < numproc; i++)
    {
       //fprintf(fp, "%d ", i);
       for( j = 0; j < numproc; j++){
         idx = j + i*numproc;
         //printf("%d ", global[idx]);
         fprintf(fp, "%d ", global[idx]);
       }
       //printf("\n");
       fprintf(fp, "\n");
    }

    fclose(fp);
}
