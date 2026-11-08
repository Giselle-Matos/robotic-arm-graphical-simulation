#include "apoio.h"
#include "aabb.h"
#include "colisao.h"
#include "manipulacao.h"
#include "simulacao_config.h"
#include "matematica.h"
#include <math.h>
#include <float.h>

static int ContemPegada(Aabb apoio, Aabb objeto, float margem)
{
    return objeto.minimo[0] >= apoio.minimo[0] + margem &&
           objeto.maximo[0] <= apoio.maximo[0] - margem &&
           objeto.minimo[2] >= apoio.minimo[2] + margem &&
           objeto.maximo[2] <= apoio.maximo[2] - margem;
}

static float RotacaoDeNivelamento(const Peca *peca, float eixo[3])
{
    int melhor = 0;
    for (int coluna = 1; coluna < 3; ++coluna)
        if (fabsf(peca->mundo.v[4 * coluna + 1]) > fabsf(peca->mundo.v[4 * melhor + 1]))
            melhor = coluna;
    float sinal = peca->mundo.v[4 * melhor + 1] < 0 ? -1.0f : 1.0f;
    float nx = sinal * peca->mundo.v[4 * melhor], ny = sinal * peca->mundo.v[4 * melhor + 1];
    float nz = sinal * peca->mundo.v[4 * melhor + 2];
    float norma = hypotf(nx, nz);
    eixo[0] = norma > 1e-6f ? -nz / norma : 1;
    eixo[1] = 0;
    eixo[2] = norma > 1e-6f ? nx / norma : 0;
    return acosf(Limitar(ny, -1, 1)) * 180 / PI;
}

static Peca GirarNoCentro(Peca peca, float angulo, const float eixo[3])
{
    Matriz rotacao = Identidade(), original = peca.mundo;
    Rotacionar(&rotacao, angulo, eixo[0], eixo[1], eixo[2]);
    peca.mundo = MultiplicarMatrizes(rotacao, original);
    for (int i = 0; i < 3; ++i)
        peca.mundo.v[12 + i] = original.v[12 + i];
    return peca;
}

float NivelarPeca(Peca *peca)
{
    float eixo[3], angulo = RotacaoDeNivelamento(peca, eixo);
    *peca = GirarNoCentro(*peca, angulo, eixo);
    return angulo;
}

static int EncontrarApoio(int indice, Peca objeto, float *altura)
{
    Aabb caixa = CalcularAabb(&objeto);
    float menorDistancia = FLT_MAX;
    int encontrou = 0;
    for (int apoio = -1; apoio < TOTAL_BLOCOS; ++apoio) {
        if (apoio >= 0 && (apoio == indice || apoio == BlocoCapturado()))
            continue;
        Aabb base = CalcularAabb(apoio < 0 ? &bancada : &blocos[apoio]);
        float distancia = fabsf(caixa.minimo[1] - base.maximo[1]);
        if (ContemPegada(base, caixa, 0) && distancia <= SOLTURA_DISTANCIA_APOIO &&
            distancia < menorDistancia) {
            *altura = base.maximo[1];
            menorDistancia = distancia;
            encontrou = 1;
        }
    }
    return encontrou;
}

static int PosicaoSoltaLivre(int indice, const Peca *solto, const Peca pecas[TOTAL_PECAS_BRACO])
{
    if (PecasColidem(solto, &bancada))
        return 0;
    for (int i = 0; i < TOTAL_BLOCOS; ++i)
        if (i != indice && PecasColidem(solto, &blocos[i]))
            return 0;
    /* Os dedos so serao liberados depois de abertos; outras partes nunca
     * podem ser atravessadas pelo pequeno assentamento do objeto. */
    for (int i = 0; i < PECA_DEDO_ESQUERDO; ++i)
        if (PecasColidem(solto, &pecas[i]))
            return 0;
    return 1;
}

int PrepararSoltura(int indice, const Braco *pose, Peca *solto, const char **motivo)
{
    Peca pecas[TOTAL_PECAS_BRACO], original;
    float altura, eixo[3], angulo;
    if (indice < 0 || indice >= TOTAL_BLOCOS) {
        *motivo = "Nenhum bloco para soltar";
        return 0;
    }
    original = blocos[indice];
    *solto = original;
    angulo = RotacaoDeNivelamento(&original, eixo);
    if (angulo > SOLTURA_INCLINACAO_MAXIMA) {
        *motivo = "Nivele a carga antes de soltar";
        return 0;
    }
    *solto = GirarNoCentro(original, angulo, eixo);
    if (!EncontrarApoio(indice, *solto, &altura)) {
        *motivo = "Aproxime a carga de um apoio valido";
        return 0;
    }
    solto->mundo.v[13] += altura - CalcularAabb(solto).minimo[1];
    CalcularPecasBraco(pose, pecas);

    /* Verifica tambem o ajuste entre a pose atual e a assentada. */
    const int amostras = SOLTURA_SUBDIVISOES;
    for (int i = 0; i <= amostras; ++i) {
        float t = (float)i / amostras;
        Peca teste = GirarNoCentro(original, angulo * t, eixo);
        teste.mundo.v[13] = original.mundo.v[13] + t * (solto->mundo.v[13] - original.mundo.v[13]);
        if (!PosicaoSoltaLivre(indice, &teste, pecas)) {
            *motivo = "Apoio ocupado ou braco no caminho";
            return 0;
        }
    }
    *motivo = 0;
    return 1;
}

int BlocoNoDestino(int indice)
{
    if (indice < 0 || indice >= TOTAL_BLOCOS || indice == BlocoCapturado())
        return 0;
    Aabb caixa = CalcularAabb(&blocos[indice]), area = CalcularAabb(&destino);
    float altura;
    if (!ContemPegada(area, caixa, DESTINO_MARGEM) ||
        !EncontrarApoio(indice, blocos[indice], &altura))
        return 0;
    return fabsf(caixa.minimo[1] - altura) < 0.005f;
}
int TotalEntregues(void)
{
    int total = 0;
    for (int i = 0; i < TOTAL_BLOCOS; ++i)
        total += BlocoNoDestino(i);
    return total;
}
int ProximaVagaDestino(float posicao[3])
{
    /* Quatro vagas em uma grade 2x2: face de apoio livre para tres blocos. */
    const float deslocamentos[4][2] = {{-.52f, -.52f}, {.52f, -.52f}, {-.52f, .52f}, {.52f, .52f}};
    for (int vaga = 0; vaga < 4; ++vaga) {
        Peca teste = blocos[BlocoCapturado() >= 0 ? BlocoCapturado() : 0];
        teste.mundo = Identidade();
        NivelarPeca(&teste);
        posicao[0] = destino.mundo.v[12] + deslocamentos[vaga][0];
        posicao[1] = teste.tamanho[1] / 2;
        posicao[2] = destino.mundo.v[14] + deslocamentos[vaga][1];
        Transladar(&teste.mundo, posicao[0], posicao[1], posicao[2]);
        int livre = 1;
        for (int i = 0; i < TOTAL_BLOCOS; ++i)
            if (i != BlocoCapturado() && PecasColidem(&teste, &blocos[i]))
                livre = 0;
        if (livre && ContemPegada(CalcularAabb(&destino), CalcularAabb(&teste), DESTINO_MARGEM))
            return 1;
    }
    return 0;
}
