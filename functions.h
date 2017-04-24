#ifndef FUNCTIONS_H
#define FUNCTIONS_H 1

#include <stddef.h>
#include <stdbool.h>

#define CARRE 3
#define INTER 0

#define DIFFTEMPS(a,b) (((b).tv_sec - (a).tv_sec) + ((b).tv_usec - (a).tv_usec)/1000000.)

extern unsigned long noir;
extern unsigned long vert;

void init(size_t hm, size_t lm, char[hm][lm]);
void calcnouv(size_t hm, size_t lm, char[hm][lm], char[hm][lm], size_t offset, size_t lines);
void affiche(size_t hm, size_t lm, char[hm][lm]);
bool egal(size_t hm, size_t lm, char[hm][lm], char[hm][lm], size_t offset, size_t lines);

#endif
