# myMPI
Custom MPI library using Socket Programming and Multi-Threading in c.

## MPI: 
Message Passing Interface is a standard designed to use inter-processes communication across the compute nodes to achieve SIMD paralellism. References for Commercial Libraries: https://www.open-mpi.org/

## MPI Funtions Implemented in this custom implementation.
1) MPI_Init()
2) MPI_Sendrecv()
3) MPI_Finalize()
4) MPI_Barrier()

## Implementational Details:
1) Although, File System is shared across the nodes, it is not used for Synchronization or any sort of communication across the nodes. (Only used to read nodefile.txt in the beggining to get the names of the nodes)
2) Server-Client Architecture is used for communication. Server is only for Reading the Messages and only the Client Will send the Messages.
3) Sever is single threaded and handles multiple connections using select() and its related functions.
4) Mutex Locks and Conditional Variables are used for synchronization among server thread and main thread.

Note: Clear Instructions and results are found in p1.README file
