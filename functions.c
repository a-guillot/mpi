#include "functions.h"
#include "graph.h"

unsigned long noir;
unsigned long vert;

// initialisation d'un tableau qui évolue de manière intéressante
void init(size_t hm, size_t lm, char t[hm][lm]) {
  /*  srand(time(0)); */
  for (size_t i=0 ; i<hm ; i++)
    for (size_t j=0 ; j<lm ; j++) {
//      t[i][j] = rand()%2;
//      t[i][j] = ((i+j)%3==0)?1:0;
//      t[i][j] = (i==0||j==0||i==HM-1||j==LM-1)?0:1;
      t[i][j] = 0;
    }

  /*  // un marcheur en haut à gauche, qui va percuter ...
    t[10][10] = 1;
    t[10][11] = 1;
    t[10][12] = 1;
    t[9][12] = 1;
    t[8][11] = 1;

    // ... une structure qui se stabilise rapidement vers (50,50)
    t[55][50] = 1;
    t[54][51] = 1;
    t[54][52] = 1;
    t[55][53] = 1;
    t[56][50] = 1;
    t[56][51] = 1;
    t[56][52] = 1;
  */

  // un "mathusalhem" : le gland.
  t[150][100] = 1;
  t[151][100] = 1;
  t[151][102] = 1;
  t[153][101] = 1;
  t[154][100] = 1;
  t[155][100] = 1;
  t[156][100] = 1;

  // en bas : une structure avec un cycle de 3 (une croix)
  int croix[][2]= { {0,3}, {1,3}, {1,2}, {1,1} }; // un 8ième de croix
  for (size_t i=0 ; i<4 ; i++) // les 4 coordonnées du tableau
    for (size_t j=0 ; j<4; j++) { // multipliées 4*2 fois
      t[230+((j%2)?(1+croix[i][0]):(-croix[i][0]))][200+((j<2)?(1+croix[i][1]):(-croix[i][1]))] = 1;
      t[230+((j%2)?(1+croix[i][1]):(-croix[i][1]))][200+((j<2)?(1+croix[i][0]):(-croix[i][0]))] = 1;
    }
  // à droite : un octogone à cycle 5
  int octo[][2]= { {0,3}, {1,2} };  // un 8ième de croix
  for (size_t i=0 ; i<2 ; i++) // les 4 coordonnées du tableau
    for (size_t j=0 ; j<4; j++) { // multipliées 4*2 fois
      t[150+((j%2)?(1+octo[i][0]):(-octo[i][0]))][300+((j<2)?(1+octo[i][1]):(-octo[i][1]))] = 1;
      t[150+((j%2)?(1+octo[i][1]):(-octo[i][1]))][300+((j<2)?(1+octo[i][0]):(-octo[i][0]))] = 1;
    }

}

// calcule le nombre de voisins de la case (i,j) du tableau t.
int inline nbvois(size_t hm, size_t lm, char t[hm][lm], size_t i, size_t j) {
  int n=0;
  if (i>0) {
    /* i-1 */
    if (j>0)
      if (t[i-1][j-1])
        n++;
    if (t[i-1][j])
      n++;
    if (j<lm-1)
      if (t[i-1][j+1])
        n++;
  }

  /* i */
  if (j>0)
    if (t[i][j-1])
      n++;
  if (j<lm-1)
    if (t[i][j+1])
      n++;

  if (i<hm-1) {
    /* i+1 */
    if (j>0)
      if (t[i+1][j-1])
        n++;
    if (t[i+1][j])
      n++;
    if (j<lm-1)
      if (t[i+1][j+1])
        n++;
  }
  return n;
}

// calcule une partie d'un nouveau tableau (n) en fonction d'un ancien (t)
// le calcul s'effectue à partire de ligne "offset" pour "lines" lignes
void calcnouv(size_t hm, size_t lm, char t[hm][lm], char n[hm][lm], size_t offset, size_t lines) {
  if (offset>=hm) return;
  if (offset+lines>hm) lines = hm-offset;
  for (size_t  i=offset ; i<offset+lines ; i++) {
    for (size_t j=0 ; j<lm ; j++) {
      size_t v = nbvois(hm, lm, t, i, j);
      if (v==3)
        n[i][j] = (t[i][j])?1:2;
      else if (v==2)
        n[i][j] = (t[i][j])?1:0;
      else
        n[i][j] = 0;
    }
  }
}

// affiche un tableau t
void affiche(size_t hm, size_t lm, char t[hm][lm]) {
  cleargraph();
  for (size_t i=0 ; i<hm ; i++) {
    for (size_t j=0 ; j<lm ; j++) {
      if (t[i][j]==1)
        setcolor(noir);
      else if (t[i][j]==2)
        setcolor(vert);
      if (t[i][j])
        for (size_t k=0 ; k<CARRE ; ++k)
          line(j*(CARRE+INTER)+k,i*(CARRE+INTER),
                j*(CARRE+INTER)+k,i*(CARRE+INTER)+CARRE-1);
    }
  }
  refresh();
}

// fonction de comparaison de deux tableaux a et b,
// renvoie vrai s'ils sont égaux, faux sinon.
// le calcul s'effectue à partire de ligne "offset" pour "lines" lignes
bool egal(size_t hm, size_t lm, char a[hm][lm], char b[hm][lm], size_t offset, size_t lines) {
  for (size_t  i=offset ; i<offset+lines ; i++)
    for (size_t j=0 ; j<lm ; j++)
      if (a[i][j] != b[i][j])
        return false;  /* si on trouve une différence on s'arrête */
  return true;         /* les deux tableaux sont identiques */
}
