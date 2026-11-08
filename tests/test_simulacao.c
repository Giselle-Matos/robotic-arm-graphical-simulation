#include "../src/braco.h"
#include "../src/braco_config.h"
#include "../src/aabb.h"
#include "../src/cena_dados.h"
#include "../src/colisao.h"
#include "../src/manipulacao.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static void Perto(float a, float b)
{
    assert(fabsf(a - b) < 0.002f);
}
static Peca CaixaTeste(float x, float y, float z, float l, float a, float p)
{
    Peca c = {.mundo = Identidade(), .tamanho = {l, a, p}, .cor = {1, 1, 1}};
    Transladar(&c.mundo, x, y, z);
    return c;
}
/* Oraculo sem arvore, com a mesma politica de pares conectados. */
static int ColisaoExaustiva(const Braco *b)
{
    Peca p[TOTAL_PECAS_BRACO];
    int i, j;
    CalcularPecasBraco(b, p);
    for (i = 0; i < TOTAL_PECAS_BRACO; ++i) {
        if (i > PECA_SUPORTE && PecasColidem(&p[i], &bancada))
            return 1;
        for (j = 0; j < TOTAL_BLOCOS; ++j)
            if (PecasColidem(&p[i], &blocos[j]))
                return 1;
        for (j = 0; j < i; ++j)
            if (PecasBracoColidem(p, i, j))
                return 1;
    }
    return 0;
}
static void TestarMatrizes(void)
{
    Matriz m = Identidade();
    Peca p[TOTAL_PECAS_BRACO], q[TOTAL_PECAS_BRACO];
    Braco b = {.abertura = 1.2f};
    int i, j;
    Transladar(&m, 1, 2, 3);
    Rotacionar(&m, 90, 0, 0, 1);
    Transladar(&m, 2, 0, 0);
    Perto(m.v[12], 1);
    Perto(m.v[13], 4);
    Perto(m.v[14], 3);
    Perto(m.v[0], 0);
    Perto(m.v[1], 1);
    Perto(m.v[4], -1);
    m = Identidade();
    Rotacionar(&m, 90, 0, 1, 0);
    Perto(m.v[2], -1);
    Perto(m.v[8], 1);
    m = Identidade();
    Rotacionar(&m, 90, 1, 0, 0);
    Perto(m.v[6], 1);
    Perto(m.v[9], -1);
    CalcularPecasBraco(&b, p);
    Perto(p[5].mundo.v[13], ALTURA_BASE + ELEVACAO_OMBRO + COMPRIMENTO_BRACO);
    Perto(p[7].mundo.v[13],
          ALTURA_BASE + ELEVACAO_OMBRO + COMPRIMENTO_BRACO + COMPRIMENTO_ANTEBRACO);
    Perto(p[10].mundo.v[12], -(ABERTURA_MAXIMA + ESPESSURA_DEDO) / 2);
    Perto(p[11].mundo.v[12], (ABERTURA_MAXIMA + ESPESSURA_DEDO) / 2);
    b = (Braco){.base = 31,
                .ombro = 17,
                .cotovelo = 44,
                .punho = 53,
                .desvioPunho = 21,
                .giroPunho = 67,
                .abertura = .8f};
    CalcularPecasBraco(&b, p);
    b.base += 720;
    b.ombro -= 360;
    b.cotovelo += 360;
    b.punho -= 720;
    b.desvioPunho += 360;
    b.giroPunho -= 360;
    CalcularPecasBraco(&b, q);
    for (i = 0; i < TOTAL_PECAS_BRACO; ++i)
        for (j = 0; j < 16; ++j)
            Perto(p[i].mundo.v[j], q[i].mundo.v[j]);
}
static void TestarArvore(void)
{
    Peca p[CAPACIDADE_AABB];
    ArvoreAabb arvore;
    int saida[CAPACIDADE_AABB], vistos[CAPACIDADE_AABB], i, j, n, k;
    assert(ConstruirArvoreAabb(&arvore, NULL, 0));
    assert(ConsultarArvoreAabb(&arvore,
                               CalcularAabb(&(Peca){.mundo = Identidade(), .tamanho = {1, 1, 1}}),
                               saida) == 0);
    assert(!ConstruirArvoreAabb(&arvore, NULL, CAPACIDADE_AABB + 1));
    assert(!ConstruirArvoreAabb(&arvore, NULL, -1));
    for (i = 0; i < CAPACIDADE_AABB; ++i) {
        p[i] = CaixaTeste((float)(i % 7), (float)(i % 3), (float)(i % 5), 2, .7f, 1);
        Rotacionar(&p[i].mundo, (float)(i * 17), 0, 1, 0);
    }
    assert(ConstruirArvoreAabb(&arvore, p, CAPACIDADE_AABB));
    assert(arvore.total == 2 * CAPACIDADE_AABB - 1);
    for (i = 0; i < CAPACIDADE_AABB; ++i) {
        for (j = 0; j < CAPACIDADE_AABB; ++j)
            vistos[j] = 0;
        n = ConsultarArvoreAabb(&arvore, CalcularAabb(&p[i]), saida);
        for (k = 0; k < n; ++k) {
            assert(saida[k] >= 0 && saida[k] < CAPACIDADE_AABB);
            assert(!vistos[saida[k]]++);
        }
        for (j = 0; j < CAPACIDADE_AABB; ++j)
            assert(vistos[j] == AabbsSobrepoem(CalcularAabb(&p[i]), CalcularAabb(&p[j])));
    }
    /* Todos os centros iguais exercitam a subdivisao degenerada. */
    for (i = 0; i < CAPACIDADE_AABB; ++i)
        p[i] = CaixaTeste(0, 0, 0, 1, 1, 1);
    assert(ConstruirArvoreAabb(&arvore, p, CAPACIDADE_AABB));
    assert(ConsultarArvoreAabb(&arvore, CalcularAabb(&p[0]), saida) == CAPACIDADE_AABB);
    assert(ConstruirArvoreAabb(&arvore, p, 1));
    assert(arvore.total == 1);
}
static void TestarColisoes(void)
{
    Peca a = CaixaTeste(0, 0, 0, 1, 1, 1), b = CaixaTeste(1, 0, 0, 1, 1, 1);
    Braco pose = {.abertura = 1.2f};
    int i, livres = 0, colidindo = 0;
    assert(!PecasColidem(&a, &b)); /* toque permitido */
    Transladar(&b.mundo, -.01f, 0, 0);
    assert(PecasColidem(&a, &b));
    Transladar(&b.mundo, 3, 0, 0);
    assert(!PecasColidem(&a, &b));
    b = CaixaTeste(0, 0, 0, .1f, .1f, .1f);
    assert(PecasColidem(&a, &b));
    /* AABBs sobrepostas, OBBs paralelas separadas: evita falso bloqueio. */
    a = CaixaTeste(0, 0, 0, 4, .2f, .2f);
    b = CaixaTeste(0, .5f, 0, 4, .2f, .2f);
    Rotacionar(&a.mundo, 45, 0, 0, 1);
    Rotacionar(&b.mundo, 45, 0, 0, 1);
    assert(AabbsSobrepoem(CalcularAabb(&a), CalcularAabb(&b)));
    assert(!PecasColidem(&a, &b));
    srand(42);
    for (i = 0; i < 1000; ++i) {
        int resultado;
        pose.base = (float)(rand() % 360);
        pose.ombro = (float)(rand() % 360);
        pose.cotovelo = (float)(rand() % 360);
        pose.punho = (float)(rand() % 360);
        pose.desvioPunho = (float)(rand() % 360);
        pose.giroPunho = (float)(rand() % 360);
        pose.abertura = (rand() % 121) / 100.0f;
        resultado = PoseColide(&pose);
        assert(resultado == ColisaoExaustiva(&pose));
        if (resultado)
            ++colidindo;
        else
            ++livres;
    }
    assert(livres > 0 && colidindo > 0);
    /* Dois extremos livres cujo percurso cruza a bancada. */
    pose = (Braco){.abertura = 1.2f};
    Braco fim = pose;
    fim.ombro = 360;
    assert(!PoseColide(&pose));
    assert(!PoseColide(&fim));
    assert(TrajetoColide(&pose, &fim));
}
static Braco PoseSobreBloco(int indice, float alturaCentroGarra)
{
    float x = blocos[indice].mundo.v[12], z = blocos[indice].mundo.v[14] + 1.5f;
    float r = hypotf(x, z), y = alturaCentroGarra + COMPRIMENTO_PUNHO + ALTURA_PALMA +
                                COMPRIMENTO_DEDO / 2 - (ALTURA_BASE + ELEVACAO_OMBRO);
    float e = acosf((r * r + y * y - COMPRIMENTO_BRACO * COMPRIMENTO_BRACO -
                     COMPRIMENTO_ANTEBRACO * COMPRIMENTO_ANTEBRACO) /
                    (2 * COMPRIMENTO_BRACO * COMPRIMENTO_ANTEBRACO));
    float o = atan2f(r, y) - atan2f(COMPRIMENTO_ANTEBRACO * sinf(e),
                                    COMPRIMENTO_BRACO + COMPRIMENTO_ANTEBRACO * cosf(e));
    const float graus = 57.295779513f;
    float base = atan2f(-z, x) * graus;
    return (Braco){.base = base,
                   .ombro = o * graus,
                   .cotovelo = e * graus,
                   .punho = 180 - (o + e) * graus,
                   .giroPunho = base,
                   .abertura = .82f};
}

