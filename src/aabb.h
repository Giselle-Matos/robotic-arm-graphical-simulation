#ifndef AABB_H
#define AABB_H
#include "cinematica.h"

#define CAPACIDADE_AABB 32
typedef struct {
    float minimo[3], maximo[3];
} Aabb;
typedef struct {
    Aabb caixa;
    int esquerda, direita, peca; /* Folha: peca >= 0. No interno: peca == -1. */
} NoAabb;
typedef struct {
    NoAabb nos[2 * CAPACIDADE_AABB - 1];
    int raiz, total;
} ArvoreAabb;

Aabb CalcularAabb(const Peca *peca);
int AabbsSobrepoem(Aabb a, Aabb b);
/* BVH binaria balanceada pelo eixo de maior extensao dos centros.
 * Copia os limites, sem guardar ponteiros. Retorna 0 para tamanho invalido. */
int ConstruirArvoreAabb(ArvoreAabb *arvore, const Peca *pecas, int quantidade);
/* Saida deve comportar CAPACIDADE_AABB indices das pecas originais. */
int ConsultarArvoreAabb(const ArvoreAabb *arvore, Aabb caixa, int indices[CAPACIDADE_AABB]);
#endif
