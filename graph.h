#ifndef GRAPH_H
#define GRAPH_H

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* ouvre une fenêtre de taille larg*haut */
void initgraph(size_t larg, size_t haut);

/* ferme la fenêtre */
void closegraph(void);

/* force l'affichage */
void flushgraph(void);

/* change la couleur du tracé: R,V,B \in [0.0, 1.0] */
unsigned long newcolor(float rouge, float vert, float bleu);
void setcolor(unsigned long);

/* efface la fenêtre */
void cleargraph(void);

/* dessine un texte */
void text(char *text, size_t x, size_t y);

/* trace un point */
void putpixel(size_t x, size_t y);

/* trace un segment */
void line(size_t x1, size_t y1, size_t x2, size_t y2);

/* trace un rectangle */
void rectangle(size_t x1, size_t y1, size_t x2, size_t y2);

/* rafraichit */
void refresh(void);

/* bloque l'exécution jusqu'à ce que l'utilisateur
   appuie sur une touche */
/* voir /usr/include/X11/keysymdef.h pour les valeurs de retour */
KeySym waitgraph(void);

/* idem non bloquant */
bool *check_keys(void);


#endif /* ! GRAPH_H */
