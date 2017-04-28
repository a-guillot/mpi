#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>
#include <mpi.h>
#include "check.h"
#include <string.h>

#include "functions.h"

// nombre maximal d'itérations
#define ITER 10000

/* longueur cycle recherche de cycle (-1) */
#define LONGCYCLE 51

int _lcm(int n1, int n2)
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

/* Each time a lcm is received : current_lcm = lcm(current_lcm, received_lcm) */
void lcm_reduce(int * in, int * inout, int * len, MPI_Datatype *dptr)
{
  int res = _lcm(*in, *inout);
  *inout = res;
}

/* Uses commutativity: lcm(a, b, c) = lcm(a, lcm(b, c)) */
int lcm(int count, int *value)
{
  if (count <= 0)
    return -1;

  int res = value[0];
  for (size_t i = 1; i < count; i++)
    res = _lcm(res, value[i]);

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

  /* Creating an mpi operator that will compute the lcm */
  MPI_Op operation;
  MPI_Datatype type;

  // The operator takes an int as an argument
  CHECK((MPI_Type_contiguous(1, MPI_INT, &type)) == MPI_SUCCESS);
  CHECK((MPI_Type_commit(&type)) == MPI_SUCCESS);
  CHECK((MPI_Op_create((MPI_User_function *) lcm_reduce, 1, &operation))
      == MPI_SUCCESS);

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

  int done = 0, length;

  for (size_t i=0 ; i<ITER ; i++) {
    /* calcul du nouveau tableau i+1 en fonction du tableau i */

    /* Now that we want to communicate with the peers we've got to sned our
      borders to the other processes. We can do it asynchronously in order to
      maximize the potential speed */
    MPI_Request request;

    if (rank) // rank != 0 -> send first line as a border
    {
      CHECK((MPI_Isend(tt[i%LONGCYCLE][offset], lm, MPI_CHAR, (rank - 1), 0,
          MPI_COMM_WORLD, &request)) == MPI_SUCCESS);

    }
    if (rank != (size - 1)) // if not last one send last line
    {
      CHECK((MPI_Isend(tt[i%LONGCYCLE][(offset + number_of_lines -1)], lm, MPI_CHAR, (rank + 1), 0,
          MPI_COMM_WORLD, &request)) == MPI_SUCCESS);
    }

    /*Now that we've sent our borders we can compute everything except borders*/
    calcnouv(hm, lm, tt[i%LONGCYCLE], tt[(i+1)%LONGCYCLE],
            (offset + 1), (number_of_lines - 2));

    /* Receive borders */
    MPI_Request request1;
    MPI_Request request2;
    MPI_Status s;

    if (rank) // rank != 0 -> receive first line
      CHECK((MPI_Irecv(tt[i%LONGCYCLE][offset-1], lm, MPI_CHAR, (rank - 1), 0,
          MPI_COMM_WORLD, &request1)) == MPI_SUCCESS);

    if (rank != (size - 1)) // if not last one receive last line
      CHECK((MPI_Irecv(tt[i%LONGCYCLE][(offset + number_of_lines)], lm, MPI_CHAR, (rank + 1), 0,
          MPI_COMM_WORLD, &request2)) == MPI_SUCCESS);

    if (rank)
    {
      MPI_Wait(&request1, &s);

      calcnouv(hm, lm, tt[i%LONGCYCLE], tt[(i+1)%LONGCYCLE],
              offset, 1);
    }
    if (rank != (size - 1))
    {
      MPI_Wait(&request2, &s);

      calcnouv(hm, lm, tt[i%LONGCYCLE], tt[(i+1)%LONGCYCLE],
            (offset + number_of_lines - 1), 1);
    }

    /* comparaison du nouveau tableau avec les (LONGCYCLE-1) précédents */
    if (!done)
    {
      for (size_t j=LONGCYCLE-1; j>0; j--)
      {
        if (egal(hm, lm, tt[(i+1)%LONGCYCLE], tt[(i+1+j)%LONGCYCLE],
            offset, number_of_lines))
          {
          // on a trouvé le tableau identique !
          gettimeofday(&tv_end, 0);
          length = LONGCYCLE-j;

          done = 1;
          break;
        }
      }
    }

    /* Test if all processes are over by looking at how many of them are done */
    int over;
    CHECK((MPI_Allreduce(&done, &over, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD))
        == MPI_SUCCESS);

    if (over == size) // everybody is done
    {
      /* If not the root process:
          - Send computed lcm (length) to root precess
         If root process:
          - Compute lcm on received values and store it in res */
      int res;
      MPI_Reduce(&length, &res, 1, MPI_INT, operation, 0,
         MPI_COMM_WORLD);

      if (!rank)
        printf("lcm: %d\n", res);

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
  if (!rank)
    printf("Calcul : %lfs.\n", DIFFTEMPS(tv_init,tv_end));
  return EXIT_SUCCESS;
}
