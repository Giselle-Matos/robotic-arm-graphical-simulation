#ifndef IK_H
#define IK_H
#include "cinematica.h"

typedef enum { IK_OK, IK_ALVO_INVALIDO, IK_FORA_DE_ALCANCE, IK_FORA_DOS_LIMITES } ResultadoIK;
#define IK_MAX_SOLUCOES 4
/* Solver puro: nenhum estado global e nenhuma alteracao na cena.
 * Posicao do punho -> dois segmentos planares -> orientacao Z-X-Y do punho.
 * Retorna ate quatro ramos (base invertida e cotovelo positivo/negativo).
 * Colisoes e trajetos sao tratados pelo planejador, nao pelo solver. */
ResultadoIK ResolverIK(Matriz alvoGarra, const Braco *referencia, Braco solucoes[IK_MAX_SOLUCOES],
                       int *quantidade);
Matriz AlvoVertical(float x, float y, float z, float giroMundo);
const char *DescricaoResultadoIK(ResultadoIK resultado);
#endif
