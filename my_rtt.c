// Single Author info:
// vkarri Vivek Reddy Karri

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>

#include "my_mpi.h"

/* This is the root process */
#define  ROOT    0

// Max Number of Iterations defined as a constant
#define  MAXITR   5

// Different types of messages defined as a constant
#define  MSGTYPES  18

int main (int argc, char *argv[])
{
        // Array to store the logging information
        double times[MAXITR];

        // Struct to get the rtt information
        struct timeval begin, end;

        /* process information */
        int  numproc, rank;

        /* initialize MPI */
        MPI_Init(argc, argv, &rank, &numproc);

        if (numproc%2 != 0) {
          printf("This Program will only work for even set of processes, so only take 2,4,6,8....");
        }

        // Populating the Message array
        int msgtypes[MSGTYPES];
        int j = 0;
        int k = 16;
        for (j = 0; j<MSGTYPES; j++){
            msgtypes[j] = k;
            k = k*2;
        }


        // Outer Loop to loop over the Different Message types.
        int i, onMsg;

        for (onMsg = 0; onMsg < MSGTYPES; onMsg++){

          //if (onMsg < 9) continue;
          wait(1);

          // A char-array of data size specified by the index onMsg. (Send Buffer)
          char msgPtr[msgtypes[onMsg]];

          memset(msgPtr, 'v', msgtypes[onMsg]*sizeof(char)); // To set all the elements in the array to some random data.

          // A char-array of data size specified by the index onMsg. (Recieve Buffer)
          char rcvPtr[msgtypes[onMsg]];

          memset(rcvPtr, '$', msgtypes[onMsg]*sizeof(char));

          // Routine to check if the dynamic memory has not been allocated.
          if (msgPtr == NULL) {
                printf("Memory not allocated, in the node of rank %d\n", rank);
                MPI_Finalize();
                exit(0);
          }

          for(i=0; i<MAXITR; i++){
            gettimeofday(&begin, NULL);  // Starting to log the time for 1 iteration and 1 MSG size
            /* Essentially a even node is sending message to odd node and vice versa */
            if (rank%2 == 0) {
               MPI_Sendrecv(&msgPtr,  msgtypes[onMsg], 1, rank+1, 1, &rcvPtr, msgtypes[onMsg], 1, rank+1, 1);
            }
            else{
               MPI_Sendrecv(&msgPtr,  msgtypes[onMsg], 1, rank-1, 1, &rcvPtr, msgtypes[onMsg], 1, rank-1, 1);
            }
            gettimeofday(&end, NULL);   // End of logging for 1 iteration and 1 MSG size
            times[i] = (end.tv_sec - begin.tv_sec)*1000000.0 + (end.tv_usec - begin.tv_usec);
          }


          double sum, average, sum1, variance, std_deviation;

          sum = 0.0;
          sum1 = 0.0;

          // Routine to find the sum, average, variance and st deviation.
          for (i = 1; i < MAXITR ; i++)
          {
              sum = sum + times[i];
          }

          average = sum / (double)(MAXITR);

          /*  Compute  variance  and standard deviation  */
          for (i = 1; i < MAXITR; i++)
          {
              sum1 = sum1 + pow((times[i] - average), 2);
          }

          variance = sum1 / (double)(MAXITR);
          std_deviation = sqrt(variance);

          // Print Statements to display the output.
          printf("%d %d %lf %lf\n",  rank, msgtypes[onMsg], average, std_deviation);

          fflush(stdout);

        }
        /* graceful exit */
        MPI_Finalize();

}
