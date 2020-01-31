#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include "my_mpi.h"

char nodeNames[MAXCONNECT][NODE_NAME_LEN];
int RANK;
int NUMPROC;
int MAXCLIENTFD = 0;
char nameFILE[NODE_NAME_LEN];
pthread_t server_thread, client_thread;

int clientFd[MAXCONNECT]; // Used by the Client
int serverFd[MAXCONNECT]; // Used by Server

memset(clientFd, 0,  MAXCONNECT);
memset(serverFd, 0,  MAXCONNECT);

int *SERVERHALT;
int *CLIENTSTART;


int MPI_Init(int argc, char **argv, int *rank, int *numproc) {

  CLIENTSTART = (int *)calloc(1, sizeof(int));

  if(argc == 4) {
      RANK = atoi(argv[1]); //Rank of the node you are in.
      NUMPROC = atoi(argv[2]); //Number of total nodes/processes.
      strcpy(nameFILE, argv[3]); // Name of the node file.
  }
  else error("Please pass the expected number of arguments which is four\n");
  int server_sockfd;
  fileAsArray(nameFILE, NUMPROC); // To load names of the nodes into an array

  struct sockaddr_in *serv_addr;
  serv_addr = getServerAddr(RANK);
  startServer(serv_addr); // Server Starts Listening. // Non-Blocking call so control returns immediately.

  /* Server SetUp Code */
  if(pthread_create(&server_thread , NULL ,  server_connection_handler , NULL) < 0) error("Could Not create Server Thread");

  // int cret, r_tosend;
  // int client_sockfd;
  // client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // if (client_sockfd < 0) error("ERROR opening socket");
  //
  // if (RANK == 1){
  //   r_tosend = 0;
  //   //cret = pthread_create(&client_thread, NULL, client_connection_handler, (void *)&r_tosend);
  //   client_connection_handler(&client_sockfd, &r_tosend);
  // }
  // else{
  //   r_tosend = 0;
  //   //cret = pthread_create(&client_thread, NULL, client_connection_handler, (void *)&r_tosend);
  //   client_connection_handler(&client_sockfd, &r_tosend);
  // }

  //if (cret < 0) error("Could Not create Client Thread");

  pthread_join(&server_thread, NULL);
  //pthread_join(&client_thread, NULL);
  //
  //printf("Thread 1 returns: %d, rank %d\n", sret, RANK);
  //printf("Thread 2 returns: %d, rank %d\n", cret, RANK);
  //fflush(stdout);
  //pthread_exit(NULL);

  free(CLIENTSTART);
  close(client_sockfd);
  close(server_sockfd);
  return 0;

}



void *server_connection_handler(void *ptr) {

    //Get the socket descriptor
    struct sockaddr_in client_addr;
    bzero((char *) &client_addr, sizeof(client_addr));
    socklen_t clilen = sizeof(client_addr);

    int accept_sockfd;

    int numConnectionsEstablished = 1;

    //set of socket descriptors
    fd_set readfds;

    // Clear FD_Sets;
    FD_ZERO(&rfds);

    FD_SET(clientFd[RANK], &amp;readfds);


    // Max Value of socket Descriptor
    // char *client_message = calloc(2048, sizeof(char));




    //while(1){
      *CLIENTSTART = 1;

      accept_sockfd = accept(server_sockfd, (struct sockaddr*)&client_addr, &clilen);
      printf("Connection Accepted\n");
      fflush(stdout);

      if (accept_sockfd < 0) {
        error("Error in accepting connections");
      }
      else {
          int numRead = read(accept_sockfd, client_message, 2048);
          if (numRead < 0) error("Error in Reading Message\n");

          printf("%s \n", client_message);
          fflush(stdout);
      }
      //sleep(1);
      //break;
    //}

    //

    free(client_message);
    close(accept_sockfd);
    return NULL;
    //pthread_exit(NULL);
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
