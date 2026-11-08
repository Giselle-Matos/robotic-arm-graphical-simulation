#ifndef APOIO_H
#define APOIO_H
#include "cena_dados.h"
/* Ajusta somente a orientacao; preserva o centro e retorna inclinacao corrigida.
 * Serve tanto ao assentamento tolerante quanto ao alvo de colocacao por IK. */
float NivelarPeca(Peca *peca);
int PrepararSoltura(int indice, const Braco *pose, Peca *solto, const char **motivo);
int BlocoNoDestino(int indice);
int TotalEntregues(void);
int ProximaVagaDestino(float posicao[3]);
#endif
