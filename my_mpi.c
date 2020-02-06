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

//char sysBuffer[BUFFSIZE];

pthread_t server_thread, client_thread;


int clientFd[MAXCONNECT]; // Used by Server
int serverFd[MAXCONNECT]; // Used by the Client

int SERVERACK;
int CLIENTSTART;

pthread_cond_t cv_server_sync = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m_server_sync = PTHREAD_MUTEX_INITIALIZER;

int MPI_Init(int argc, char **argv, int *rank, int *numproc) {

  MAXCLIENTFD = 0;

  memset(clientFd, 0,  MAXCONNECT);
  memset(serverFd, 0,  MAXCONNECT);

  //SERVERHALT = 1;

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

  if (RANK%2 == 0){
      if(pthread_create(&server_thread , NULL , server_connection_handler , NULL) < 0) error("Could Not create Server Thread");
      //pthread_detach(server_thread);
      sleep(2);
      client_connection_handler(NULL, 1);
  }
  else{
      if(pthread_create(&server_thread , NULL , server_connection_handler , NULL) < 0) error("Could Not create Server Thread");
      //pthread_detach(server_thread);
      sleep(2);
      client_connection_handler(NULL, 0);
  }



  if(RANK%2 == 0){
      client_connection_handler(NULL, 0);
  }
  else{
      client_connection_handler(NULL, 1);
  }

  void * ptr = NULL;
  pthread_join(server_thread, &ptr);

  fflush(stdout);

  return 0;
}

int MPI_Sendrecv(char *sendbuf, int sendcount, int send_sizeofDtype, int dest, int tag, char *recvbuf, int recvcount, int recv_sizeofDtype, int source, int recvtag){

    SERVERACK = 0;

    CLIENTSTART = 0;

    IndexStore ind = {source, recvbuf, recvcount};

    if(pthread_create(&server_thread , NULL , server_listen_fd , (void *)&ind) < 0) error("Could Not create Server Thread");

    //printf("Number Bytes Written is %d\n", sendcount);

    int numBytesSent = 0;

    int numWrite;

    numWrite = write(serverFd[dest], sendbuf, sendcount);
    if (numWrite <= 0) {error("Error in Writing to the socket!!!\n");}
    else if (numWrite < sendcount) {
       numBytesSent += numWrite;
       wait(0.000001);
       //printf("Not all the bytes have been written, requested for %d, but could only able to send %d\n", sendcount, numWrite);
    }
    //break;

    pthread_mutex_lock(&m_server_sync);
    while(SERVERACK == 0){
        pthread_cond_wait(&cv_server_sync, &m_server_sync);
    }
    pthread_mutex_unlock(&m_server_sync);

    char ack[] = ACK;

    numWrite = write(serverFd[dest], ack, ACK_SIZE);

    if (numWrite <= 0 && errno != ECONNRESET) {error("Error in Writing Acknoeledgement to the socket!!!\n");}
    else if (numWrite < ACK_SIZE && errno != ECONNRESET) {
       printf("Not all the bytes have been written, requested for %d, but could only able to send %d", sendcount, numWrite);
    }


    pthread_mutex_lock(&m_server_sync);
    CLIENTSTART = 1;
    pthread_cond_signal(&cv_server_sync);
    pthread_mutex_unlock(&m_server_sync);

    void * ptr = NULL;
    pthread_join(server_thread, &ptr);

    return 0;
}

int MPI_Finalize() {
  MPI_Barrier();
  shutdownServer();
  shutdownClient();
  return 0;
}

int MPI_Barrier(){

  char readArr[] = ACK;
  char msgArr[] = ACK;

  if (RANK == 0) {

      //set of socket descriptors
      fd_set readfds;

      int numAck, i, sd, activity;

      int ackArr[NUMPROC];

      memset(ackArr, 0, NUMPROC*sizeof(int));

      numAck = 1;

      ackArr[RANK] = 1;

      int timer = 0;

      while(1)
      {
          // Clear FD_Sets;
          FD_ZERO(&readfds);

          int MAXCLIENTFD;

          for ( i = 0 ; i < NUMPROC ; i++)
          {
              if (i==RANK) continue;
              //socket descriptor
              sd = clientFd[i];
              //if valid socket descriptor then add to read list
              if(sd > 0 && ackArr[i] == 0)
                FD_SET(sd , &readfds);

              //highest file descriptor number, need it for the select function
              if(sd > MAXCLIENTFD)
                MAXCLIENTFD = sd;
          }

          activity = select(MAXCLIENTFD + 1 , &readfds , NULL , NULL , NULL);

          for ( i = 0 ; i < NUMPROC ; i++)
          {
              if (ackArr[i] == 1) continue;

              //socket descripto
              sd = clientFd[i];

              if (FD_ISSET(sd , &readfds)){
                  read(sd, readArr, ACK_SIZE);
                  numAck += 1;
                  ackArr[i] = 1;
              }
          }

          for ( i = 0 ; i < NUMPROC ; i++)
          {
              if (i == RANK) continue;

              //socket descripto
              sd = serverFd[i];

              write(sd, msgArr, ACK_SIZE);
          }

          timer++;

          if (timer > 10){
            break;
          }

          if (numAck == NUMPROC) break;
      }

  }

  else{
        int errSend = write(serverFd[0], msgArr, ACK_SIZE);

        int errRecv = read(clientFd[0], msgArr, ACK_SIZE);
  }

  //printf("Barrier has been Crossed by Node %d\n", RANK);

}

// int main(int argc, char **argv){
//
//     int rank;
//     int numproc;
//
//     MPI_Init(argc, argv, &rank, &numproc);
//
//     char msgPtr[65];
//     char rcvPtr[65];
//
//     memset(rcvPtr, 'L', 64*sizeof(char));
//     rcvPtr[64] = '\0';
//
//     if (rank%2 == 0){
//       memset(msgPtr, 'E', 64*sizeof(char));
//       msgPtr[64] = '\0';
//       MPI_Sendrecv(&msgPtr, 65, 1, rank+1, 1, &rcvPtr, 65, 1, rank+1, 1);
//     }
//     else{
//       memset(msgPtr, 'O', 64*sizeof(char));
//       msgPtr[64] = '\0';
//       MPI_Sendrecv(&msgPtr, 65, 1, rank-1, 1, &rcvPtr, 65, 1, rank-1, 1);
//     }
//
//     MPI_Finalize();
//
//
// }
