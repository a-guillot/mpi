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
      if (egal(number_of_lines, lm, tt[(i+1)%LONGCYCLE], tt[(i+1+j)%LONGCYCLE],
          offset, number_of_lines)) {
        // on a trouvé le tableau identique !
        gettimeofday(&tv_end, 0);
        printf("Cycle trouvé : iteration %zu, longueur %zu\n",
                i+1-(LONGCYCLE-j),
                LONGCYCLE-j);
        printf("Calcul : %lfs.\n", DIFFTEMPS(tv_init,tv_end));
        goto CLEANUP;
      }
  }

  gettimeofday(&tv_end, 0);
  printf("pas de cycle trouvé en %d itérations\n", ITER);
  printf("Calcul : %lfs.\n", DIFFTEMPS(tv_init,tv_end));

 CLEANUP:
  free(tt);
  /* Fin du programme */
  MPI_Finalize();
  return EXIT_SUCCESS;
}
