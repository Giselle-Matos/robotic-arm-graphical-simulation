#include "painel.h"
#include "braco.h"
#include "manipulacao.h"
#include "movimento.h"
#include "apoio.h"
#include "visual_config.h"
#include <GL/freeglut.h>
#include <stdio.h>
#include <string.h>

static int aberto = 1;
static int compacto;
int LarguraCena(int largura)
{
    return aberto ? largura - (largura < 560 ? largura / 2 : 280) : largura;
}
void AlternarPainel(void)
{
    aberto = !aberto;
}

static void Retangulo(float x, float y, float largura, float altura)
{
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + largura, y);
    glVertex2f(x + largura, y + altura);
    glVertex2f(x, y + altura);
    glEnd();
}

static void Texto(float x, float y, const char *texto, int tamanho)
{
    /* Fontes bitmap mantem os caracteres preenchidos e legiveis, sem os
     * contornos tecnicos da fonte vetorial usada nas pecas da cena. */
    void *fonte = tamanho == 18   ? GLUT_BITMAP_HELVETICA_18
                  : tamanho == 10 ? GLUT_BITMAP_HELVETICA_10
                                  : GLUT_BITMAP_HELVETICA_12;
    if (compacto)
        fonte = GLUT_BITMAP_HELVETICA_10;
    glRasterPos2f(x, y);
    while (*texto)
        glutBitmapCharacter(fonte, (unsigned char)*texto++);
}

typedef struct {
    const char *tecla;
    const char *acao;
} Atalho;
static const Atalho movimento[] = {{"A / D", "Base: esq. / dir."},
                                   {"W / S", "Ombro: subir / descer"},
                                   {"Q / E", "Cotovelo: subir / descer"},
                                   {"J / L", "Punho: subir / descer"},
                                   {"I / K", "Pinca: frente / tras"},
                                   {"O / P", "Anti-horario / horario"},
                                   {"F / G", "Fechar / abrir"}};
static const Atalho interacao[] = {{"Clique", "Selecionar bloco"},
                                   {"T", "Aproximar"},
                                   {"Espaco", "Pegar / soltar"},
                                   {"Y", "Levar ao destino"}};
static const Atalho outros[] = {{"Setas / mouse", "Orbitar (botao dir.)"},
                                {"+ / - / R", "Zoom / restaurar"},
                                {"Ctrl + clique", "Alvo IK"},
                                {"PgUp / PgDn", "Altura do alvo"},
                                {"F5 / C", "Demo / cancelar"},
                                {"V / X", "Limites / eixos"},
                                {"B / H", "Reiniciar / ajuda"},
                                {"Esc", "Encerrar"}};

static void Secao(const char *titulo, float y, const Atalho *atalhos, int quantidade)
{
    glColor3f(COR_TEXTO_SUAVE);
    Texto(24, y, titulo, 10);
    for (int i = 0; i < quantidade; ++i) {
        float linha = y - 25 - i * 21;
        glColor3f(COR_ALUMINIO);
        Texto(24, linha, atalhos[i].tecla, 12);
        glColor3f(COR_TEXTO);
        Texto(112, linha, atalhos[i].acao, 12);
    }
}

