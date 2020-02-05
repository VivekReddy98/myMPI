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
#include <errno.h>
#include "my_mpi.h"

// Helper Functions for core MPI Functions

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

void *client_connection_handler(void *ptr, int odd) {

  int rank_server = 0;
  int offset = 2;

  if (odd){
     rank_server = 1;
  }

  struct sockaddr_in serv_addr;

  for( ; rank_server < NUMPROC; rank_server += offset){
    if (rank_server == RANK) continue;

    serv_addr = getServerAddr(rank_server); // Get the Server Address
    serverFd[rank_server] = socket(AF_INET, SOCK_STREAM, 0);

    if (serverFd[rank_server] < 0) error("ERROR opening socket");

    int connect_err = -1;
    int limit = 0;
    while (connect_err < 0){
      serverFd[rank_server] = socket(AF_INET, SOCK_STREAM, 0);
      if (serverFd[rank_server] < 0) error("ERROR opening socket");

      connect_err = connect(serverFd[rank_server], (struct sockaddr *) &serv_addr, sizeof(serv_addr));

      if (connect_err < 0) {
        //perror("Error in connecting to the server: ");
      }
      else{
        //printf("Connected to server, socket fd is %d , ip is : %s , port : %d, Rank: %d\n" , clientFd[rank_server] , inet_ntoa(serv_addr.sin_addr) , ntohs(serv_addr.sin_port), RANK);
        //
        break;
      }
      sleep(1);
      limit++;
      if(limit > 10){
        printf("Connection with Server of the Node: %d did not happen at RANK: %d", rank_server, RANK);
        fflush(stdout);
        break;
      }
    }
  }

  // printf("Connected With all the servers!!!!!!!!!!!!!! %d\n", RANK);
  // fflush(stdout);

  return NULL;
  //pthread_exit(NULL);
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
        //printf("New connection , socket fd is %d , ip is : %s , Rank: %d\n" , accept_sockfd , inet_ntoa(client_addr.sin_addr), RANK);
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

void *server_listen_fd(void *ind){

    IndexStore* in = (IndexStore *)ind;

    int src = in->src;
    char *rcvBuff = in->rcvBuff;
    int recvcount = in->recvcount;

    fflush(stdout);
    int fd = clientFd[src];

    if (fd <= 0) error("No Connection established between my rank of %d and other node of %d");

    int numBytesRead = 0;
    int i;

    while(1) {

        i = read(fd+numBytesRead, rcvBuff, recvcount);

        if (i>0){
          recvcount -= i;
          numBytesRead += i;
          if (numBytesRead >= recvcount) {
            printf("SUCESS: Was able to read %d bytes %d \n", numBytesRead, RANK);
            break;
          }
        }

        if (i <= 0 && errno == ETIMEDOUT) {
          continue;
        }
        else if (i <= 0 && errno != ETIMEDOUT){
           error("Something Wrong with the Connection\n");
        }
    }

    printf("Message Recieved Was: %s\n", rcvBuff);
    fflush(stdout);

    pthread_mutex_lock(&m_server_sync);
    SERVERACK = 1;
    pthread_cond_signal(&cv_server_sync);
    pthread_mutex_unlock(&m_server_sync);

    pthread_mutex_lock(&m_server_sync);
    while(CLIENTSTART == 0){
        pthread_cond_wait(&cv_server_sync, &m_server_sync);
    }
    pthread_mutex_unlock(&m_server_sync);

    i = read(fd, rcvBuff, ACK_SIZE);

    printf("%s\n", rcvBuff);

    if (i > ACK_SIZE){
        printf("Acknowledgement Recieved!! Aborting the execution \n");
        fflush(stdout);
    }

    pthread_exit(NULL);
}
