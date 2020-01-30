
mpi:
	gcc my_mpi.c -o my_mpi

run:
	./my_prun.sh ./my_mpi

clean:
	rm mp_mpi
