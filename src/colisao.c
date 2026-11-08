#include "colisao.h"
#include "cena_dados.h"
#include "aabb.h"
#include "manipulacao.h"
#include "simulacao_config.h"
#include "juntas.h"
#include <math.h>
#include <float.h>

/* Intersecao raio/OBB por slabs no referencial local. Direcao nao precisa
 * ser normalizada; o mesmo parametro t permite ordenar todos os objetos. */
float DistanciaRaioPeca(const Peca *p, const float origem[3], const float direcao[3])
{
    Matriz inv = InversaRigida(p->mundo);
    float entrada = 0, saida = FLT_MAX;
    int eixo, k;
    for (eixo = 0; eixo < 3; ++eixo) {
        float o = inv.v[12 + eixo], d = 0, h = p->tamanho[eixo] / 2;
        for (k = 0; k < 3; ++k) {
            o += inv.v[4 * k + eixo] * origem[k];
            d += inv.v[4 * k + eixo] * direcao[k];
        }
        if (fabsf(d) < 1e-8f) {
            if (o < -h || o > h)
                return FLT_MAX;
        } else {
            float a = (-h - o) / d, b = (h - o) / d;
            entrada = fmaxf(entrada, fminf(a, b));
            saida = fminf(saida, fmaxf(a, b));
            if (entrada > saida)
                return FLT_MAX;
        }
    }
    return entrada;
}
static float Produto(const float a[3], const float b[3])
{
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

/* SAT: projeta centros e meias-extensoes no eixo candidato.
 * Eixos quase nulos (arestas paralelas) nao sao candidatos validos. */
static int Separadas(const Peca *a, const Peca *b, const float eixo[3])
{
    float norma = sqrtf(Produto(eixo, eixo));
    float delta[3], raio = 0;
    int i;
    if (norma < 1e-6f)
        return 0;
    for (i = 0; i < 3; ++i) {
        delta[i] = b->mundo.v[12 + i] - a->mundo.v[12 + i];
        raio += a->tamanho[i] / 2 * fabsf(Produto(&a->mundo.v[4 * i], eixo));
        raio += b->tamanho[i] / 2 * fabsf(Produto(&b->mundo.v[4 * i], eixo));
    }
    return fabsf(Produto(delta, eixo)) >= raio - 1e-5f * norma;
}

int PecasColidem(const Peca *a, const Peca *b)
{
    static const float eixosMundo[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    int i, j;
    /* Fase ampla: AABBs obtidas por projecao nos eixos globais. */
    for (i = 0; i < 3; ++i)
        if (Separadas(a, b, eixosMundo[i]))
            return 0;
    /* Refinamento OBB/SAT: 3 eixos de cada caixa e 9 produtos vetoriais. */
    for (i = 0; i < 3; ++i) {
        if (Separadas(a, b, &a->mundo.v[4 * i]))
            return 0;
        if (Separadas(a, b, &b->mundo.v[4 * i]))
            return 0;
        for (j = 0; j < 3; ++j) {
            const float *u = &a->mundo.v[4 * i], *v = &b->mundo.v[4 * j];
            float eixo[3] = {u[1] * v[2] - u[2] * v[1], u[2] * v[0] - u[0] * v[2],
                             u[0] * v[1] - u[1] * v[0]};
            if (Separadas(a, b, eixo))
                return 0;
        }
    }
    return 1;
}

int PecasBracoColidem(const Peca pecas[TOTAL_PECAS_BRACO], int a, int b)
{
    if (a > b) {
        int troca = a;
        a = b;
        b = troca;
    }
    /* Pares que formam a mesma montagem rigida ou compartilham a caixa do
     * pivo. A lista e pequena e visivel para explicar/alterar na apresentacao. */
    static const int encaixes[][2] = {
        {PECA_BASE, PECA_MARCADOR},       {PECA_BASE, PECA_SUPORTE},
        {PECA_MARCADOR, PECA_SUPORTE},    {PECA_SUPORTE, PECA_OMBRO},
        {PECA_OMBRO, PECA_BRACO},         {PECA_BRACO, PECA_COTOVELO},
        {PECA_COTOVELO, PECA_ANTEBRACO},  {PECA_ANTEBRACO, PECA_PIVO_PUNHO},
        {PECA_PIVO_PUNHO, PECA_PUNHO},    {PECA_PUNHO, PECA_PALMA},
        {PECA_PALMA, PECA_DEDO_ESQUERDO}, {PECA_PALMA, PECA_DEDO_DIREITO}};
    for (unsigned int i = 0; i < sizeof encaixes / sizeof encaixes[0]; ++i)
        if (a == encaixes[i][0] && b == encaixes[i][1])
            return 0;
    Peca primeira = pecas[a], segunda = pecas[b];
    /* Apenas nos pares que encontram o mesmo pivo, retira o volume da junta
     * das extremidades. O resto dos segmentos ainda colide quando dobra. */
    if ((a == PECA_BRACO && b == PECA_ANTEBRACO) || (a == PECA_SUPORTE && b == PECA_BRACO) ||
        (a == PECA_ANTEBRACO && b == PECA_PUNHO)) {
        if (a == PECA_BRACO || a == PECA_ANTEBRACO)
            primeira.tamanho[1] -= 0.70f;
        if (b == PECA_BRACO || b == PECA_ANTEBRACO)
            segunda.tamanho[1] -= 0.70f;
    }
    return PecasColidem(&primeira, &segunda);
}

int PoseColideComBlocos(const Braco *pose, const Peca previstos[], int indiceCarga)
{
    Peca pecas[TOTAL_PECAS_BRACO + 1 + TOTAL_BLOCOS];
    ArvoreAabb arvore;
    int candidatos[CAPACIDADE_AABB];
    int i, j, k, n;
    int transportado = indiceCarga < 0 ? -1 : TOTAL_PECAS_BRACO + 1 + indiceCarga;
    CalcularPecasBraco(pose, pecas);
    pecas[TOTAL_PECAS_BRACO] = bancada;
    for (i = 0; i < TOTAL_BLOCOS; ++i)
        pecas[TOTAL_PECAS_BRACO + 1 + i] = previstos[i];
    ConstruirArvoreAabb(&arvore, pecas, TOTAL_PECAS_BRACO + 1 + TOTAL_BLOCOS);
    for (i = 0; i < TOTAL_PECAS_BRACO + 1 + TOTAL_BLOCOS; ++i) {
        n = ConsultarArvoreAabb(&arvore, CalcularAabb(&pecas[i]), candidatos);
        for (k = 0; k < n; ++k) {
            j = candidatos[k];
            if (j <= i)
                continue;
            if (i >= TOTAL_PECAS_BRACO && i != transportado && j != transportado)
                continue;
            /* Apenas o par de dedos que segura o bloco tem contato permitido. */
            if (j == transportado && (i == PECA_DEDO_ESQUERDO || i == PECA_DEDO_DIREITO))
                continue;
            if (j < TOTAL_PECAS_BRACO) {
                if (PecasBracoColidem(pecas, i, j))
                    return 1;
                continue;
            }
            /* Pedestal e suporte sao fixados diretamente na bancada. */
            if (j == TOTAL_PECAS_BRACO && i <= PECA_SUPORTE)
                continue;
            if (PecasColidem(&pecas[i], &pecas[j]))
                return 1;
        }
    }
    return 0;
}

int PoseColide(const Braco *pose)
{
    Peca previstos[TOTAL_BLOCOS];
    PreverBlocos(pose, previstos);
    return PoseColideComBlocos(pose, previstos, BlocoCapturado());
}

int TrajetoColide(const Braco *inicio, const Braco *fim)
{
    /* Soma angular <= 0.25 grau e vao <= 0.005 por amostra, inclusive
     * movimentos simultaneos. Nao e CCD exata. Angulos sem wrap na entrada. */
    float variacao = 0;
    for (int junta = 0; junta < TOTAL_JUNTAS; ++junta)
        variacao += fabsf(ObterAngulo(fim, (Junta)junta) - ObterAngulo(inicio, (Junta)junta));
    if (!isfinite(variacao) || variacao > 3600 || !isfinite(fim->abertura) ||
        !isfinite(fim->centroGarra) || !isfinite(inicio->abertura) ||
        !isfinite(inicio->centroGarra))
        return 1;
    float deslocamento = fmaxf(fabsf(fim->abertura - inicio->abertura),
                               fabsf(fim->centroGarra - inicio->centroGarra));
    int amostras =
        (int)ceilf(fmaxf(variacao / COLISAO_PASSO_ANGULAR, deslocamento / COLISAO_PASSO_LINEAR));
    if (amostras < 1)
        amostras = 1;
    for (int i = 1; i <= amostras; ++i) {
        float t = (float)i / amostras;
        Braco pose = InterpolarPose(inicio, fim, t);
        if (PoseColide(&pose))
            return 1;
    }
    return 0;
}
