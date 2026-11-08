#include "cena_dados.h"
#include <string.h>
#include "visual_config.h"
/* Tabela inicial: posicao, dimensoes e cor de cada bloco.
 * O mesmo inicializador atende ao estado inicial e a copia usada pelo reset. */
// clang-format off
#define POSICAO(x,y,z) {{1,0,0,0, 0,1,0,0, 0,0,1,0, x,y,z,1}}
#define BLOCOS_INICIAIS { \
    {POSICAO(-2.5f,.4f,1.4f), {.8f,.8f,.8f}, {COR_BLOCO_01}}, \
    {POSICAO(-1.2f,.4f,2.1f), {.8f,.8f,.8f}, {COR_BLOCO_02}}, \
    {POSICAO(-3,.4f,-.4f),    {.8f,.8f,.8f}, {COR_BLOCO_03}}  \
}
// clang-format on
const Peca bancada = {POSICAO(0, -.15f, 0), {10, .3f, 8}, {COR_GRAFITE}};
static const Peca iniciais[TOTAL_BLOCOS] = BLOCOS_INICIAIS;
Peca blocos[TOTAL_BLOCOS] = BLOCOS_INICIAIS;
void ReiniciarCena(void)
{
    memcpy(blocos, iniciais, sizeof blocos);
}
/* Marcador decorativo, nao obstaculo. */
const Peca destino = {POSICAO(2.7f, .012f, .8f), {2.2f, .024f, 2.2f}, {COR_DESTINO}};
