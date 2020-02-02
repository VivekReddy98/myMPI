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

int clientFd[MAXCONNECT]; // Used by the Client
int serverFd[MAXCONNECT]; // Used by Server

int *SERVERHALT;
int CLIENTSTART;

char sysBuffer[MAXMSGSIZE];

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

  pthread_cancel(server_starting_thread);

  return 0;
}

int MPI_Sendrecv(void *sendbuf, int sendcount, int send_sizeofDtype, int dest, int tag, void *recvbuf, int recvcount, int recv_sizeofDtype, int source, int recvtag){

}

void* server_read(void *ptr){

  // while(TRUE)
  // {
  //   // Clear FD_Sets;
  //   FD_ZERO(&readfds);
  //
  //   FD_SET(serverFd[RANK], &readfds);
  //
  //   MAXCLIENTFD = clientFd[RANK];
  //
  //   //add child sockets to set
  //   for ( i = 0 ; i < NUMPROC ; i++)
  //   {
  //       //socket descriptor
  //       sd = clientFd[i];
  //
  //       //if valid socket descriptor then add to read list
  //       if(sd > 0)
  //         FD_SET(sd , &readfds);
  //
  //       //highest file descriptor number, need it for the select function
  //       if(sd > MAXCLIENTFD)
  //         MAXCLIENTFD = sd;
  //   }
  //
  //     //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
  //     activity = select( max_sd + 1 , &amp;readfds , NULL , NULL , NULL);
  //
  //     if ((activity &lt; 0) &amp;&amp; (errno!=EINTR))
  //     {
  //         printf(&quot;select error&quot;);
  //     }
  //
  //
  //     //else its some IO operation on some other socket :)
  //     for (i = 0; i &lt; max_clients; i++)
  //     {
  //         sd = client_socket[i];
  //
  //         if (FD_ISSET( sd , &amp;readfds))
  //         {
  //             //Check if it was for closing , and also read the incoming message
  //             if ((valread = read( sd , buffer, 1024)) == 0)
  //             {
  //                 //Somebody disconnected , get his details and print
  //                 getpeername(sd , (struct sockaddr*)&amp;address , (socklen_t*)&amp;addrlen);
  //                 printf(&quot;Host disconnected , ip %s , port %d \n&quot; , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
  //
  //                 //Close the socket and mark as 0 in list for reuse
  //                 close( sd );
  //                 client_socket[i] = 0;
  //             }
  //
  //             //Echo back the message that came in
  //             else
  //             {
  //                 //set the string terminating NULL byte on the end of the data read
  //                 buffer[valread] = '&#92;&#48;';
  //                 send(sd , buffer , strlen(buffer) , 0 );
  //             }
  //         }
  //     }
  // }
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
