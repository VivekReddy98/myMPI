
mpi:
	gcc -w -lpthread my_mpi.c my_mpi_helper.c -o ./my_mpi.elf

run:
	./my_prun.sh ./my_mpi.elf

clean:
	rm my_mpi.elf
