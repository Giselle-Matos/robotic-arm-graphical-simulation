#ifndef CINEMATICA_H
#define CINEMATICA_H
#include "braco.h"
#include "transformacao.h"

typedef enum {
    PECA_BASE,
    PECA_MARCADOR,
    PECA_SUPORTE,
    PECA_OMBRO,
    PECA_BRACO,
    PECA_COTOVELO,
    PECA_ANTEBRACO,
    PECA_PIVO_PUNHO,
    PECA_PUNHO,
    PECA_PALMA,
    PECA_DEDO_ESQUERDO,
    PECA_DEDO_DIREITO,
    TOTAL_PECAS_BRACO
} IndicePeca;
/* Referencial sem escala; dimensoes totais e cor para desenho e colisao. */
typedef struct {
    Matriz mundo;
    float tamanho[3];
    float cor[3];
} Peca;
/* Preenche exatamente TOTAL_PECAS_BRACO, na ordem da cadeia. */
void CalcularPecasBraco(const Braco *braco, Peca pecas[TOTAL_PECAS_BRACO]);
/* Centro do volume entre os dedos; orientacao herdada do punho, sem escala. */
Matriz CalcularReferencialGarra(const Braco *braco);
#endif
