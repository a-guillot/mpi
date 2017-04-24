CC=mpicc
PG=
CFLAGS= -O3 -Wall $(PG)
LDLIBS= -lX11 -L/usr/X11R6/lib $(PG)

e= gvie gvie_cycle gvie_cycle_mpi1 gvie_cycle_mpi2

all: $(e)

gvie: graph.o functions.o
gvie_cycle: graph.o functions.o
gvie_cycle_mpi1: graph.o functions.o
gvie_cycle_mpi2: graph.o functions.o
gvie_cycle_mpi3: graph.o functions.o

clean:
	/bin/rm -f $(e) *.o gmon.out

archive:
	zip GUILLOT_andreas.zip -r *.c *.h rapport.txt
