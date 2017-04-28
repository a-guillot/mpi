#!/bin/bash

TIMEFORMAT="%U"

for i in 1 2 3 4 5
do
	for j in 1 2 4 8
	do
		# { time mpiexec -n $j gvie_cycle_mpi$i $k $k ; } 2>> v${i}_p${j}_d${k}.res
		awk '{sum+=$1; sumsq+=$1*$1} END {print sum/NR "/" sqrt(sumsq/NR - (sum/NR)^2)}' v${i}_p${j}.res >> v${i}_p${j}.mstd
	done
done

for i in 1 2 3 4 5
do
	paste v${i}_p1.mstd v${i}_p2.mstd v${i}_p4.mstd v${i}_p8.mstd | column -s $'\t' -t > vf${i}.tab
done
