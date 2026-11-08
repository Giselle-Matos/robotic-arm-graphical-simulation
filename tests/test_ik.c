#include "../src/ik.h"
#include "../src/juntas.h"
#include "../src/manipulacao.h"
#include "../src/movimento.h"
#include "../src/apoio.h"
#include "../src/colisao.h"
#include "../src/simulacao_config.h"
#include "../src/aabb.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void ConferirReferenciais(Matriz esperado, Matriz atual)
{
    for (int i = 0; i < 16; ++i)
        assert(fabsf(esperado.v[i] - atual.v[i]) < 0.003f);
}
static void TestarSolver(void)
{
    Braco referencia = {.abertura = 1.2f}, solucoes[IK_MAX_SOLUCOES];
    int quantidade;
    srand(18);
    for (int i = 0; i < 400; ++i) {
        Braco pose = {.base = (float)(rand() % 360),
                      .ombro = (float)(rand() % 201 - 100),
                      .cotovelo = (float)(rand() % 281 - 140),
                      .punho = (float)(rand() % 301 - 150),
                      .desvioPunho = (float)(rand() % 141 - 70),
                      .giroPunho = (float)(rand() % 360),
                      .abertura = 1.2f};
        Matriz alvo = CalcularReferencialGarra(&pose);
        assert(ResolverIK(alvo, &pose, solucoes, &quantidade) == IK_OK);
        for (int s = 0; s < quantidade; ++s) {
            assert(PoseDentroDosLimites(&solucoes[s]));
            ConferirReferenciais(alvo, CalcularReferencialGarra(&solucoes[s]));
        }
    }
    Matriz alvo = AlvoVertical(100, .7f, 0, 0);
    assert(ResolverIK(alvo, &referencia, solucoes, &quantidade) == IK_FORA_DE_ALCANCE);
    alvo.v[12] = NAN;
    assert(ResolverIK(alvo, &referencia, solucoes, &quantidade) == IK_ALVO_INVALIDO);
    alvo = Identidade();
    alvo.v[0] = -1;
    assert(ResolverIK(alvo, &referencia, solucoes, &quantidade) == IK_ALVO_INVALIDO);
    Braco dobrada = {.cotovelo = 180, .abertura = 1.2f};
    assert(ResolverIK(CalcularReferencialGarra(&dobrada), &referencia, solucoes, &quantidade) ==
           IK_FORA_DOS_LIMITES);
    Braco polo = {.ombro = 30, .cotovelo = 50, .punho = 20, .desvioPunho = 90, .abertura = 1.2f};
    alvo = CalcularReferencialGarra(&polo);
    assert(ResolverIK(alvo, &polo, solucoes, &quantidade) == IK_OK);
    for (int i = 0; i < quantidade; ++i)
        ConferirReferenciais(alvo, CalcularReferencialGarra(&solucoes[i]));
    puts("OK: IK, 400 FK->IK, orientacao, polos, alvos invalidos/inalcancaveis e limites.");
}
static void TestarFalhasSemEfeito(void)
{
    CancelarMovimento();
    ReiniciarBraco();
    Braco antes = ObterEstadoBraco();
    assert(!MoverParaAlvo(AlvoVertical(20, .7f, 0, 0)));
    Braco depois = ObterEstadoBraco();
    ConferirReferenciais(CalcularReferencialGarra(&antes), CalcularReferencialGarra(&depois));
    assert(!MovimentoAutomaticoAtivo());
    assert(!AproximarBlocoSelecionado());
    assert(!TransportarParaDestino());
    SelecionarBloco(0);
    assert(AproximarBlocoSelecionado());
    CancelarMovimento();
    assert(!MovimentoAutomaticoAtivo());
    depois = ObterEstadoBraco();
    ConferirReferenciais(CalcularReferencialGarra(&antes), CalcularReferencialGarra(&depois));
    puts("OK: falhas de planejamento e cancelamento preservam estado.");
}
static void TestarTarefaCompleta(void)
{
    const char *anterior = 0;
    assert(IniciarDemonstracao());
    int passos;
    for (passos = 0; passos < 60000 && MovimentoAutomaticoAtivo(); ++passos) {
        if (EstadoMovimento() != anterior) {
            anterior = EstadoMovimento();
            printf("demo: %s | entregues %d\n", anterior, TotalEntregues());
            fflush(stdout);
        }
        AtualizarMovimento(1.0f / 120);
        Braco pose = ObterEstadoBraco();
        assert(PoseDentroDosLimites(&pose));
        assert(!PoseColide(&pose));
    }
    printf("demo final: %s | entregues %d | passos %d\n", EstadoMovimento(), TotalEntregues(),
           passos);
    fflush(stdout);
    assert(!MovimentoAutomaticoAtivo());
    assert(TotalEntregues() == TOTAL_BLOCOS);
    assert(BlocoCapturado() == -1);
    for (int i = 0; i < TOTAL_BLOCOS; ++i)
        assert(BlocoNoDestino(i));
    ReiniciarBraco();
    CancelarMovimento();
    assert(TotalEntregues() == 0);
    puts(
        "OK: tres blocos selecionados, aproximados, agarrados, transportados e entregues sem colisao.");
}
static void TestarApoioEDestino(void)
{
    Peca solto;
    const char *motivo = 0;
    CancelarMovimento();
    ReiniciarBraco();
    Braco pose = ObterEstadoBraco();
    blocos[0].mundo = Identidade();
    Transladar(&blocos[0].mundo, -3.8f, .50f, 2.8f);
    Rotacionar(&blocos[0].mundo, 10, 0, 0, 1);
    assert(PrepararSoltura(0, &pose, &solto, &motivo));
    assert(fabsf(CalcularAabb(&solto).minimo[1]) < .001f);
    blocos[0].mundo.v[12] = 5.1f;
    assert(!PrepararSoltura(0, &pose, &solto, &motivo));
    ReiniciarCena();
    blocos[0].mundo = blocos[1].mundo;
    assert(!PrepararSoltura(0, &pose, &solto, &motivo)); /* apoio ocupado */
    blocos[0].mundo.v[13] = 1.2f;
    assert(PrepararSoltura(0, &pose, &solto, &motivo)); /* face sobre outro bloco */
    blocos[0].mundo.v[12] += .1f;
    assert(!PrepararSoltura(0, &pose, &solto, &motivo)); /* apoio parcial */
    ReiniciarCena();
    blocos[0].mundo.v[12] = 2.18f;
    blocos[0].mundo.v[14] = .28f;
    assert(BlocoNoDestino(0));
    assert(TotalEntregues() == 1);
    blocos[0].mundo.v[12] = 1.7f;
    assert(!BlocoNoDestino(0)); /* nao cabe inteiro */
    ReiniciarBraco();
    puts("OK: assentamento tolerante, apoio ocupado/parcial, empilhamento e destino integral.");
}