static void TestarManipulacao(void)
{
    Braco pose, levantada;
    Peca original, fora, previstos[TOTAL_BLOCOS];
    Matriz relativa, conferida;
    int i;
    ReiniciarBraco();
    pose = PoseSobreBloco(0, .46f);
    original = blocos[0];
    assert(!PoseColide(&pose));
    assert(BlocoCompativel(&pose, &blocos[0]));
    fora = original;
    fora.mundo.v[12] += .4f;
    assert(!BlocoCompativel(&pose, &fora));
    fora = original;
    fora.mundo.v[13] += .5f;
    assert(!BlocoCompativel(&pose, &fora));
    fora = original;
    fora.mundo.v[14] += .7f;
    assert(!BlocoCompativel(&pose, &fora));
    fora = original;
    Rotacionar(&fora.mundo, 30, 0, 1, 0);
    assert(BlocoCompativel(&pose, &fora));
    pose.abertura = 1.2f;
    assert(BlocoCompativel(&pose, &original));
    pose.abertura = .70f;
    assert(BlocoCompativel(&pose, &original));
    pose.abertura = .82f;
    fora = original;
    fora.mundo.v[12] += .06f;
    pose.abertura = .94f;
    assert(BlocoCompativel(&pose, &fora));
    pose.abertura = .82f;
    fora = original;
    fora.mundo.v[14] += .20f;
    assert(BlocoCompativel(&pose, &fora));
    AtualizarManipulacao(&pose);
    assert(BlocoCapturado() == -1);
    relativa = MultiplicarMatrizes(InversaRigida(CalcularReferencialGarra(&pose)), original.mundo);
    AtualizarManipulacao(&pose);
    assert(BlocoCapturado() == -1);
    SelecionarBloco(0);
    assert(AlternarCaptura(&pose));
    assert(BlocoCapturado() == 0);
    for (i = 0; i < 16; ++i)
        Perto(blocos[0].mundo.v[i], original.mundo.v[i]);
    levantada = PoseSobreBloco(0, .76f);
    PreverBlocos(&levantada, previstos);
    Perto(blocos[0].mundo.v[13], .4f); /* predicao nao modifica estado */
    Perto(previstos[0].mundo.v[13], .7f);
    assert(!PoseColide(&levantada));
    AtualizarManipulacao(&levantada);
    assert(!TentarSoltarBloco()); /* nao deixa bloco suspenso */
    conferida =
        MultiplicarMatrizes(InversaRigida(CalcularReferencialGarra(&levantada)), blocos[0].mundo);
    for (i = 0; i < 16; ++i)
        Perto(relativa.v[i], conferida.v[i]);
    /* Colisao do volume do bloco, mesmo quando o braco sozinho esta livre. */
    blocos[1] = blocos[0];
    blocos[1].mundo.v[14] += .7f;
    assert(PoseColide(&levantada));
    assert(TrajetoColide(&pose, &levantada));
    Perto(blocos[0].mundo.v[13], .7f); /* rejeicao/predicao nao move a cena */
    ReiniciarCena();
    AtualizarManipulacao(&levantada);
    levantada.giroPunho += 90;
    assert(!PoseColide(&levantada));
    AtualizarManipulacao(&levantada);
    conferida =
        MultiplicarMatrizes(InversaRigida(CalcularReferencialGarra(&levantada)), blocos[0].mundo);
    for (i = 0; i < 16; ++i)
        Perto(relativa.v[i], conferida.v[i]);
    AtualizarManipulacao(&pose);
    assert(TentarSoltarBloco());
    assert(BlocoCapturado() == -1);
    Perto(CalcularAabb(&blocos[0]).minimo[1], 0);
    AtualizarManipulacao(&pose);
    assert(BlocoCapturado() == -1);
    SelecionarBloco(0);
    assert(AlternarCaptura(&pose));
    assert(BlocoCapturado() == 0);
    ReiniciarBraco();
    assert(BlocoCapturado() == -1);
    for (i = 0; i < 16; ++i)
        Perto(blocos[0].mundo.v[i], original.mundo.v[i]);
    puts(
        "OK: captura geometrica, rejeicoes, transporte rigido, colisao da carga, soltura e reset.");
}

