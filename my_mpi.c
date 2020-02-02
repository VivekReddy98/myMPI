#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include "my_mpi.h"

char nodeNames[MAXCONNECT][NODE_NAME_LEN];
int RANK;
int NUMPROC;
int MAXCLIENTFD;
char nameFILE[NODE_NAME_LEN];

pthread_t server_starting_thread, client_thread;
pthread_cond_t cv_server_sync = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m_server_sync = PTHREAD_MUTEX_INITIALIZER;

int clientFd[MAXCONNECT]; // Used by Server
int serverFd[MAXCONNECT]; // Used by the Client

int *SERVERHALT;
int CLIENTSTART;

pthread_cond_t cv_server_sync = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m_server_sync = PTHREAD_MUTEX_INITIALIZER;

int MPI_Init(int argc, char **argv, int *rank, int *numproc) {


  CLIENTSTART = 0;

  if(argc == 4) {
      RANK = atoi(argv[1]); //Rank of the node you are in.
      NUMPROC = atoi(argv[2]); //Number of total nodes/processes.
      strcpy(nameFILE, argv[3]); // Name of the node file.
  }
  else error("Please pass the expected number of arguments which is four\n");

  *rank = RANK;
  *numproc = NUMPROC;

  int server_sockfd;
  fileAsArray(nameFILE, NUMPROC); // To load names of the nodes into an array

  struct sockaddr_in serv_addr;
  serv_addr = getServerAddr(RANK);

  startServer(serv_addr); // Server Starts Listening. // Non-Blocking call so control returns immediately.

  /* Server SetUp Code */  // printf("Address of the server: ip is : %s \n" , inet_ntoa(serv_addr.sin_addr));

  if (RANK%2 == 0){
      if(pthread_create(&server_starting_thread , NULL , server_connection_handler , NULL) < 0) error("Could Not create Server Thread");
      pthread_detach(server_starting_thread);
      //client_connection_handler(NULL, 1);
  }
  else{
      // if(pthread_create(&server_starting_thread , NULL , server_connection_handler , NULL) < 0) error("Could Not create Server Thread");
      // pthread_detach(server_starting_thread);
      client_connection_handler(NULL, 0);
  }



  if(RANK%2 == 0){
      client_connection_handler(NULL, 0);
  }
  else{
   client_connection_handler(NULL, 1);
  }

  //pthread_join(&server_thread, NULL);

  int i=0;
  while(i<10){
    sleep(1);
    i++;
  }

  pthread_cancel(server_thread);
  free(CLIENTSTART);
  return 0;
}

int MPI_Finalize(){
  shutdownServer();
  shutdownClient();
  return 0;
}


int MPI_Finalize() {
    shutdownServer();
    shutdownClient();
    return 0;
}

int main(int argc, char **argv){

    int rank = 0;
    int numproc = 0;
    int MAXCLIENTFD = 0;
    memset(clientFd, 0,  MAXCONNECT);
    memset(serverFd, 0,  MAXCONNECT);

    MPI_Init(argc, argv, &rank, &numproc);

    MPI_Finalize();

    // int i;
    // for (i=0; i<NUMPROC; i++) {
    //      printf("Node: %s, Rank %d\n", nodeNames[i], RANK);
    // }

}
