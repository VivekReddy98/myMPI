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
#include "my_mpi.h"

extern char nodeNames[MAXCONNECT][NODE_NAME_LEN];
extern int RANK;
extern int NUMPROC;
extern char nameFILE[NODE_NAME_LEN];
extern pthread_t server_thread;
extern int MAXCLIENTFD;

extern int clientFd[MAXCONNECT];
extern int serverFd[MAXCONNECT];

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

void *client_connection_handler(void *client_fd, void *rank) {

  while(!(*CLIENTSTART)){
      sleep(1);
  }

  int client_sockfd = *(int *)client_fd;

  int rank_server = *(int *)rank;

  //Get the socket descriptor
  int read_size_c, connfd_c;

  // Get the Server Address
  struct hostent* host_s;
  struct sockaddr_in serv_addr;
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVERPORT);
  host_s = gethostbyname((const char *)nodeNames[rank_server]);
  bcopy((char *)host_s->h_addr, (char *)&serv_addr.sin_addr.s_addr,host_s->h_length);

  //printf("In Client, Trying to Send a Message !!!!!!!!!!!!, rank %d\n", RANK);
  // printtIP(host_s);
  // fflush(stdout);

  socklen_t serlen = sizeof(serv_addr);

  int connect_err = -1;
  while (connect_err < 0){
    connect_err = connect(client_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (connect_err < 0) {
      perror("Error in connecting to the server ");
    }
    else{
      printf("Connected !! Proceeding\n");
      fflush(stdout);
    }
    sleep(1);
  }

  char *message;
  if (RANK == 0) {
    char message[] = "Message from SOme M**** F***** Node  0\n ";
  }
  else{
    char message[] = "Message from SOme M**** F***** Node  1\n ";
  }


  int n = write(client_sockfd, message, strlen(message));
  if (n < 0) perror("ERROR Writing to Socket ");

  printf("Message to be sent: %s", message);
  fflush(stdout);
  return NULL;
  //pthread_exit(NULL);
}

struct sockaddr_in * getServerAddr(int rk){
  /* Server SetUp Code */
  struct hostent* host;
  struct sockaddr_in serv_addr;

  host =  gethostbyname((const char *)nodeNames[rk]);

  bzero((char *) &serv_addr, sizeof(serv_addr));
  bcopy((char *)host->h_addr, (char *)&serv_addr.sin_addr.s_addr, host->h_length);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVERPORT);

  return &serv_addr;
}

void startServer(struct sockaddr_in *serv_addr){

  clientFd[RANK] = socket(AF_INET, SOCK_STREAM, 0);
  if (clientFd[RANK]< 0) error("ERROR opening socket");

  if (setsockopt(clientFd[RANK], SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) error("setsockopt(SO_REUSEADDR) failed");

  if (bind(clientFd[RANK], (struct sockaddr *) serv_addr, sizeof(*serv_addr)) < 0) {
     perror("ERROR on binding\n");
     exit(1);
  }
  listen(clientFd[RANK], MAXCONNECT-1);
}

int getRankFromIPaddr(struct sockaddr_in *serv_addr){

    char recievedIP[INET_ADDRSTRLEN];

    inet_ntop(AF_INET, &(serv_addr->sin_addr), recievedIP, INET_ADDRSTRLEN);

    int i;
    char tempIP[INET_ADDRSTRLEN];
    struct sockaddr_in *temp;
    for(i=0; i<NUMPROC; i++){
       temp = getServerAddr(i);
       inet_ntop(AF_INET, &(temp->sin_addr), tempIP, INET_ADDRSTRLEN);
       if(!strcmp((const char *)tempIP, (const char *)recievedIP)){
         return i;
       }
    }
    error("Client Not Specified in the NODELIST");
}
