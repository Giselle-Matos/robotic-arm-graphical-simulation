#ifndef COLISAO_H
#define COLISAO_H
#include "cinematica.h"
/* Caixas orientadas: toque permitido; penetracao acima da tolerancia bloqueada. */
int PecasColidem(const Peca *a, const Peca *b);
/* Parametro t do primeiro encontro com a caixa orientada; FLT_MAX se nao houver.
 * A direcao nao precisa ser unitaria: use a mesma para comparar distancias. */
float DistanciaRaioPeca(const Peca *peca, const float origem[3], const float direcao[3]);
int PoseColide(const Braco *pose);
int PoseColideComBlocos(const Braco *pose, const Peca blocosPrevistos[], int indiceCarga);
/* Excecoes de montagem explicitas; segmentos vizinhos usam nucleos reduzidos
 * apenas na regiao do pivo, em vez de ignorar grupos inteiros. */
int PecasBracoColidem(const Peca pecas[TOTAL_PECAS_BRACO], int a, int b);
/* Verifica subpassos; rejeita o comando inteiro se qualquer pose colidir. */
int TrajetoColide(const Braco *inicio, const Braco *fim);
#endif
