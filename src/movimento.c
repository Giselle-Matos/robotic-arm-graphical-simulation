#include "movimento.h"
#include "juntas.h"
#include "braco_config.h"
#include "simulacao_config.h"
#include "manipulacao.h"
#include "apoio.h"
#include "colisao.h"
#include "aabb.h"
#include "matematica.h"
#include <math.h>

typedef struct {
    Braco pontos[IK_MAXIMO_PONTOS];
    int quantidade;
} Caminho;

typedef enum {
    DEMO_DESLIGADA,
    DEMO_APROXIMANDO,
    DEMO_FECHANDO,
    DEMO_TRANSPORTANDO,
    DEMO_SOLTANDO
} EtapaDemo;
static Caminho caminho;
static int trajetoAtivo, pontoAtual;
static float tempoSegmento, tempoEspera;
static Matriz alvoAtual;
static int possuiAlvo;
static float alturaMouse = IK_ALTURA_MOUSE_INICIAL;
static EtapaDemo etapaDemo;
static const char *mensagem = "FK manual | T: aproximar | Y: destino";

static float CustoAngular(const Braco *a, const Braco *b)
{
    float custo = 0;
    for (int j = 0; j < TOTAL_JUNTAS; ++j) {
        float delta = DiferencaAngular(ObterAngulo(a, (Junta)j), ObterAngulo(b, (Junta)j));
        custo += delta * delta;
    }
    return custo;
}

static int AdicionarAlvo(Caminho *plano, Matriz alvo)
{
    Braco solucoes[IK_MAX_SOLUCOES];
    Braco *inicio = &plano->pontos[plano->quantidade - 1];
    int quantidade;
    ResultadoIK resultado = ResolverIK(alvo, inicio, solucoes, &quantidade);
    if (resultado != IK_OK || plano->quantidade >= IK_MAXIMO_PONTOS)
        return 0;

    /* Testa primeiro o ramo mais proximo para evitar trocas bruscas do cotovelo. */
    for (int tentativa = 0; tentativa < quantidade; ++tentativa) {
        int melhor = tentativa;
        for (int j = tentativa + 1; j < quantidade; ++j)
            if (CustoAngular(inicio, &solucoes[j]) < CustoAngular(inicio, &solucoes[melhor]))
                melhor = j;
        Braco troca = solucoes[tentativa];
        solucoes[tentativa] = solucoes[melhor];
        solucoes[melhor] = troca;
        Braco fim = DesenrolarPose(inicio, &solucoes[tentativa]);
        if (!TrajetoDentroDosLimites(inicio, &fim) || TrajetoColide(inicio, &fim))
            continue;
        plano->pontos[plano->quantidade++] = fim;
        return 1;
    }
    return 0;
}

static int AdicionarLinha(Caminho *plano, Matriz destinoLinha)
{
    Matriz inicio = CalcularReferencialGarra(&plano->pontos[plano->quantidade - 1]);
    float dx = destinoLinha.v[12] - inicio.v[12], dy = destinoLinha.v[13] - inicio.v[13];
    float dz = destinoLinha.v[14] - inicio.v[14];
    int n = (int)ceilf(sqrtf(dx * dx + dy * dy + dz * dz) / IK_PASSO_CARTESIANO);
    if (n < 1)
        n = 1;
    for (int i = 1; i <= n; ++i) {
        Matriz passo = destinoLinha;
        float t = (float)i / n;
        passo.v[12] = inicio.v[12] + t * dx;
        passo.v[13] = inicio.v[13] + t * dy;
        passo.v[14] = inicio.v[14] + t * dz;
        if (!AdicionarAlvo(plano, passo))
            return 0;
    }
    return 1;
}

static Matriz AlvoIntermediario(Matriz orientacao, float base, float altura)
{
    float radianos = base * PI / 180;
    orientacao.v[12] = IK_RAIO_INTERMEDIARIO * cosf(radianos);
    orientacao.v[13] = altura;
    orientacao.v[14] = POSICAO_BASE_Z - IK_RAIO_INTERMEDIARIO * sinf(radianos);
    return orientacao;
}

