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
char nameFILE[NODE_NAME_LEN];
pthread_t server_thread, client_thread;

int * SERVERHALT;
int * CLIENTSTART;

void error(const char *msg) {
    perror(msg);
    //exit(1);
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

int MPI_Init(int argc, char **argv, int *rank, int *numproc) {

  CLIENTSTART = (int *)calloc(1, sizeof(int));

  if(argc == 4) {
      RANK = atoi(argv[1]); //Rank of the node you are in.
      NUMPROC = atoi(argv[2]); //Number of total nodes/processes.
      strcpy(nameFILE, argv[3]); // Name of the node file.
  }
  else error("Please pass the expected number of arguments which is four\n");

  fileAsArray(nameFILE, NUMPROC); // To load names of the nodes into an array

  /* Server SetUp Code */
  struct hostent* host;
  struct sockaddr_in serv_addr;
  int server_sockfd;

  host =  gethostbyname((const char *)nodeNames[RANK]);

  server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sockfd < 0) error("ERROR opening socket");

  if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) error("setsockopt(SO_REUSEADDR) failed");

  bzero((char *) &serv_addr, sizeof(serv_addr));

  bcopy((char *)host->h_addr, (char *)&serv_addr.sin_addr.s_addr, host->h_length);

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVERPORT);

  if (bind(server_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
     perror("ERROR on binding\n");
     exit(1);
  }

  listen(server_sockfd, MAXCONNECT-1);
  /* Server SetUp Code */

  if( pthread_create(&server_thread , NULL ,  server_connection_handler , (void*) &server_sockfd) < 0) error("Could Not create Server Thread");

  int cret, r_tosend;
  int client_sockfd;
  client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sockfd < 0) error("ERROR opening socket");

  if (RANK == 1){
    r_tosend = 0;
    //cret = pthread_create(&client_thread, NULL, client_connection_handler, (void *)&r_tosend);
    client_connection_handler(&client_sockfd, &r_tosend);
  }
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

void *server_connection_handler(void *fd) {

    //Get the socket descriptor
    struct sockaddr_in client_addr;
    bzero((char *) &client_addr, sizeof(client_addr));

    int server_sockfd = *(int *)fd;

    char *client_message = calloc(2048, sizeof(char));

    int accept_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (accept_sockfd < 0) error("ERROR opening socket");

    socklen_t clilen = sizeof(client_addr);

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
