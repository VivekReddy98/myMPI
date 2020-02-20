//vkarri Vivek Reddy Karri
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"
#include <string.h>
#include <unistd.h>
#include <limits.h>

// Placeholders to store the frequency data. Global only used in root node.
int *global, *local;

// NUMproc and rank
int numproc, rank;

// Function Declaration of print_matrix() function.
void print_matrix();


// MPI_Init Wrapper function. 
int MPI_Init( int *argc, char ***argv ){

    // Init MPI_init() first to get the rank and numproc values.
    int retValue = PMPI_Init(argc, argv);

    // Retieve Necessary Values.
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    MPI_Comm_size(MPI_COMM_WORLD, &numproc);

    // Instantiate the local placeholder.
    local = (int *)calloc(numproc, sizeof(int));

    // Return the return value returned by original MPI_Init Function.
    return retValue;
}

// MPI Send Wrapper function.
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
    
    // Increment the corresponding index in the placeholder.
    local[dest]++;

    int retValue = PMPI_Send(buf, count, datatype, dest, tag, comm);

    // Return the return value returned by original MPI_Init Function.
    return retValue;
}

// MPI ISend Wrapper function. Note: Non-Blocking Characteristics have been retained.
int MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
              MPI_Comm comm, MPI_Request * request){
    
    // Increment the corresponding index in the placeholder.
    local[dest]++;

    int retValue = PMPI_Isend(buf, count, datatype, dest, tag, comm, request);

    // Return the return value returned by original MPI_Init Function.
    return retValue;
}

int MPI_Finalize(){

    // Time to Gather the results.
    if (rank == 0){
       // Node Zero is supposed to gather all the results.
       global = (int *)calloc(numproc*numproc, sizeof(int));
       
       // Pre-Processing for GatherV.
       int rk;

       int displ[numproc];
       int rcvCounts[numproc];
       for (rk = 0; rk < numproc; rk++){
            rcvCounts[rk] = numproc;
            displ[rk] = rk*numproc;
       }

       // Invoke GatherV function.
       MPI_Gatherv(local, numproc, MPI_INT, global, (const int *)rcvCounts, (const int *)displ, MPI_INT, 0, MPI_COMM_WORLD);

       print_matrix();
    }

    else{
       MPI_Gatherv(local, numproc, MPI_INT, NULL, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
    }

    // To synchronize the execution
    MPI_Barrier(MPI_COMM_WORLD);

    // Free dynamic variables.
    if (rank == 0){
      free(global);
    }

    free(local);

    // Invoke PMPI_Finalize.
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
