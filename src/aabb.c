#include "aabb.h"
#include <math.h>

Aabb CalcularAabb(const Peca *peca)
{
    Aabb caixa;
    int eixo, coluna;
    for (eixo = 0; eixo < 3; ++eixo) {
        float raio = 0;
        for (coluna = 0; coluna < 3; ++coluna)
            raio += fabsf(peca->mundo.v[4 * coluna + eixo]) * peca->tamanho[coluna] / 2;
        caixa.minimo[eixo] = peca->mundo.v[12 + eixo] - raio;
        caixa.maximo[eixo] = peca->mundo.v[12 + eixo] + raio;
    }
    return caixa;
}

int AabbsSobrepoem(Aabb a, Aabb b)
{
    int i;
    for (i = 0; i < 3; ++i)
        if (a.maximo[i] < b.minimo[i] || b.maximo[i] < a.minimo[i])
            return 0;
    return 1;
}

static Aabb Unir(Aabb a, Aabb b)
{
    int i;
    for (i = 0; i < 3; ++i) {
        a.minimo[i] = fminf(a.minimo[i], b.minimo[i]);
        a.maximo[i] = fmaxf(a.maximo[i], b.maximo[i]);
    }
    return a;
}

/* A soma dos extremos equivale ao dobro do centro: evita dividir para ordenar. */
static float CentroDuplicado(Aabb caixa, int eixo)
{
    return caixa.minimo[eixo] + caixa.maximo[eixo];
}

static int EixoDeMaiorDispersao(const Aabb *caixas, const int *indices, int quantidade)
{
    float maiorDispersao = -1;
    int escolhido = 0;
    for (int eixo = 0; eixo < 3; ++eixo) {
        float minimo = CentroDuplicado(caixas[indices[0]], eixo), maximo = minimo;
        for (int i = 1; i < quantidade; ++i) {
            float centro = CentroDuplicado(caixas[indices[i]], eixo);
            minimo = fminf(minimo, centro);
            maximo = fmaxf(maximo, centro);
        }
        if (maximo - minimo > maiorDispersao) {
            maiorDispersao = maximo - minimo;
            escolhido = eixo;
        }
    }
    return escolhido;
}

/* Insertion sort estavel: simples e suficiente para as 16 caixas atuais. */
static void OrdenarPeloCentro(const Aabb *caixas, int *indices, int quantidade, int eixo)
{
    for (int i = 1; i < quantidade; ++i) {
        int id = indices[i], j = i;
        float centro = CentroDuplicado(caixas[id], eixo);
        while (j > 0 && CentroDuplicado(caixas[indices[j - 1]], eixo) > centro) {
            indices[j] = indices[j - 1];
            --j;
        }
        indices[j] = id;
    }
}

static int ConstruirNo(ArvoreAabb *arvore, const Aabb *caixas, int *indices, int quantidade)
{
    int atual = arvore->total++;
    NoAabb *no = &arvore->nos[atual];
    no->caixa = caixas[indices[0]];
    no->peca = -1;
    no->esquerda = no->direita = -1;
    for (int i = 1; i < quantidade; ++i)
        no->caixa = Unir(no->caixa, caixas[indices[i]]);
    if (quantidade == 1) {
        no->peca = indices[0];
        return atual;
    }
    int eixo = EixoDeMaiorDispersao(caixas, indices, quantidade);
    OrdenarPeloCentro(caixas, indices, quantidade, eixo);
    int metade = quantidade / 2;
    no->esquerda = ConstruirNo(arvore, caixas, indices, metade);
    no->direita = ConstruirNo(arvore, caixas, indices + metade, quantidade - metade);
    return atual;
}

int ConstruirArvoreAabb(ArvoreAabb *arvore, const Peca *pecas, int quantidade)
{
    Aabb caixas[CAPACIDADE_AABB];
    int indices[CAPACIDADE_AABB], i;
    arvore->total = 0;
    arvore->raiz = -1;
    if (quantidade < 0 || quantidade > CAPACIDADE_AABB)
        return 0;
    for (i = 0; i < quantidade; ++i) {
        caixas[i] = CalcularAabb(&pecas[i]);
        indices[i] = i;
    }
    if (quantidade)
        arvore->raiz = ConstruirNo(arvore, caixas, indices, quantidade);
    return 1;
}

static void ConsultarNo(const ArvoreAabb *arvore, int id, Aabb caixa, int *indices, int *n)
{
    const NoAabb *no;
    if (id < 0)
        return;
    no = &arvore->nos[id];
    if (!AabbsSobrepoem(no->caixa, caixa))
        return;
    if (no->peca >= 0)
        indices[(*n)++] = no->peca;
    else {
        ConsultarNo(arvore, no->esquerda, caixa, indices, n);
        ConsultarNo(arvore, no->direita, caixa, indices, n);
    }
}

int ConsultarArvoreAabb(const ArvoreAabb *arvore, Aabb caixa, int indices[CAPACIDADE_AABB])
{
    int n = 0;
    ConsultarNo(arvore, arvore->raiz, caixa, indices, &n);
    return n;
}
