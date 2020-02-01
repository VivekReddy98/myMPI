#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include "my_mpi.h"

// extern char nodeNames[MAXCONNECT][NODE_NAME_LEN];
// extern int RANK;
// extern int NUMPROC;
// extern char nameFILE[NODE_NAME_LEN];
// extern pthread_t server_thread;
// extern int MAXCLIENTFD;
//
// extern int clientFd[MAXCONNECT];
// extern int serverFd[MAXCONNECT];

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void printtIP(struct hostent* host){
  printf("ip: ");
  for (int i = 0; i < host->h_length; i++) {
      printf("%d", host->h_addr_list[0][i]);
      if (i != host->h_length - 1) printf(".");
  }
  printf("\n");
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

struct sockaddr_in getServerAddr(int rk){
  /* Server SetUp Code */
  struct hostent* host;
  struct sockaddr_in serv_addr;

  host =  gethostbyname((const char *)nodeNames[rk]);


  //bzero((char *)&serv_addr, sizeof(serv_addr));

  memcpy(&serv_addr.sin_addr, host->h_addr_list[0], host->h_length);
  //bcopy((char *)host->h_addr_list[0], (char *)&serv_addr.sin_addr, host->h_length);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVERPORT);

  return serv_addr;
}

void startServer(struct sockaddr_in serv_addr){

  clientFd[RANK] = socket(AF_INET, SOCK_STREAM, 0);
  if (clientFd[RANK]< 0) error("ERROR opening socket");

  if (bind(clientFd[RANK], (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
     perror("ERROR on binding\n");
     exit(1);
  }

  if (setsockopt(clientFd[RANK], SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) error("setsockopt(SO_REUSEADDR) failed");

  listen(clientFd[RANK], MAXCONNECT-1);
}

int getRankFromIPaddr(struct sockaddr_in *serv_addr){

    char recievedIP[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(serv_addr->sin_addr), recievedIP, INET_ADDRSTRLEN);

    //printf("Input Address: %s\n", recievedIP);

    int i;
    char tempIP[INET_ADDRSTRLEN];
    struct sockaddr_in temp;
    for(i=0; i<NUMPROC; i++){

       if (i == RANK) continue;
       temp = getServerAddr(i);
       inet_ntop(AF_INET, &(temp.sin_addr), tempIP, INET_ADDRSTRLEN);
       if(strcmp((const char *)tempIP, (const char *)recievedIP) == 0) {
         return i;
       }
    }
    error("Client Not Specified in the NODELIST");
}

void shutdownServer(){
    int i;
    for (i = 0; i< NUMPROC; i++){
       if(i==RANK) continue;
       close(clientFd[i]);
    }
    close(clientFd[RANK]);
}

void shutdownClient(){
    int i;
    for (i = 0; i< NUMPROC; i++){
       if(i==RANK) continue;
       close(serverFd[i]);
    }
    //close(serverFd[RANK]);
}