static void EstadoDaTarefa(void)
{
    char texto[96];
    Braco pose = ObterEstadoBraco();
    int entregues = TotalEntregues(), carga = BlocoCapturado(), selecao = BlocoSelecionado();
    glColor3f(COR_TEXTO_SUAVE);
    Retangulo(24, 109, 232, .7f);
    glColor3f(COR_TEXTO);
    Texto(24, 92, entregues == TOTAL_BLOCOS ? "Tarefa concluida" : "Destino", 12);
    snprintf(texto, sizeof texto, "%d / %d", entregues, TOTAL_BLOCOS);
    glColor3f(COR_MENTA);
    Texto(231, 92, texto, 12);
    glColor3f(COR_GRAFITE);
    Retangulo(24, 81, 232, 3);
    glColor3f(COR_VERDE);
    Retangulo(24, 81, 232.0f * entregues / TOTAL_BLOCOS, 3);

    if (carga >= 0) {
        snprintf(texto, sizeof texto, "Bloco %02d em transporte", carga + 1);
        glColor3f(COR_COBRE);
    } else if (ProntoParaPegar(&pose)) {
        snprintf(texto, sizeof texto, "Pronto para pegar");
        glColor3f(COR_MENTA);
    } else if (selecao >= 0) {
        snprintf(texto, sizeof texto, "Bloco %02d selecionado", selecao + 1);
        glColor3f(COR_TEXTO);
    } else {
        snprintf(texto, sizeof texto, "Pinca livre");
        glColor3f(COR_TEXTO);
    }
    Texto(24, 60, texto, 12);
    glColor3f(COR_TEXTO_SUAVE);
    Texto(24, 43, InstrucaoCaptura(&pose), 10);
    if (MovimentoBloqueado()) {
        glColor3f(COR_COBRE);
        Texto(24, 25, "Contato ou limite de movimento", 10);
    } else if (strncmp(EstadoMovimento(), "FK manual", 9) != 0) {
        glColor3f(COR_TEXTO_SUAVE);
        Texto(24, 25, EstadoMovimento(), 10);
    }
    snprintf(texto, sizeof texto, "Limites %s  |  Altura alvo %.1f",
             LimitesFkAtivos() ? "ativos" : "livres", AlturaAlvoMouse());
    glColor3f(COR_TEXTO_SUAVE);
    Texto(24, 8, texto, 10);
}

void DesenharPainel(int largura, int altura)
{
    int inicio = LarguraCena(largura);
    compacto = altura < 600 || (aberto && largura - inicio < 260);
    glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT | GL_VIEWPORT_BIT | GL_LINE_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glLineWidth(1);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    if (aberto) {
        glViewport(inicio, 0, largura - inicio, altura);
        glOrtho(0, 280, 0, 700, -1, 1);
    } else {
        glViewport(0, 0, largura, altura);
        glOrtho(0, largura, 0, altura, -1, 1);
    }
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3f(COR_PAINEL);
    if (aberto) {
        Retangulo(0, 0, 280, 700);
        glColor3f(COR_TEXTO);
        Texto(24, 658, "Braco robotico", 18);
        glColor3f(COR_TEXTO_SUAVE);
        Texto(24, 637, "Simulacao 3D", 12);
        glBegin(GL_LINES);
        glVertex2f(247, 660);
        glVertex2f(255, 668);
        glVertex2f(247, 668);
        glVertex2f(255, 660);
        glEnd();
        Secao("MOVIMENTO", 603, movimento, (int)(sizeof movimento / sizeof movimento[0]));
        Secao("INTERACAO", 414, interacao, (int)(sizeof interacao / sizeof interacao[0]));
        Secao("CAMERA E OUTROS", 288, outros, (int)(sizeof outros / sizeof outros[0]));
        EstadoDaTarefa();
    } else {
        Retangulo((float)largura - 128, (float)altura - 36, 120, 28);
        glColor3f(COR_TEXTO);
        Texto((float)largura - 115, (float)altura - 26, "Controles   H", 12);
        char progresso[32];
        snprintf(progresso, sizeof progresso, "Destino  %d / %d", TotalEntregues(), TOTAL_BLOCOS);
        glColor3f(COR_ALUMINIO);
        Texto(18, 18, progresso, 12);
    }
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}

int CliquePainel(int x, int y, int largura, int altura)
{
    if (aberto && x >= LarguraCena(largura)) {
        if (x >= largura - (largura - LarguraCena(largura)) * .16f && y < altura * .08f)
            AlternarPainel();
        return 1;
    }
    if (!aberto && x >= largura - 128 && x <= largura - 8 && y >= 8 && y <= 36) {
        AlternarPainel();
        return 1;
    }
    return 0;
}
