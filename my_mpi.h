
/* --------------------Define  Constants here ----------------*/
#ifndef MAXCONNECT
#define MAXCONNECT 16
#endif

#ifndef NODE_NAME_LEN
#define NODE_NAME_LEN 10
#endif

#ifndef CLIENTPORT
#define CLIENTPORT 8999  // Client Port on every Node
#endif

#ifndef SERVERPORT
#define SERVERPORT 8989  // Server Port on every Node
#endif

#ifndef MAXMSGSIZE
#define MAXMSGSIZE 2097153  // Max System Buffer size to recieve messages, 2 MB + 1 bytes.
#endif
/* --------------------------------------------------------------- */


/* ------------Define Data Structure Required here --------------- */

/* Array To store the names of nodes */
extern char nodeNames[MAXCONNECT][NODE_NAME_LEN];
extern int serverFd[MAXCONNECT];
extern int clientFd[MAXCONNECT];

extern pthread_t server_thread;
extern pthread_cond_t cv_server_sync;
extern pthread_mutex_t m_server_sync;

#ifndef RANK
extern int RANK;
#endif

#ifndef NUMPROC
extern int NUMPROC;
#endif

#ifndef nameFILE
extern char nameFILE[NODE_NAME_LEN];
#endif

#ifndef MAXCLIENTFD
extern int MAXCLIENTFD;
#endif

/* --------------------------------------------------------------- */


/* --------------------Function Declarations here --------------- */
/* --------------------Function Declarations here --------------- */
/* Helper Functions */
void error(const char *msg);
void fileAsArray(char *filename, int numLines); // To read stuff from a line
void printtIP(struct hostent* host);
void *server_connection_handler(void *ptr); // Threaded Function used by pthread to create a server
void *client_connection_handler(void *ptr, int odd); // Threaded Function used by pthread to create a client
struct sockaddr_in getServerAddr(int rank);
void startServer(struct sockaddr_in serv_addr);
int getRankFromIPaddr(struct sockaddr_in *serv_addr); // To get the rank of the connection from a recieved Node.
void shutdownServer();
void shutdownClient();

/* Core MPI Functions */
int MPI_Init(int argc, char **argv, int *rank, int *numproc);
int MPI_Sendrecv(void *sendbuf, int sendcount, int send_sizeofDtype, int dest, int tag, void *recvbuf, int recvcount, int recv_sizeofDtype, int source, int recvtag);
int MPI_Finalize();
/* --------------------------------------------------------------- */
