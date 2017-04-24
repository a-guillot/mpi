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

int ppcm(int count, int *value)
{
  if (count <= 0)
    return -1;

  int res = value[0];
  for (size_t i = 1; i < count; i++)
    res = _ppcm(res, value[i]);

  return res;
}

void ppcm_reduce(int * in, int * inout, int * len, MPI_Datatype *dptr)
{
  int res = _ppcm(*in, *inout);
  *inout = res;
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

  if (hm <= 200) {
    printf("ERROR : hm trop petit.\n");
    exit(1);
  }

  if (lm <= 200) {
    printf("ERROR : lm trop petit.\n");
    exit(1);
  }

  /* Initialisation d'MPI */
  MPI_Init(&argc, &argv);

  /* Récupère le numéro de rank */
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* Le nombre de processus */
  int size = 0;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  /* Vérification si le nombre de ligne est divisible par le nombre de processus */
  if (lm%size != 0) {
    printf("Le nombre de ligne n'est pas divisible par le nombre de processus\n");
    exit(EXIT_FAILURE);
  }

  /* Nombre de ligne par processus */
  unsigned int nb_line_per_proc = hm/size;

  MPI_Op operation;
  MPI_Datatype type;
  MPI_Type_contiguous(1, MPI_INT, &type);
  MPI_Type_commit(&type);

  MPI_Op_create((MPI_User_function *) ppcm_reduce, 1, &operation);

  /*
  L'indice de départ de chaque processus
  Rank 0 : 0 * nb_line_per_proc = 0
  Rank 1 : 1 * nb_line_per_proc
  ...
  */
  size_t offset = rank * nb_line_per_proc;


  // allocation dynamique sinon stack overflow...
  char (*tt)[hm][lm] = calloc(sizeof(char[hm][lm]), LONGCYCLE); // tableau de tableaux

  /* initialisation du premier tableau */
  init(hm, lm, tt[0]);

  gettimeofday(&tv_init, 0);
  for (size_t i=0 ; i<ITER ; i++) {
    /* calcul du nouveau tableau i+1 en fonction du tableau i */
    /* Chaque processus calcul le même nombre d'itération et le travail est répartit */
    calcnouv(nb_line_per_proc, lm, tt[i%LONGCYCLE], tt[(i+1)%LONGCYCLE], offset, nb_line_per_proc);

    /* comparaison du nouveau tableau avec les (LONGCYCLE-1) précédents */
    for (size_t j=LONGCYCLE-1; j>0; j--)
      if (egal(nb_line_per_proc, lm, tt[(i+1)%LONGCYCLE], tt[(i+1+j)%LONGCYCLE], 0, nb_line_per_proc)) {
        // on a trouvé le tableau identique !
        gettimeofday(&tv_end, 0);
        int longueur = LONGCYCLE-j;
        printf("Cycle trouvé : iteration %zu, longueur %d\n",
                i+1-(LONGCYCLE-j),
                longueur);
        printf("Calcul : %lfs.\n", DIFFTEMPS(tv_init,tv_end));

        int res;

        MPI_Reduce(&longueur, &res, 1, MPI_INT, operation, 0,
           MPI_COMM_WORLD);

        if (rank == 0)
          printf("ppcm : %d\n", res);

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