static int PlanejarComAltura(Caminho *plano, Matriz alvo, float altura)
{
    Braco atual = ObterEstadoBraco();
    plano->quantidade = 1;
    plano->pontos[0] = atual;
    Matriz inicio = CalcularReferencialGarra(&atual);

    /* Primeiro retira a garra/carga da bancada sem girar o objeto apoiado. */
    if (inicio.v[13] < altura - 0.05f) {
        Matriz elevado = inicio;
        elevado.v[13] = altura;
        if (!AdicionarLinha(plano, elevado))
            return 0;
    }
    /* Abre somente depois de retirar os dedos dos blocos apoiados. Abrir
     * sobre uma vaga vizinha ocupada pode bater no objeto ja entregue. */
    Braco anterior = plano->pontos[plano->quantidade - 1], preparada = anterior;
    preparada.abertura = BlocoCapturado() >= 0 ? AberturaAssistida() : ABERTURA_MAXIMA;
    preparada.centroGarra = CentroAssistido();
    if (preparada.abertura != anterior.abertura || preparada.centroGarra != anterior.centroGarra) {
        if (TrajetoColide(&anterior, &preparada))
            return 0;
        plano->pontos[plano->quantidade++] = preparada;
    }
    float baseAtual = atan2f(-(inicio.v[14] - POSICAO_BASE_Z), inicio.v[12]) * 180 / PI;
    float baseFinal = atan2f(-(alvo.v[14] - POSICAO_BASE_Z), alvo.v[12]) * 180 / PI;
    Matriz intermediario = AlvoIntermediario(alvo, baseAtual, altura);
    if (!AdicionarAlvo(plano, intermediario))
        return 0;
    intermediario = AlvoIntermediario(alvo, baseFinal, altura);
    if (!AdicionarAlvo(plano, intermediario))
        return 0;
    Matriz acima = alvo;
    acima.v[13] = fmaxf(alvo.v[13], altura);
    if (!AdicionarAlvo(plano, acima))
        return 0;
    return AdicionarLinha(plano, alvo);
}

int MoverParaAlvo(Matriz alvo)
{
    Braco referencia = ObterEstadoBraco(), solucoes[IK_MAX_SOLUCOES];
    int quantidade;
    alvoAtual = alvo;
    possuiAlvo = 1;
    trajetoAtivo = 0;
    ResultadoIK resultado = ResolverIK(alvo, &referencia, solucoes, &quantidade);
    if (resultado == IK_ALVO_INVALIDO)
        possuiAlvo = 0;
    if (resultado != IK_OK) {
        mensagem = DescricaoResultadoIK(resultado);
        return 0;
    }
    if (!PoseDentroDosLimites(&referencia)) {
        mensagem = "IK: pose inicial fora dos limites; use B";
        return 0;
    }

    /* Tenta alturas de corredor diferentes, sempre validando cada trajeto.
     * Nao e um planejador completo: se nao houver caminho, mantem a pose. */
    Caminho plano;
    const float alturas[] = {IK_ALTURA_TRANSITO, 1.60f, 2.15f, 1.40f};
    int encontrou = 0;
    for (unsigned int i = 0; i < sizeof alturas / sizeof alturas[0] && !encontrou; ++i)
        encontrou = PlanejarComAltura(&plano, alvo, alturas[i]);
    if (!encontrou) {
        plano.quantidade = 1;
        plano.pontos[0] = referencia;
        encontrou = AdicionarAlvo(&plano, alvo);
    }
    if (!encontrou) {
        mensagem = "IK: sem trajeto livre neste corredor";
        return 0;
    }

    caminho = plano;
    pontoAtual = 1;
    tempoSegmento = 0;
    trajetoAtivo = caminho.quantidade > 1;
    LiberarControlesBraco();
    mensagem = "IK: executando trajeto validado";
    return 1;
}

int AproximarBlocoSelecionado(void)
{
    int bloco = BlocoSelecionado();
    if (BlocoCapturado() >= 0) {
        mensagem = "Ja ha uma carga; use Y para destino";
        return 0;
    }
    if (bloco < 0) {
        mensagem = "Selecione um bloco com o mouse";
        return 0;
    }
    Peca p = blocos[bloco];
    float yaw = atan2f(-p.mundo.v[2], p.mundo.v[0]) * 180 / PI;
    Matriz alvo = AlvoVertical(p.mundo.v[12], p.mundo.v[13] + IK_ALTURA_PEGA, p.mundo.v[14], yaw);
    return MoverParaAlvo(alvo);
}

int TransportarParaDestino(void)
{
    int indiceCarga = BlocoCapturado();
    float posicao[3];
    if (indiceCarga < 0) {
        mensagem = "Pegue um bloco com Espaco primeiro";
        return 0;
    }
    if (!ProximaVagaDestino(posicao)) {
        mensagem = "Destino ocupado: libere uma vaga";
        return 0;
    }
    Peca desejada = blocos[indiceCarga];
    NivelarPeca(&desejada);
    desejada.mundo.v[12] = posicao[0];
    desejada.mundo.v[14] = posicao[2];
    desejada.mundo.v[13] = 0;
    desejada.mundo.v[13] = -CalcularAabb(&desejada).minimo[1] + 0.025f;
    Matriz alvo = MultiplicarMatrizes(desejada.mundo, InversaRigida(RelativaCarga()));
    return MoverParaAlvo(alvo);
}

