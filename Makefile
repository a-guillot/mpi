CC=mpicc
PG=
CFLAGS= -O3 -Wall $(PG)
LDLIBS= -lX11 -L/usr/X11R6/lib $(PG)

e= gvie gvie_cycle

all: $(e)

gvie: graph.o functions.o
gvie_cycle: graph.o functions.o

clean:
	/bin/rm -f $(e) *.o gmon.out
