
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <locale.h>

// interface graphique
#include "graph.h"
#include "functions.h"


// nombre max d'itérations
#define ITER 10000

/* longueur cycle recherche - 1 */
#define LONGCYCLE 11

int main(int argc, char* argv[argc+1]) {
  setlocale(LC_ALL, "");
  struct timeval tv_init, tv_end;

  size_t hm = 240;
  size_t lm = 400;
  switch (argc) {
  default:;
    lm = strtoull(argv[2], 0, 0);
  case 2:;
    hm = strtoull(argv[1], 0, 0);
  case 1:;
  case 0:;
  }

  // allocation dynamique sinon stack overflow...
  char (*tt)[hm][lm] = calloc(sizeof(char[hm][lm]), LONGCYCLE);  // tableau de tableaux

  /* initialisation du premier tableau */
  init(hm, lm, tt[0]);

  /* init interface graphique */
  initgraph(lm*(CARRE+INTER), hm*(CARRE+INTER));
  vert = newcolor(0.,0.6,0.);
  noir = newcolor(0.,0.,0.);

  /* mesure temps */
  gettimeofday(&tv_init, 0);

  /* itérations */
  for (size_t i=0 ; i<ITER ; i++) {
    affiche(hm, lm, tt[i%LONGCYCLE]);
    calcnouv(hm, lm, tt[i%LONGCYCLE], tt[(i+1)%LONGCYCLE], 0, hm);

    // à commenter pour ne pas attendre entre deux itérations
    // if (waitgraph() == XK_q)
    //   break;                  // l'utilisateur a appuyé la touche 'q'

  }

  gettimeofday(&tv_end, 0);
  /* temps d'exécution */
  printf("Temps : %lfs.\n", DIFFTEMPS(tv_init,tv_end));

  /* fermeture interface graphique */
  closegraph();
  free(tt);
  return EXIT_SUCCESS;
}
