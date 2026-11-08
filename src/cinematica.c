#include "cinematica.h"
#include "braco_config.h"
#include "simulacao_config.h"
#include "visual_config.h"

static void DefinirPeca(Peca *pecas, IndicePeca indice, Matriz m, float x, float y, float z,
                        float largura, float altura, float profundidade, float r, float g, float b)
{
    Peca *peca = &pecas[indice];
    Transladar(&m, x, y, z);
    *peca = (Peca){m, {largura, altura, profundidade}, {r, g, b}};
}

void CalcularPecasBraco(const Braco *braco, Peca pecas[TOTAL_PECAS_BRACO])
{
    /* m acompanha os pivos, sem escala. DefinirPeca recebe uma copia para
     * colocar cada centro sem deslocar o referencial das pecas seguintes. */
    Matriz m = Identidade();
    float afastamento = (braco->abertura + ESPESSURA_DEDO) / 2;
    float centroY = (ALTURA_PALMA + COMPRIMENTO_DEDO) / 2;
    Transladar(&m, 0, 0, POSICAO_BASE_Z);
    DefinirPeca(pecas, PECA_BASE, m, 0, ALTURA_BASE / 2, 0, 1.2f, ALTURA_BASE, 1.2f, COR_GRAFITE);
    Transladar(&m, 0, ALTURA_BASE, 0);
    Rotacionar(&m, braco->base, 0, 1, 0);
    DefinirPeca(pecas, PECA_MARCADOR, m, .42f, .06f, 0, .3f, .12f, .25f, COR_VERDE);
    DefinirPeca(pecas, PECA_SUPORTE, m, 0, ELEVACAO_OMBRO / 2, 0, .4f, ELEVACAO_OMBRO, .4f,
                COR_GRAFITE);
    Transladar(&m, 0, ELEVACAO_OMBRO, 0);
    Rotacionar(&m, -braco->ombro, 0, 0, 1);
    DefinirPeca(pecas, PECA_OMBRO, m, 0, 0, 0, .65f, .65f, .65f, COR_GRAFITE);
    DefinirPeca(pecas, PECA_BRACO, m, 0, COMPRIMENTO_BRACO / 2, 0, .38f, COMPRIMENTO_BRACO, .38f,
                COR_VERDE);
    Transladar(&m, 0, COMPRIMENTO_BRACO, 0);
    DefinirPeca(pecas, PECA_COTOVELO, m, 0, 0, 0, .5f, .5f, .5f, COR_ACO);
    Rotacionar(&m, -braco->cotovelo, 0, 0, 1);
    DefinirPeca(pecas, PECA_ANTEBRACO, m, 0, COMPRIMENTO_ANTEBRACO / 2, 0, .30f,
                COMPRIMENTO_ANTEBRACO, .30f, COR_VERDE);
    Transladar(&m, 0, COMPRIMENTO_ANTEBRACO, 0);
    DefinirPeca(pecas, PECA_PIVO_PUNHO, m, 0, 0, 0, .4f, .4f, .4f, COR_ACO);
    /* Ordem local: inclinacao Z (J/L), desvio X (I/K), giro Y (O/P).
     * Y aponta ao longo do punho e dos dedos: giro nao muda o eixo da pinca. */
    Rotacionar(&m, -braco->punho, 0, 0, 1);
    Rotacionar(&m, braco->desvioPunho, 1, 0, 0);
    Rotacionar(&m, braco->giroPunho, 0, 1, 0);
    DefinirPeca(pecas, PECA_PUNHO, m, 0, COMPRIMENTO_PUNHO / 2, 0, .24f, COMPRIMENTO_PUNHO, .24f,
                COR_VERDE);
    Transladar(&m, 0, COMPRIMENTO_PUNHO + ALTURA_PALMA / 2, 0);
    DefinirPeca(pecas, PECA_PALMA, m, 0, 0, 0,
                ABERTURA_MAXIMA + 2 * ESPESSURA_DEDO + 2 * CAPTURA_DESLOCAMENTO_MAXIMO,
                ALTURA_PALMA, .36f, COR_GRAFITE);
    DefinirPeca(pecas, PECA_DEDO_ESQUERDO, m, braco->centroGarra - afastamento, centroY, 0,
                ESPESSURA_DEDO, COMPRIMENTO_DEDO, PROFUNDIDADE_DEDO, COR_ALUMINIO);
    DefinirPeca(pecas, PECA_DEDO_DIREITO, m, braco->centroGarra + afastamento, centroY, 0,
                ESPESSURA_DEDO, COMPRIMENTO_DEDO, PROFUNDIDADE_DEDO, COR_ALUMINIO);
}

Matriz CalcularReferencialGarra(const Braco *braco)
{
    Peca pecas[TOTAL_PECAS_BRACO];
    CalcularPecasBraco(braco, pecas);
    Matriz referencia = pecas[PECA_PALMA].mundo;
    Transladar(&referencia, 0, (ALTURA_PALMA + COMPRIMENTO_DEDO) / 2, 0);
    return referencia;
}
