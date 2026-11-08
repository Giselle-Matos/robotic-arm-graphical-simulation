#ifndef BRACO_CONFIG_H
#define BRACO_CONFIG_H

/* Geometria em unidades da cena, compartilhada por FK, IK, desenho e colisao. */
#define ALTURA_BASE 0.6f
#define POSICAO_BASE_Z (-1.5f)
#define COMPRIMENTO_BRACO 2.6f
#define COMPRIMENTO_ANTEBRACO 2.0f
#define ELEVACAO_OMBRO 0.5f
#define VELOCIDADE_JUNTA 45.0f   /* Graus por segundo. */
#define VELOCIDADE_ABERTURA 0.6f /* Unidades por segundo, tambem para o cursor. */
#define FREQUENCIA_SIMULACAO 240
#define PASSO_SIMULACAO (1.0f / FREQUENCIA_SIMULACAO)
#define COMPRIMENTO_PUNHO 0.45f
#define ALTURA_PALMA 0.18f
#define COMPRIMENTO_DEDO 0.9f
#define ESPESSURA_DEDO 0.12f
#define PROFUNDIDADE_DEDO 0.30f
#define ABERTURA_MAXIMA 1.2f
/* Distancia do pivo do punho ao centro util entre os dedos. */
#define DISTANCIA_PUNHO_GARRA (COMPRIMENTO_PUNHO + ALTURA_PALMA + COMPRIMENTO_DEDO / 2)

#endif