static void TestarPegaBloco01Descentrada(void)
{
    CancelarMovimento();
    ReiniciarBraco();
    SelecionarBloco(0);
    Matriz alvo = AlvoVertical(blocos[0].mundo.v[12] + .18f, .68f, blocos[0].mundo.v[14], 0);
    assert(MoverParaAlvo(alvo));
    for (int i = 0; i < 20000 && MovimentoAutomaticoAtivo(); ++i)
        AtualizarMovimento(1.0f / 120);
    assert(!MovimentoAutomaticoAtivo());
    Braco pose = ObterEstadoBraco();
    assert(ProntoParaPegar(&pose));
    Matriz antes = blocos[0].mundo;
    assert(AlternarSegurar());
    ConferirReferenciais(antes, blocos[0].mundo);
    AtualizarBraco(1);
    pose = ObterEstadoBraco();
    assert(fabsf(pose.centroGarra) > .15f);
    assert(!PoseColide(&pose));
    ConferirReferenciais(antes, blocos[0].mundo);
    CancelarMovimento();
    ReiniciarBraco();
    puts("OK: bloco 01 deslocado 0.18, cursor lateral dos dedos, sem snap ou colisao.");
}
int main(void)
{
    TestarSolver();
    TestarFalhasSemEfeito();
    TestarApoioEDestino();
    TestarPegaBloco01Descentrada();
    TestarTarefaCompleta();
    return 0;
}
