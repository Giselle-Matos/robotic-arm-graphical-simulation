#include "cena.h"
#include "cena_dados.h"
#include "geometria.h"
#include "manipulacao.h"
#include "movimento.h"
#include "apoio.h"
#include "visual_config.h"
#include <GL/glut.h>
#include <stdio.h>

void DesenharFundo(void)
{
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_QUADS);
    glColor3f(COR_FUNDO_BAIXO);
    glVertex2f(0, 0);
    glVertex2f(1, 0);
    glColor3f(COR_FUNDO_ALTO);
    glVertex2f(1, 1);
    glVertex2f(0, 1);
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}

void DesenharEixos(void)
{
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f(0, .04f, 0);
    glVertex3f(2, .04f, 0);
    glColor3f(0, .65f, 0);
    glVertex3f(0, .04f, 0);
    glVertex3f(0, 2, 0);
    glColor3f(0, .3f, 1);
    glVertex3f(0, .04f, 0);
    glVertex3f(0, .04f, 2);
    glEnd();
    glPopAttrib();
}

/* Rotulos coplanares: ajuste apenas de rasterizacao, sem relevo fisico. */
static void ComecarPintura(void)
{
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT | GL_VIEWPORT_BIT);
    glDisable(GL_LIGHTING);
    glLineWidth(1);
    glDepthRange(0, 0.999999);
}

static void DesenharBancada(void)
{
    float l = bancada.tamanho[0], a = bancada.tamanho[1], p = bancada.tamanho[2];
    float espessuraTampo = a / 6;
    glPushMatrix();
    glMultMatrixf(bancada.mundo.v);
    /* Duas superficies continuas; topo e espessura fisica sao preservados. */
    CaixaChanfrada(0, -espessuraTampo / 2, 0, l, a - espessuraTampo, p, .035f, COR_GRAFITE);
    CaixaChanfrada(0, (a - espessuraTampo) / 2, 0, l - .10f, espessuraTampo, p - .10f, .012f,
                   COR_TAMPO);
    glPopMatrix();
}

static void DesenharBloco(int indice)
{
    const Peca *p = &blocos[indice];
    glPushMatrix();
    glMultMatrixf(p->mundo.v);
    CaixaChanfrada(0, 0, 0, p->tamanho[0], p->tamanho[1], p->tamanho[2], .035f, p->cor[0],
                   p->cor[1], p->cor[2]);
    /* Uma unica identificacao no topo, sem molduras ou rotulos repetidos. */
    ComecarPintura();
    if (indice == 2)
        glColor3f(COR_GRAFITE);
    else
        glColor3f(COR_TEXTO);
    glTranslatef(-p->tamanho[0] * .30f, p->tamanho[1] / 2, p->tamanho[2] * .32f);
    glRotatef(-90, 1, 0, 0);
    float escala = p->tamanho[0] * .0015f;
    glScalef(escala, escala, escala);
    char rotulo[16];
    snprintf(rotulo, sizeof rotulo, "%02d", indice + 1);
    for (const char *letra = rotulo; *letra; ++letra)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, (unsigned char)*letra);
    glPopAttrib();
    glPopMatrix();
}

static void DesenharDestino(void)
{
    float l = destino.tamanho[0], a = destino.tamanho[1], p = destino.tamanho[2];
    glPushMatrix();
    glMultMatrixf(destino.mundo.v);
    /* Borda por superficies, sem linhas coplanares ou guias decorativas. */
    if (TotalEntregues() == TOTAL_BLOCOS)
        CaixaChanfrada(0, -a * .20f, 0, l, a * .60f, p, .006f, COR_MENTA);
    else
        CaixaChanfrada(0, -a * .20f, 0, l, a * .60f, p, .006f, COR_VERDE);
    CaixaChanfrada(0, a * .30f, 0, l - .08f, a * .40f, p - .08f, .004f, COR_DESTINO);
    glPopMatrix();
}

static void DesenharAlvo(void)
{
    Matriz alvo;
    if (!ObterAlvoMovimento(&alvo))
        return;
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
    glDisable(GL_LIGHTING);
    glColor3f(COR_COBRE);
    glPushMatrix();
    glTranslatef(alvo.v[12], alvo.v[13], alvo.v[14]);
    glutSolidSphere(.035, 12, 8);
    glPopMatrix();
    glPopAttrib();
}

static void DestacarSelecionado(void)
{
    int indice = BlocoSelecionado();
    if (indice < 0)
        return;
    const Peca *p = &blocos[indice];
    Braco pose = ObterEstadoBraco();
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_LINE_BIT);
    glDisable(GL_LIGHTING);
    glLineWidth(1.25f);
    if (BlocoCapturado() >= 0)
        glColor3f(COR_COBRE);
    else if (ProntoParaPegar(&pose))
        glColor3f(COR_MENTA);
    else
        glColor3f(COR_TEXTO);
    glPushMatrix();
    glMultMatrixf(p->mundo.v);
    glScalef(p->tamanho[0] * 1.018f, p->tamanho[1] * 1.018f, p->tamanho[2] * 1.018f);
    glutWireCube(1);
    glPopMatrix();
    glPopAttrib();
}

void DesenharCena(void)
{
    DesenharBancada();
    for (int i = 0; i < TOTAL_BLOCOS; ++i)
        DesenharBloco(i);
    DesenharDestino();
    DesenharAlvo();
    DestacarSelecionado();
}