static float DuracaoSegmento(const Braco *a, const Braco *b)
{
    float duracao = 0.02f;
    for (int j = 0; j < TOTAL_JUNTAS; ++j)
        duracao = fmaxf(duracao, fabsf(ObterAngulo(b, (Junta)j) - ObterAngulo(a, (Junta)j)) /
                                     VELOCIDADE_JUNTA);
    duracao = fmaxf(duracao, fabsf(b->abertura - a->abertura) / VELOCIDADE_ABERTURA);
    return fmaxf(duracao, fabsf(b->centroGarra - a->centroGarra) / VELOCIDADE_ABERTURA);
}

static int ProximoBlocoDemo(void)
{
    for (int i = 0; i < TOTAL_BLOCOS; ++i)
        if (!BlocoNoDestino(i)) {
            SelecionarBloco(i);
            etapaDemo = DEMO_APROXIMANDO;
            int sucesso = AproximarBlocoSelecionado();
            if (!sucesso)
                etapaDemo = DEMO_DESLIGADA;
            return sucesso;
        }
    etapaDemo = DEMO_DESLIGADA;
    mensagem = "TAREFA CONCLUIDA: todos no destino";
    return 1;
}
int IniciarDemonstracao(void)
{
    CancelarMovimento();
    ReiniciarBraco();
    return ProximoBlocoDemo();
}
void CancelarMovimento(void)
{
    trajetoAtivo = 0;
    etapaDemo = DEMO_DESLIGADA;
    tempoEspera = 0;
    possuiAlvo = 0;
    LiberarControlesBraco();
    mensagem = "FK manual | T: aproximar | Y: destino";
}

/* Executa um unico trecho; a demonstracao decide o que fazer ao chegar. */
static void ExecutarSegmento(float segundos)
{
    Braco *a = &caminho.pontos[pontoAtual - 1], *b = &caminho.pontos[pontoAtual];
    float duracao = DuracaoSegmento(a, b);
    tempoSegmento = fminf(tempoSegmento + segundos, duracao);
    Braco pose = InterpolarPose(a, b, tempoSegmento / duracao), atual = ObterEstadoBraco();
    pose = DesenrolarPose(&atual, &pose);
    if (!AplicarPoseValidada(&pose, 1)) {
        trajetoAtivo = 0;
        etapaDemo = DEMO_DESLIGADA;
        mensagem = "IK interrompida: colisao ou limite";
        return;
    }
    if (tempoSegmento >= duracao) {
        ++pontoAtual;
        tempoSegmento = 0;
        if (pontoAtual >= caminho.quantidade) {
            trajetoAtivo = 0;
            mensagem = "IK: alvo alcancado | Espaco";
        }
    }
}

static void AtualizarDemonstracao(float segundos)
{
    /* A demonstracao usa as mesmas funcoes do usuario, sem teletransporte. */
    switch (etapaDemo) {
        case DEMO_APROXIMANDO:
            if (!AlternarSegurar()) {
                etapaDemo = DEMO_DESLIGADA;
                mensagem = "Demo: pega negada";
                break;
            }
            etapaDemo = DEMO_FECHANDO;
            tempoEspera = 0;
            break;
        case DEMO_FECHANDO:
            AtualizarBraco(segundos);
            tempoEspera += segundos;
            if (tempoEspera > 0.8f) {
                etapaDemo = DEMO_TRANSPORTANDO;
                if (!TransportarParaDestino())
                    etapaDemo = DEMO_DESLIGADA;
            }
            break;
        case DEMO_TRANSPORTANDO:
            if (!AlternarSegurar()) {
                etapaDemo = DEMO_DESLIGADA;
                mensagem = "Demo: soltura negada";
                break;
            }
            etapaDemo = DEMO_SOLTANDO;
            tempoEspera = 0;
            break;
        case DEMO_SOLTANDO:
            AtualizarBraco(segundos);
            tempoEspera += segundos;
            if (BlocoCapturado() < 0 && tempoEspera > 0.8f) {
                if (!ProximoBlocoDemo())
                    etapaDemo = DEMO_DESLIGADA;
            } else if (tempoEspera > 4) {
                etapaDemo = DEMO_DESLIGADA;
                mensagem = "Demo: nao foi possivel liberar a carga";
            }
            break;
        default:
            break;
    }
}
void AtualizarMovimento(float segundos)
{
    if (!isfinite(segundos) || segundos <= 0)
        return;
    if (trajetoAtivo)
        ExecutarSegmento(segundos);
    else
        AtualizarDemonstracao(segundos);
}

int MovimentoAutomaticoAtivo(void)
{
    return trajetoAtivo || etapaDemo != DEMO_DESLIGADA;
}
const char *EstadoMovimento(void)
{
    return mensagem;
}
int ObterAlvoMovimento(Matriz *alvo)
{
    *alvo = alvoAtual;
    return possuiAlvo;
}
float AlturaAlvoMouse(void)
{
    return alturaMouse;
}
void AlterarAlturaAlvoMouse(float delta)
{
    alturaMouse = Limitar(alturaMouse + delta, 0.3f, 3.0f);
}
