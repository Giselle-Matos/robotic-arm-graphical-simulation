#ifndef MOVIMENTO_H
#define MOVIMENTO_H
#include "ik.h"

/* Planeja antes de executar; uma falha nao altera a pose nem o objeto. */
int MoverParaAlvo(Matriz alvo);
int AproximarBlocoSelecionado(void); /* T */
int TransportarParaDestino(void);    /* Y, com carga */
int IniciarDemonstracao(void);       /* F5, reinicia e usa os mesmos comandos */
void CancelarMovimento(void);        /* C ou controle manual */
void AtualizarMovimento(float segundos);
int MovimentoAutomaticoAtivo(void);
const char *EstadoMovimento(void);
int ObterAlvoMovimento(Matriz *alvo);
float AlturaAlvoMouse(void);
void AlterarAlturaAlvoMouse(float delta);
#endif
