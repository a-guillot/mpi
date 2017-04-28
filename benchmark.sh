#!/bin/bash

TIMEFORMAT="%U"

for i in 1 2 3 4 5
do
	for j in 1 2 6 10
	do
		echo "ver:$i np:$j"
		{ time mpiexec -n $j ./gvie_cycle_mpi$i ; } 2>> v${i}_p${j}.res
		{ time mpiexec -n $j ./gvie_cycle_mpi$i ; } 2>> v${i}_p${j}.res
		{ time mpiexec -n $j ./gvie_cycle_mpi$i ; } 2>> v${i}_p${j}.res
		{ time mpiexec -n $j ./gvie_cycle_mpi$i ; } 2>> v${i}_p${j}.res
		{ time mpiexec -n $j ./gvie_cycle_mpi$i ; } 2>> v${i}_p${j}.res
	done
done
