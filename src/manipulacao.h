#ifndef MANIPULACAO_H
#define MANIPULACAO_H
#include "cena_dados.h"

int BlocoCompativel(const Braco *pose, const Peca *bloco);
int BlocoSelecionado(void);
void SelecionarBloco(int indice);
/* Ray casting OBB; testa tambem oclusao pela bancada e pelo braco. */
int SelecionarPorRaio(const Braco *pose, const float origem[3], const float direcao[3]);
int ProntoParaPegar(const Braco *pose);
int AlternarCaptura(const Braco *pose);
float AberturaAssistida(void); /* -1 quando nao ha ajuste automatico. */
float CentroAssistido(void);
Matriz RelativaCarga(void);
void CancelarAjusteManual(void);
const char *InstrucaoCaptura(const Braco *pose);
void ReiniciarManipulacao(void);
int BlocoCapturado(void); /* -1 se nenhum. */
/* Atualiza somente o transporte apos aceitar uma pose. Nao captura com F. */
void AtualizarManipulacao(const Braco *pose);
/* Prediz transporte sem modificar a cena (usado pelas colisoes do trajeto). */
void PreverBlocos(const Braco *pose, Peca saida[TOTAL_BLOCOS]);
/* Soltura apoiada na bancada ou em outra face valida. Regras em apoio.c. */
int TentarSoltarBloco(void);
#endif
