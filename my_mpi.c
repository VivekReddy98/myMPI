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

int MPI_Init(int argc, char **argv, int *rank, int *numproc) {

  if(argc == 4) {
      RANK = atoi(argv[1]); //Rank of the node you are in.
      NUMPROC = atoi(argv[2]); //Number of total nodes/processes.
      strcpy(nameFILE, argv[3]); // Name of the node file.
  }
  else error("Please pass the expected number of arguments which is four\n");

  fileAsArray(nameFILE, NUMPROC); // To load names of the nodes into an array

  pthread_t server_thread, client_thread;
  int sret, cret;

  sret = pthread_create(&server_thread, NULL, server_connection_handler, NULL);

  int r_tosend;

  if (RANK == 0){
    r_tosend = 1;
    cret = pthread_create(&client_thread, NULL, client_connection_handler, (void *)&r_tosend);
  }
  else{
    r_tosend = 0;
    cret = pthread_create(&client_thread, NULL, client_connection_handler, (void *)&r_tosend);
  }

  pthread_join(&server_thread, NULL);
  pthread_join(&client_thread, NULL);

  printf("Thread 1 returns: %d, rank %d\n",sret);
  printf("Thread 2 returns: %d, rank %d\n",cret);

  return 0;

}

void *client_connection_handler(void *rank) {

  int rank_server = *(int *)rank;

  //Get the socket descriptor
  int read_size_c, connfd_c;
  char message[2048];

  // Get the Server Address
  struct hostent* host_s;
  struct sockaddr_in serv_addr;
  host_s = gethostbyname((const char *)nodeNames[rank_server]);
  memcpy(&serv_addr.sin_addr, host_s->h_addr_list[0], host_s->h_length);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SERVERPORT);

  // Get the Client Socket
  int client_sockfd;
  client_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (client_sockfd < 0) error("ERROR opening socket");

  connect(client_sockfd, (const struct sockaddr_in*)&serv_addr, host_s->h_length);

  if (RANK == 0) {
    char message[] = "Message from Node 0";
  }
  else {char message[] = "Message from Node 1";}

  int n = write(client_sockfd, message, strlen(message));

  if (n < 0) error("ERROR writing to socket \n");

  close(client_sockfd);
}


void *server_connection_handler(void *ptr) {

    //Get the socket descriptor
    int read_size, connfd;
    char client_message[2048];

    struct hostent* host;
    struct sockaddr_in serv_addr, accept_sock;
    int server_sockfd;

    host =  gethostbyname((const char *)nodeNames[RANK]);

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_sockfd < 0) error("ERROR opening socket");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    bzero((char *) &accept_sock, sizeof(accept_sock));

    memcpy(&serv_addr.sin_addr, host->h_addr_list[0], host->h_length);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVERPORT);

    if (bind(server_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
       perror("ERROR on binding\n");
       exit(1);
    }

    listen(server_sockfd, MAXCONNECT-1);

    //Receive a message from client
    while(1)
    {
        connfd = accept(server_sockfd, (struct sockaddr*)&accept_sock, NULL);
        if (connfd < 0) {
          error("Error in accepting connections \n");
        }
        else {
            int numRead = read(server_sockfd, client_message, 2048);
            if (numRead < 0) error("Error in Reading Message\n");
            else{
              for (int i = 0; i<numRead; i++){
                  printf("%c", client_message[i]);
              }
            }
            //clear the message buffer
        		memset(client_message, 0, 2000);
        }
        sleep(0.1);
    }

    close(server_sockfd);
}

int main(int argc, char **argv){

    int rank = 0;
    int numproc = 0;

    MPI_Init(argc, argv, &rank, &numproc);

    // int i;
    // for (i=0; i<NUMPROC; i++) {
    //     printf("Node: %s, Rank %d\n", nodeNames[i], RANK);
    // }
}
