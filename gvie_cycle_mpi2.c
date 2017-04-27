#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>
#include <mpi.h>
#include "check.h"

#include "functions.h"

// nombre maximal d'itérations
#define ITER 10000

/* longueur cycle recherche de cycle (-1) */
#define LONGCYCLE 51

int _ppcm(int n1, int n2)
{
  int minMultiple;
  minMultiple = (n1>n2) ? n1 : n2;

  while(1)
  {
      if( minMultiple%n1==0 && minMultiple%n2==0 )
          return minMultiple;
      ++minMultiple;
  }
}

/* Uses commutativity: lcm(a, b, c) = lcm(a, lcm(b, c)) */
int ppcm(int count, int *value)
{
  if (count <= 0)
    return -1;

  int res = value[0];
  for (size_t i = 1; i < count; i++)
    res = _ppcm(res, value[i]);

  return res;
}


int main(int argc, char* argv[argc+1]) {
  setlocale(LC_ALL, "");
  struct timeval tv_init, tv_end;

  size_t hm = 600;
  size_t lm = 800;
  switch (argc) {
  default:;
    lm = strtoull(argv[2], 0, 0);
  case 2:;
    hm = strtoull(argv[1], 0, 0);
  case 1:;
  case 0:;
  }

  /* MPI initialization */
  CHECK((MPI_Init(&argc, &argv)) == MPI_SUCCESS);

  /* Get process rank and number of processes */
  int rank, size;
  CHECK((MPI_Comm_rank(MPI_COMM_WORLD, &rank)) == MPI_SUCCESS);
  CHECK((MPI_Comm_size(MPI_COMM_WORLD, &size)) == MPI_SUCCESS);

  /* Define where a process will start, and for how many lines */
  unsigned int number_of_lines = hm/size;
  size_t offset = rank * number_of_lines;

  /* If the number of lines isn't divisible be the number of processes,
      the last one is going to do more work.
  */
  if ((rank == (size - 1)) && ((hm % size) != 0))
    number_of_lines += hm % size;

  // allocation dynamique sinon stack overflow...
  char (*tt)[hm][lm] = calloc(sizeof(char[hm][lm]), LONGCYCLE); // tableau de tableaux

  /* initialisation du premier tableau */
  init(hm, lm, tt[0]);
  gettimeofday(&tv_init, 0);

  for (size_t i=0 ; i<ITER ; i++) {
    /* calcul du nouveau tableau i+1 en fonction du tableau i */

    /* Each process computes [number_of_lines] lines starting from [offset]*/
    calcnouv(hm, lm, tt[i%LONGCYCLE], tt[(i+1)%LONGCYCLE],
            offset, number_of_lines);

    /* comparaison du nouveau tableau avec les (LONGCYCLE-1) précédents */
    for (size_t j=LONGCYCLE-1; j>0; j--)
      if (egal(hm, lm, tt[(i+1)%LONGCYCLE], tt[(i+1+j)%LONGCYCLE],
          offset, number_of_lines)) {
        // on a trouvé le tableau identique !
        gettimeofday(&tv_end, 0);
        int length = LONGCYCLE-j;
        printf("Cycle trouvé : iteration %zu, longueur %d\n",
                i+1-(LONGCYCLE-j),
                length);
        printf("Calcul : %lfs.\n", DIFFTEMPS(tv_init,tv_end));

        /* root process computes it's lcm and receives the others */
        if (rank == 0)
        {
          int res;
          MPI_Status status;
          int *array = malloc(size * sizeof(int));
          array[0] = length;

          for (size_t it = 1; it < size; it++)
          {
            /* receive an integer from the process [it] */
            CHECK((MPI_Recv(&res, 1, MPI_INT, it, 0, MPI_COMM_WORLD, &status))
                  == MPI_SUCCESS);
            array[it] = res;
          }
          printf("ppcm total: %d\n", ppcm(size, array));
          free(array);
        }
        /* The other processes compute their lcm and send it to the root proc */
        else
          CHECK((MPI_Ssend(&length, 1, MPI_INT, 0, 0, MPI_COMM_WORLD))
                == MPI_SUCCESS);

        goto CLEANUP;
      }
  }

  gettimeofday(&tv_end, 0);
  printf("pas de cycle trouvé en %d itérations\n", ITER);
  printf("Calcul : %lfs.\n", DIFFTEMPS(tv_init,tv_end));

 CLEANUP:
  free(tt);
  /* Fin du programme */
  CHECK((MPI_Finalize()) == MPI_SUCCESS);
  return EXIT_SUCCESS;
}
