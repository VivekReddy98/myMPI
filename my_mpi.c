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
#include "my_mpi.h"

char nodeNames[MAXCONNECT][NODE_NAME_LEN];
int RANK;
int NUMPROC;
int MAXCLIENTFD;
char nameFILE[NODE_NAME_LEN];
pthread_t server_thread, client_thread;

int clientFd[MAXCONNECT]; // Used by the Client
int serverFd[MAXCONNECT]; // Used by Server

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

  struct sockaddr_in serv_addr;
  serv_addr = getServerAddr(RANK);

  startServer(serv_addr); // Server Starts Listening. // Non-Blocking call so control returns immediately.

  /* Server SetUp Code */  // printf("Address of the server: ip is : %s \n" , inet_ntoa(serv_addr.sin_addr));
  // fflush(stdout)

  if (RANK%2 == 0){
      if(pthread_create(&server_thread , NULL , server_connection_handler , NULL) < 0) error("Could Not create Server Thread");
      pthread_detach(server_thread);
      client_connection_handler(NULL);
  }
  else{
      client_connection_handler(NULL);
      if(pthread_create(&server_thread , NULL , server_connection_handler , NULL) < 0) error("Could Not create Server Thread");
      pthread_detach(server_thread);
  }

  //pthread_join(&server_thread, NULL);

  int i=0;
  while(i<20){
    sleep(1);
    i++;
  }

  shutdownServer();
  shutdownClient();
  pthread_cancel(server_thread);
  free(CLIENTSTART);

  return 0;
}

void *server_connection_handler(void *ptr) {

    //Get the socket descriptor
    struct sockaddr_in client_addr;
    bzero((char *) &client_addr, sizeof(client_addr));
    socklen_t clilen = sizeof(client_addr);

    int accept_sockfd;

    //set of socket descriptors
    fd_set readfds;

    // Clear FD_Sets;
    FD_ZERO(&readfds);

      FD_SET(clientFd[RANK], &readfds);

    MAXCLIENTFD = clientFd[RANK];

    int numConnectionsEstablished = 1;

    int activity, sd, i;

    while(1){

      // Clear FD_Sets;
      FD_ZERO(&readfds);

      FD_SET(clientFd[RANK], &readfds);

      MAXCLIENTFD = clientFd[RANK];

      //add child sockets to set
      for ( i = 0 ; i < NUMPROC ; i++)
      {
          //socket descriptor
    			sd = clientFd[i];

    			//if valid socket descriptor then add to read list
    			if(sd > 0)
    				FD_SET(sd , &readfds);

          //highest file descriptor number, need it for the select function
          if(sd > MAXCLIENTFD)
    				MAXCLIENTFD = sd;
      }

      //*CLIENTSTART = 1;
      activity = select( MAXCLIENTFD + 1 , &readfds , NULL , NULL , NULL);  // Wait indefinitely till some activity is found on the Master Socket.

      if (activity < 0)  perror("select error");

      //printf("In Server !!!!!!!!!!!! %d\n", RANK);

      if (FD_ISSET(clientFd[RANK], &readfds)){
        accept_sockfd = accept(clientFd[RANK], (struct sockaddr *)&client_addr, &clilen);
        if (accept_sockfd < 0) error("Error In Accepting Connections!!!!!!!!!!!");
        printf("New connection , socket fd is %d , ip is : %s , Rank: %d\n" , accept_sockfd , inet_ntoa(client_addr.sin_addr), RANK);
        int rk = getRankFromIPaddr(&client_addr);
        clientFd[rk] = accept_sockfd;
        if(accept_sockfd>MAXCLIENTFD){
          MAXCLIENTFD = accept_sockfd;
        }
        numConnectionsEstablished++;

      }

      if (numConnectionsEstablished >= NUMPROC){
        break;
      }

      sleep(0.5);

    }

    printf("ALL Client Connections have establihsed:, Rank %d server sleeping for now\n", RANK);
    //free(client_message);
    //close(accept_sockfd);
    return NULL;
    //pthread_exit(NULL);
}

void *client_connection_handler(void *ptr) {

  // while(!(*CLIENTSTART)){
  //     sleep(1);
  // }

  int rank_server = 0;

  //Get the socket descriptor
  int read_size_c, connfd_c;

  struct sockaddr_in serv_addr;

  for(int rank_server = 0; rank_server < NUMPROC; rank_server++){
    if (rank_server == RANK) continue;
    //if (clientFd[rank_server] > 0)
    serv_addr = getServerAddr(rank_server); // Get the Server Address
    clientFd[rank_server] = socket(AF_INET, SOCK_STREAM, 0);

    if (clientFd[rank_server] < 0) error("ERROR opening socket");

    int connect_err = -1;
    int limit = 0;
    while (connect_err < 0){
      connect_err = connect(clientFd[rank_server], (struct sockaddr *) &serv_addr, sizeof(serv_addr));

      if (connect_err < 0) {
        //perror("Error in connecting to the server: ");
      }
      else{
        printf("Connected to server, socket fd is %d , ip is : %s , port : %d, Rank: %d\n" , clientFd[rank_server] , inet_ntoa(serv_addr.sin_addr) , ntohs(serv_addr.sin_port), RANK);
        fflush(stdout);
        break;
      }
      sleep(1);
      limit++;
      if(limit > 10){
        break;
      }
    }
  }

  shutdownClient();

  // printf("Connected With all the servers!!!!!!!!!!!!!! %d\n", RANK);
  // fflush(stdout);

  return NULL;
  //pthread_exit(NULL);
}

int main(int argc, char **argv){

    int rank = 0;
    int numproc = 0;
    int MAXCLIENTFD = 0;
    memset(clientFd, 0,  MAXCONNECT);
    memset(serverFd, 0,  MAXCONNECT);

    MPI_Init(argc, argv, &rank, &numproc);


    // int i;
    // for (i=0; i<NUMPROC; i++) {
    //      printf("Node: %s, Rank %d\n", nodeNames[i], RANK);
    // }

}
