#include "ik.h"
#include "juntas.h"
#include "braco_config.h"
#include "matematica.h"
#include "simulacao_config.h"
#include <math.h>

static int ReferencialValido(Matriz alvo)
{
    for (int i = 0; i < 16; ++i)
        if (!isfinite(alvo.v[i]))
            return 0;
    for (int i = 0; i < 3; ++i)
        for (int j = i; j < 3; ++j) {
            float produto = 0;
            for (int k = 0; k < 3; ++k)
                produto += alvo.v[4 * i + k] * alvo.v[4 * j + k];
            if (fabsf(produto - (i == j ? 1.0f : 0.0f)) > 0.001f)
                return 0;
        }
    float determinante = alvo.v[0] * (alvo.v[5] * alvo.v[10] - alvo.v[9] * alvo.v[6]) -
                         alvo.v[4] * (alvo.v[1] * alvo.v[10] - alvo.v[9] * alvo.v[2]) +
                         alvo.v[8] * (alvo.v[1] * alvo.v[6] - alvo.v[5] * alvo.v[2]);
    return fabsf(determinante - 1) < 0.001f && fabsf(alvo.v[15] - 1) < 0.001f &&
           fabsf(alvo.v[3]) + fabsf(alvo.v[7]) + fabsf(alvo.v[11]) < 0.001f;
}

static int SolucaoReproduzAlvo(const Braco *pose, Matriz alvo)
{
    Matriz calculado = CalcularReferencialGarra(pose);
    float erroPosicao = 0, erroOrientacao = 0;
    for (int i = 0; i < 3; ++i) {
        float delta = calculado.v[12 + i] - alvo.v[12 + i];
        erroPosicao += delta * delta;
        for (int j = 0; j < 3; ++j)
            erroOrientacao =
                fmaxf(erroOrientacao, fabsf(calculado.v[4 * j + i] - alvo.v[4 * j + i]));
    }
    return sqrtf(erroPosicao) <= IK_ERRO_POSICAO && erroOrientacao <= IK_ERRO_ORIENTACAO;
}

Matriz AlvoVertical(float x, float y, float z, float giroMundo)
{
    Matriz alvo = Identidade();
    Transladar(&alvo, x, y, z);
    Rotacionar(&alvo, giroMundo, 0, 1, 0);
    Rotacionar(&alvo, 180, 0, 0, 1);
    return alvo;
}

/* Retira as rotacoes da base/elos e resolve o restante nos tres eixos do punho.
 * A mesma ordem Z-X-Y deve ser mantida em cinematica.c. Angulos em radianos. */
static void ResolverOrientacaoPunho(Matriz alvo, float base, float ombro, float cotovelo,
                                    Braco *pose)
{
    const float graus = 180.0f / PI;
    Matriz pai = Identidade(), local;
    Rotacionar(&pai, base * graus, 0, 1, 0);
    Rotacionar(&pai, -(ombro + cotovelo) * graus, 0, 0, 1);
    local = MultiplicarMatrizes(InversaRigida(pai), alvo);

    /* R = Rz(a) Rx(b) Ry(c). R21=sen(b).
     * No polo, c=0 preserva uma solucao equivalente sem divisao por zero. */
    float desvio = asinf(Limitar(local.v[6], -1, 1));
    float inclinacao, giro;
    if (fabsf(cosf(desvio)) < 1e-5f) {
        inclinacao = atan2f(local.v[1], local.v[0]);
        giro = 0;
    } else {
        inclinacao = atan2f(-local.v[4], local.v[5]);
        giro = atan2f(-local.v[2], local.v[10]);
    }
    pose->punho = -inclinacao * graus;
    pose->desvioPunho = desvio * graus;
    pose->giroPunho = giro * graus;
}

ResultadoIK ResolverIK(Matriz alvo, const Braco *referencia, Braco solucoes[IK_MAX_SOLUCOES],
                       int *quantidade)
{
    const float graus = 180.0f / PI;
    const float deslocamento = DISTANCIA_PUNHO_GARRA;
    const float l1 = COMPRIMENTO_BRACO, l2 = COMPRIMENTO_ANTEBRACO;
    float x, y, z, raio, cossenoCotovelo, baseInicial;
    *quantidade = 0;
    if (!ReferencialValido(alvo))
        return IK_ALVO_INVALIDO;

    /* A origem da garra esta deslocada do punho ao longo do Y local. */
    x = alvo.v[12] - deslocamento * alvo.v[4];
    y = alvo.v[13] - deslocamento * alvo.v[5] - (ALTURA_BASE + ELEVACAO_OMBRO);
    z = alvo.v[14] - deslocamento * alvo.v[6] - POSICAO_BASE_Z;
    raio = hypotf(x, z);
    cossenoCotovelo = (raio * raio + y * y - l1 * l1 - l2 * l2) / (2 * l1 * l2);
    if (cossenoCotovelo < -1.00001f || cossenoCotovelo > 1.00001f)
        return IK_FORA_DE_ALCANCE;
    cossenoCotovelo = Limitar(cossenoCotovelo, -1, 1);
    baseInicial = raio < 1e-6f ? referencia->base / graus : atan2f(-z, x);

    /* Dois sentidos para a base x dois sinais do cotovelo = ate quatro poses.
     * A FK confere cada candidata antes de devolve-la ao planejador. */
    for (int ramoBase = 0; ramoBase < 2; ++ramoBase) {
        float base = baseInicial + (ramoBase ? PI : 0);
        float radial = ramoBase ? -raio : raio;
        for (int sinal = -1; sinal <= 1; sinal += 2) {
            float cotovelo = sinal * acosf(cossenoCotovelo);
            float ombro = atan2f(radial, y) - atan2f(l2 * sinf(cotovelo), l1 + l2 * cosf(cotovelo));
            Braco candidata = *referencia;
            candidata.base = base * graus;
            candidata.ombro = ombro * graus;
            candidata.cotovelo = cotovelo * graus;
            ResolverOrientacaoPunho(alvo, base, ombro, cotovelo, &candidata);
            if (PoseDentroDosLimites(&candidata) && SolucaoReproduzAlvo(&candidata, alvo))
                solucoes[(*quantidade)++] = candidata;
        }
    }
    return *quantidade ? IK_OK : IK_FORA_DOS_LIMITES;
}
const char *DescricaoResultadoIK(ResultadoIK resultado)
{
    switch (resultado) {
        case IK_OK:
            return "IK: solucao encontrada";
        case IK_ALVO_INVALIDO:
            return "IK: alvo invalido";
        case IK_FORA_DE_ALCANCE:
            return "IK: alvo fora do alcance";
        case IK_FORA_DOS_LIMITES:
            return "IK: alvo excede limites das juntas";
        default:
            return "IK: erro desconhecido";
    }
}
