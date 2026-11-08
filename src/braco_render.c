#include "braco_render.h"
#include "cinematica.h"
#include "geometria.h"
#include "braco_config.h"
#include "visual_config.h"
#include <GL/glut.h>

/* Silhuetas limpas e materiais acetinados, dentro das caixas fisicas existentes.
 * Os detalhes visuais nao participam da FK, captura ou colisao. */
static void AcabamentoMetal(void)
{
    const GLfloat reflexo[4] = {.36f, .39f, .36f, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, reflexo);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 48);
}
static void AcabamentoFosco(void)
{
    const GLfloat reflexo[4] = {.12f, .15f, .13f, 1};
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, reflexo);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 24);
}

static void DesenharArticulacao(const Peca *p)
{
    float l = p->tamanho[0], a = p->tamanho[1], prof = p->tamanho[2];
    AcabamentoFosco();
    CaixaChanfrada(0, 0, 0, l, a, prof * .82f, .045f, COR_GRAFITE);
    for (int lado = -1; lado <= 1; lado += 2) {
        glPushMatrix();
        glTranslatef(0, 0, lado * prof * .44f);
        AcabamentoMetal();
        CilindroZ(l * .37f, prof * .10f, 28, COR_ACO);
        glTranslatef(0, 0, lado * prof * .055f);
        AcabamentoFosco();
        CilindroZ(l * .23f, prof * .01f, 28, COR_GRAFITE);
        glPopMatrix();
    }
}

static void DesenharSegmento(const Peca *p)
{
    AcabamentoFosco();
    CaixaChanfrada(0, 0, 0, p->tamanho[0], p->tamanho[1], p->tamanho[2], .035f, COR_VERDE);
}

static void DesenharPunho(const Peca *p)
{
    float raio = p->tamanho[0] / 2, comprimento = p->tamanho[1];
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    AcabamentoMetal();
    CilindroZ(raio * .88f, comprimento, 28, COR_ALUMINIO);
    AcabamentoFosco();
    for (int lado = -1; lado <= 1; lado += 2) {
        glPushMatrix();
        glTranslatef(0, 0, lado * comprimento * .35f);
        CilindroZ(raio, comprimento * .16f, 28, COR_GRAFITE);
        glPopMatrix();
    }
    glPopMatrix();
}

static void DesenharPalma(const Peca *p, const Braco *braco)
{
    float l = p->tamanho[0], a = p->tamanho[1], prof = p->tamanho[2];
    AcabamentoFosco();
    CaixaChanfrada(0, 0, 0, l, a * .78f, prof * .84f, CHANFRO_CARCACA, COR_GRAFITE);
    for (int face = -1; face <= 1; face += 2)
        CaixaChanfrada(0, 0, face * prof * .455f, l - .20f, a * .46f, prof * .09f, .006f,
                       COR_VERDE);

    /* Mantem somente o trilho e os cursores que explicam o movimento da pinca. */
    AcabamentoMetal();
    for (int lado = -1; lado <= 1; lado += 2) {
        CaixaChanfrada(lado * (l / 2 - .05f), 0, 0, .10f, a, prof * .94f, .012f, COR_ACO);
        Caixa(0, a * .405f, lado * prof * .22f, l - .26f, a * .08f, prof * .09f, COR_ACO);
    }
    float afastamento = (braco->abertura + ESPESSURA_DEDO) / 2;
    for (int lado = -1; lado <= 1; lado += 2)
        CaixaChanfrada(braco->centroGarra + lado * afastamento, a * .28f, 0, ESPESSURA_DEDO,
                       a * .42f, prof * .80f, .008f, COR_ALUMINIO);
}

static void DesenharDedo(const Peca *p, int sentidoInterno)
{
    float l = p->tamanho[0], a = p->tamanho[1], prof = p->tamanho[2];
    AcabamentoMetal();
    CaixaChanfrada(-sentidoInterno * l * .10f, 0, 0, l * .80f, a, prof * .94f, CHANFRO_DEDO,
                   COR_ALUMINIO);
    /* Apoio continuo: a face interna coincide com o vao usado pela captura. */
    AcabamentoFosco();
    CaixaChanfrada(sentidoInterno * l * .40f, a * .06f, 0, l * .20f, a * .76f, prof * .82f, .005f,
                   COR_BORRACHA);
}

void DesenharBraco(const Braco *braco)
{
    Peca pecas[TOTAL_PECAS_BRACO];
    CalcularPecasBraco(braco, pecas);
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
    for (int i = 0; i < TOTAL_PECAS_BRACO; ++i) {
        const Peca *p = &pecas[i];
        glPushMatrix();
        glMultMatrixf(p->mundo.v);
        switch (i) {
            case PECA_OMBRO:
            case PECA_COTOVELO:
            case PECA_PIVO_PUNHO:
                DesenharArticulacao(p);
                break;
            case PECA_BRACO:
            case PECA_ANTEBRACO:
                DesenharSegmento(p);
                break;
            case PECA_PUNHO:
                DesenharPunho(p);
                break;
            case PECA_PALMA:
                DesenharPalma(p, braco);
                break;
            case PECA_DEDO_ESQUERDO:
                DesenharDedo(p, 1);
                break;
            case PECA_DEDO_DIREITO:
                DesenharDedo(p, -1);
                break;
            default:
                AcabamentoFosco();
                CaixaChanfrada(0, 0, 0, p->tamanho[0], p->tamanho[1], p->tamanho[2], .025f,
                               p->cor[0], p->cor[1], p->cor[2]);
                break;
        }
        glPopMatrix();
    }
    glPopAttrib();
}
