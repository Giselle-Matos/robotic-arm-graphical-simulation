#include "manipulacao.h"
#include "braco_config.h"
#include "aabb.h"
#include "colisao.h"
#include "simulacao_config.h"
#include "apoio.h"
#include <math.h>
#include <float.h>

/* -1 significa nenhum bloco; selecionar nao cria vinculo de transporte. */
static int capturado = -1, selecionado = -1;
/* Matriz do bloco no referencial da garra, constante enquanto estiver preso. */
static Matriz cargaNaGarra;
static float aberturaAlvo = -1; /* -1 deixa a abertura sob controle manual. */
static float centroAlvo;
static Braco poseAtual;
static int solturaPendente;
static const char *aviso;

static Aabb CaixaLocal(const Braco *pose, const Peca *bloco)
{
    Peca local = *bloco;
    local.mundo = MultiplicarMatrizes(InversaRigida(CalcularReferencialGarra(pose)), bloco->mundo);
    return CalcularAabb(&local);
}

static Matriz MundoDaCarga(const Braco *pose)
{
    return MultiplicarMatrizes(CalcularReferencialGarra(pose), cargaNaGarra);
}
static float VaoNecessario(Aabb c)
{
    return c.maximo[0] - c.minimo[0] + CAPTURA_FOLGA;
}
int BlocoCompativel(const Braco *pose, const Peca *bloco)
{
    Aabb c = CaixaLocal(pose, bloco);
    float sobreY =
        fminf(c.maximo[1], COMPRIMENTO_DEDO / 2) - fmaxf(c.minimo[1], -COMPRIMENTO_DEDO / 2);
    float sobreZ =
        fminf(c.maximo[2], PROFUNDIDADE_DEDO / 2) - fmaxf(c.minimo[2], -PROFUNDIDADE_DEDO / 2);
    float centro = (c.minimo[0] + c.maximo[0]) / 2;
    /* A selecao do mouse e independente da captura. A pega precisa apenas de
     * sobreposicao util: o cursor lateral ajusta os dedos, nunca o objeto. */
    return c.minimo[1] >= -(COMPRIMENTO_DEDO + ALTURA_PALMA) / 2 &&
           sobreY >= CAPTURA_CONTATO_MINIMO_Y && sobreZ >= CAPTURA_CONTATO_MINIMO_Z &&
           fabsf(centro) <= CAPTURA_DESLOCAMENTO_MAXIMO && VaoNecessario(c) <= ABERTURA_MAXIMA;
}
void ReiniciarManipulacao(void)
{
    capturado = selecionado = -1;
    cargaNaGarra = Identidade();
    aberturaAlvo = -1;
    centroAlvo = 0;
    aviso = 0;
    solturaPendente = 0;
}
int BlocoCapturado(void)
{
    return capturado;
}
int BlocoSelecionado(void)
{
    return selecionado;
}
void SelecionarBloco(int indice)
{
    if (capturado >= 0)
        return;
    selecionado = indice >= 0 && indice < TOTAL_BLOCOS ? indice : -1;
    aviso = 0;
}
float AberturaAssistida(void)
{
    return aberturaAlvo;
}
float CentroAssistido(void)
{
    return capturado >= 0 ? centroAlvo : 0;
}
Matriz RelativaCarga(void)
{
    return cargaNaGarra;
}
void CancelarAjusteManual(void)
{
    if (capturado < 0)
        aberturaAlvo = -1;
}
int ProntoParaPegar(const Braco *pose)
{
    if (capturado >= 0 || selecionado < 0 || !BlocoCompativel(pose, &blocos[selecionado]))
        return 0;
    /* Mesmo criterio do comando, com contexto de carga explicito e sem
     * alterar nem temporariamente o estado global da manipulacao. */
    return !PoseColideComBlocos(pose, blocos, selecionado);
}
const char *InstrucaoCaptura(const Braco *pose)
{
    if (aviso)
        return aviso;
    if (solturaPendente)
        return "Abrindo a pinca para soltar...";
    if (capturado >= 0)
        return "Espaco: soltar sobre a bancada";
    if (selecionado < 0)
        return "Clique em um bloco para selecionar";
    if (ProntoParaPegar(pose))
        return "Pronto para pegar: ESPACO";
    return "Aproxime os dedos do selecionado";
}
void PreverBlocos(const Braco *pose, Peca saida[TOTAL_BLOCOS])
{
    int i;
    for (i = 0; i < TOTAL_BLOCOS; ++i)
        saida[i] = blocos[i];
    if (capturado >= 0)
        saida[capturado].mundo = MundoDaCarga(pose);
}
void AtualizarManipulacao(const Braco *pose)
{
    poseAtual = *pose;
    aviso = 0;
    if (capturado >= 0)
        blocos[capturado].mundo = MundoDaCarga(pose);
    if (solturaPendente && !TentarSoltarBloco())
        solturaPendente = 0;
}
int AlternarCaptura(const Braco *pose)
{
    if (capturado >= 0)
        return TentarSoltarBloco();
    if (!ProntoParaPegar(pose)) {
        aviso =
            selecionado < 0 ? "Selecione um bloco com o mouse" : "Bloco fora da regiao de captura";
        return 0;
    }
    cargaNaGarra = MultiplicarMatrizes(InversaRigida(CalcularReferencialGarra(pose)),
                                       blocos[selecionado].mundo);
    capturado = selecionado;
    /* Transacao: permite contato dos dedos, mas nunca atravessar palma,
     * braco, mesa ou outro bloco. Nao desloca o objeto ao anexar. */
    if (PoseColide(pose)) {
        capturado = -1;
        aviso = "Afaste a palma ou outros obstaculos";
        return 0;
    }
    Aabb local = CaixaLocal(pose, &blocos[capturado]);
    aberturaAlvo = VaoNecessario(local);
    centroAlvo = (local.minimo[0] + local.maximo[0]) / 2;
    poseAtual = *pose;
    aviso = 0;
    return 1;
}
int TentarSoltarBloco(void)
{
    Peca solto, pecas[TOTAL_PECAS_BRACO];
    const char *motivo = 0;
    if (!PrepararSoltura(capturado, &poseAtual, &solto, &motivo)) {
        aviso = motivo;
        return 0;
    }
    CalcularPecasBraco(&poseAtual, pecas);
    for (int dedo = PECA_DEDO_ESQUERDO; dedo <= PECA_DEDO_DIREITO; ++dedo) {
        if (PecasColidem(&solto, &pecas[dedo])) {
            aberturaAlvo = ABERTURA_MAXIMA;
            solturaPendente = 1;
            aviso = 0;
            return 1;
        }
    }
    blocos[capturado] = solto;
    capturado = -1;
    aberturaAlvo = ABERTURA_MAXIMA;
    centroAlvo = 0;
    aviso = 0;
    solturaPendente = 0;
    return 1;
}

int SelecionarPorRaio(const Braco *pose, const float origem[3], const float direcao[3])
{
    Peca pecas[TOTAL_PECAS_BRACO];
    float menor = FLT_MAX, obstaculo;
    int i, id = -1;
    if (capturado >= 0)
        return selecionado;
    if (direcao[0] * direcao[0] + direcao[1] * direcao[1] + direcao[2] * direcao[2] < 1e-12f)
        return -1;
    for (i = 0; i < TOTAL_BLOCOS; ++i) {
        float t = DistanciaRaioPeca(&blocos[i], origem, direcao);
        if (t < menor) {
            menor = t;
            id = i;
        }
    }
    obstaculo = DistanciaRaioPeca(&bancada, origem, direcao);
    CalcularPecasBraco(pose, pecas);
    for (i = 0; i < TOTAL_PECAS_BRACO; ++i)
        obstaculo = fminf(obstaculo, DistanciaRaioPeca(&pecas[i], origem, direcao));
    if (obstaculo < menor - 1e-5f)
        id = -1;
    SelecionarBloco(id);
    return id;
}