static void MoverAte(unsigned char tecla, float graus)
{
    DefinirTeclaBraco(tecla, 1);
    AtualizarBraco(graus / VELOCIDADE_JUNTA);
    DefinirTeclaBraco(tecla, 0);
    if (MovimentoBloqueado())
        fprintf(stderr, "Movimento bloqueado no teste: %c %.3f graus\n", tecla, graus);
    assert(!MovimentoBloqueado());
}

static void TestarCapturaPorTeclado(void)
{
    Braco alvo, antes, agora;
    ReiniciarBraco();
    alvo = PoseSobreBloco(0, .46f);
    MoverAte('d', -alvo.base);
    MoverAte('p', -alvo.giroPunho);
    /* Aproximacao por cima com pinca vertical: pares de juntas mantem
     * a soma das inclinacoes em 180 graus durante a descida. */
    MoverAte('l', 50);
    DefinirTeclaBraco('j', 1);
    MoverAte('s', alvo.ombro - 25);
    DefinirTeclaBraco('j', 0);
    DefinirTeclaBraco('j', 1);
    MoverAte('e', alvo.cotovelo - 60);
    DefinirTeclaBraco('j', 0);
    assert(BlocoCapturado() == -1);
    /* F apenas fecha: mesmo ao encontrar contato, nao captura. */
    DefinirTeclaBraco('f', 1);
    AtualizarBraco(1);
    DefinirTeclaBraco('f', 0);
    assert(BlocoCapturado() == -1);
    SelecionarBloco(0);
    assert(AlternarSegurar());
    AtualizarBraco(1);
    assert(BlocoCapturado() == 0);
    antes = ObterEstadoBraco();
    assert(antes.abertura >= .80f && antes.abertura <= .8251f);
    MoverAte('w', 10);
    agora = ObterEstadoBraco();
    assert(blocos[0].mundo.v[13] > .5f);
    assert(!PoseColide(&agora));
    /* G no ar nao perde a carga nem abre os dedos. */
    DefinirTeclaBraco('g', 1);
    AtualizarBraco(.2f);
    DefinirTeclaBraco('g', 0);
    assert(BlocoCapturado() == 0);
    Perto(ObterEstadoBraco().abertura, antes.abertura);
    ReiniciarBraco();
    assert(BlocoCapturado() == -1);
    Perto(blocos[0].mundo.v[13], .4f);
    puts("OK: F manual sem captura, Espaco prende, ajuste gradual, levantamento e G sem soltura.");
}

