#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "my_mpi.h"

char nodeNames[MAXCONNECT][NODE_NAME_LEN];
int RANK;
int NUMPROC;
char nameFILE[NODE_NAME_LEN];

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void fileAsArray(char *filename, int numLines) {
    if (numLines >= MAXCONNECT){
      fprintf(stderr, "Num Nodes Exceeded your Limit, Consider changing the size of the array!\n");
      exit(2);
    }
    FILE *fp = fopen(filename, "r");
        if (fp==NULL) {
        fprintf(stderr, "Error: File not found!\n");
        exit(2);
    }
    int i;
    for (i=0; i<numLines; i++) {
        if (fgets(nodeNames[i],NODE_NAME_LEN-1,fp)==NULL) break;
        int j = strlen(nodeNames[i])-1;
        if (nodeNames[i][j]=='\n' || nodeNames[i][j]=='\r') {
            nodeNames[i][j]='\0';
        }

    }
    /* Close file */
    fclose(fp);
}

int MPI_Init(int argc, char **argv, int *rank, int *numproc){
    if(argc == 4) {
        RANK = atoi(argv[1]); //Rank of the node you are in.
        NUMPROC = atoi(argv[2]); //Number of total nodes/processes.
        strcpy(nameFILE, argv[3]); // Name of the node file.
    }
    else
    {
        printf("Please pass the expected number of arguments which is four\n");
        exit(0);
    }

    fileAsArray(nameFILE, NUMPROC); // To load names of the nodes into an array

}


int main(int argc, char **argv){

    int rank = 0;
    int numproc = 0;

    MPI_Init(argc, argv, &rank, &numproc);

    int i;
    for (i=0; i<NUMPROC; i++) {
        printf("Node: %s, Rank %d\n", nodeNames[i], RANK);
    }
}
