
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
/* --------------------------------------------------------------- */


/* ------------Define Data Structure Required here --------------- */

/* Array To store the names of nodes */
extern char nodeNames[MAXCONNECT][NODE_NAME_LEN];

#ifndef RANK
extern int RANK;
#endif

#ifndef NUMPROC
extern int NUMPROC;
#endif

#ifndef nameFILE
extern char nameFILE[NODE_NAME_LEN];
#endif

/* --------------------------------------------------------------- */


/* --------------------Function Declarations here --------------- */

int MPI_Init(int argc, char **argv, int *rank, int *numproc);
void error(const char *msg);
void fileAsArray(char *filename, int numLines); // To read stuff from a line


/* --------------------------------------------------------------- */