static void TestarPegaPelasPontas(void)
{
    int indices[] = {0, 2}, i;
    for (i = 0; i < 2; ++i) {
        Braco alvo, antes, depois;
        ReiniciarBraco();
        alvo = PoseSobreBloco(indices[i], .75f);
        MoverAte('d', -alvo.base);
        MoverAte('p', -alvo.giroPunho);
        MoverAte('l', 50);
        DefinirTeclaBraco('j', 1);
        MoverAte('e', alvo.cotovelo - 60);
        DefinirTeclaBraco('j', 0);
        DefinirTeclaBraco('j', 1);
        MoverAte('s', alvo.ombro - 25);
        DefinirTeclaBraco('j', 0);
        /* Contato pela parte inferior dos dedos e erro manual de giro de 12 graus. */
        MoverAte('o', 12);
        SelecionarBloco(indices[i]);
        assert(AlternarSegurar());
        DefinirTeclaBraco('f', 1);
        AtualizarBraco(1);
        assert(BlocoCapturado() == indices[i]);
        antes = ObterEstadoBraco();
        DefinirTeclaBraco('a', 1);
        AtualizarBraco(.05f);
        DefinirTeclaBraco('a', 0);
        depois = ObterEstadoBraco();
        Perto(depois.base - antes.base, 2.25f);
        assert(!MovimentoBloqueado());
        DefinirTeclaBraco('d', 1);
        AtualizarBraco(.05f);
        DefinirTeclaBraco('d', 0);
        Perto(ObterEstadoBraco().base, antes.base);
        assert(!MovimentoBloqueado());
        /* Captura continua firme ao levantar com F ainda pressionado. */
        MoverAte('w', 10);
        assert(blocos[indices[i]].mundo.v[13] > .5f);
        DefinirTeclaBraco('f', 0);
    }
    ReiniciarBraco();
    puts("OK: pega pelas pontas com giro impreciso, F+D, F+A e transporte dos blocos 01/03.");
}

