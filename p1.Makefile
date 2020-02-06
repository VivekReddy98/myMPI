
all: my_mpi.c my_mpi_helper.c my_rtt.c
	gcc -w -lm -lpthread my_rtt.c my_mpi.c my_mpi_helper.c -o ./my_rtt

my_mpi:
	gcc -w -lpthread my_mpi.c my_mpi_helper.c -o ./my_mpi.elf

run:
	./my_prun.sh 2 ./my_rtt

clean:
	rm my_rtt