static void TestarIntegracao(void)
{
    Braco a, b;
    int i;
    ReiniciarBraco();
    a = ObterEstadoBraco();
    assert(!PoseColide(&a));
    DefinirTeclaBraco('a', 1);
    for (i = 0; i < 2160; ++i) {
        AtualizarBraco(1.0f / 120);
        b = ObterEstadoBraco();
        assert(!PoseColide(&b));
    }
    b = ObterEstadoBraco();
    Perto(b.base, 90); /* 810 graus sem batente */
    ReiniciarBraco();
    DefinirTeclaBraco('s', 1);
    for (i = 0; i < 1200; ++i) {
        a = ObterEstadoBraco();
        AtualizarBraco(PASSO_SIMULACAO);
        b = ObterEstadoBraco();
        assert(!PoseColide(&b));
        if (MovimentoBloqueado()) {
            Perto(a.ombro, b.ombro);
            break;
        }
    }
    assert(i < 1200);
    assert(MovimentoBloqueado());
    ReiniciarBraco();
    a = ObterEstadoBraco();
    AtualizarBraco(1);
    b = ObterEstadoBraco();
    Perto(a.ombro, 25);
    Perto(b.ombro, 25);
    assert(!MovimentoBloqueado());
    puts("OK: colisao real preserva pose valida e reset limpa bloqueio/teclas.");
}
static void TestarSentidosDeControle(void)
{
    ReiniciarBraco();
    Braco pose = ObterEstadoBraco();
    Matriz inicial = CalcularReferencialGarra(&pose);
    DefinirTeclaBraco('w', 1);
    AtualizarBraco(.1f);
    DefinirTeclaBraco('w', 0);
    pose = ObterEstadoBraco();
    assert(CalcularReferencialGarra(&pose).v[13] > inicial.v[13]);
    ReiniciarBraco();
    DefinirTeclaBraco('s', 1);
    AtualizarBraco(.1f);
    DefinirTeclaBraco('s', 0);
    pose = ObterEstadoBraco();
    assert(CalcularReferencialGarra(&pose).v[13] < inicial.v[13]);
    /* Olhando no sentido +X do braco inicial, esquerda aponta para -Z. */
    ReiniciarBraco();
    DefinirTeclaBraco('a', 1);
    AtualizarBraco(.1f);
    DefinirTeclaBraco('a', 0);
    pose = ObterEstadoBraco();
    assert(CalcularReferencialGarra(&pose).v[14] < inicial.v[14]);
    ReiniciarBraco();
    DefinirTeclaBraco('d', 1);
    AtualizarBraco(.1f);
    DefinirTeclaBraco('d', 0);
    pose = ObterEstadoBraco();
    assert(CalcularReferencialGarra(&pose).v[14] > inicial.v[14]);
    ReiniciarBraco();
    puts("OK: A/D esquerda/direita do braco; W eleva e S abaixa a garra.");
}
static void TestarSelecaoEComando(void)
{
    Braco pose;
    float origem[3], direcao[3] = {0, -1, 0};
    Peca original;
    int i;
    ReiniciarBraco();
    pose = ObterEstadoBraco();
    assert(BlocoSelecionado() == -1);
    assert(!AlternarSegurar());
    for (i = 0; i < TOTAL_BLOCOS; ++i) {
        origem[0] = blocos[i].mundo.v[12];
        origem[1] = 10;
        origem[2] = blocos[i].mundo.v[14];
        assert(SelecionarPorRaio(&pose, origem, direcao) == i);
    }
    origem[0] = 20;
    origem[2] = 20;
    assert(SelecionarPorRaio(&pose, origem, direcao) == -1);
    origem[0] = blocos[0].mundo.v[12];
    origem[2] = blocos[0].mundo.v[14];
    blocos[0].mundo.v[13] = -1; /* mesa oculta o bloco */
    assert(SelecionarPorRaio(&pose, origem, direcao) == -1);
    ReiniciarBraco();
    SelecionarBloco(0);
    assert(!AlternarSegurar());
    pose = PoseSobreBloco(0, .75f);
    pose.abertura = 1.2f;
    original = blocos[0];
    assert(ProntoParaPegar(&pose));
    SelecionarBloco(1);
    assert(!AlternarCaptura(&pose));
    assert(BlocoCapturado() == -1);
    SelecionarBloco(0);
    assert(AlternarCaptura(&pose));
    for (i = 0; i < 16; ++i)
        Perto(original.mundo.v[i], blocos[0].mundo.v[i]);
    SelecionarBloco(1);
    assert(BlocoSelecionado() == 0); /* selecao preservada com carga */
    assert(AlternarCaptura(&pose));
    assert(BlocoCapturado() == -1);
    /* Se os dedos ainda estiverem fechados, solta so depois de abri-los. */
    pose.abertura = .7f;
    assert(AlternarCaptura(&pose));
    assert(AlternarCaptura(&pose));
    assert(BlocoCapturado() == 0);
    pose.abertura = 1.2f;
    AtualizarManipulacao(&pose);
    assert(BlocoCapturado() == -1);
    ReiniciarBraco();
    pose = PoseSobreBloco(0, .75f);
    SelecionarBloco(0);
    blocos[1] = blocos[0];
    blocos[1].mundo.v[14] += .7f;
    assert(!AlternarCaptura(&pose));
    assert(BlocoCapturado() == -1);
    ReiniciarBraco();
    puts("OK: selecao por raio, oclusao, alvo correto, comando explicito e rollback de colisao.");
}
int main(void)
{
    TestarMatrizes();
    TestarArvore();
    TestarColisoes();
    TestarIntegracao();
    TestarSentidosDeControle();
    TestarManipulacao();
    TestarCapturaPorTeclado();
    TestarPegaPelasPontas();
    TestarSelecaoEComando();
    puts("OK: matrizes, FK, SAT, arvore AABB e equivalencia em 1000 poses.");
    return 0;
}
