#include "aplicacao.h"
#include "braco.h"
#include "braco_config.h"
#include "braco_render.h"
#include "camera.h"
#include "cena.h"
#include "painel.h"
#include "manipulacao.h"
#include "movimento.h"
#include "visual_config.h"
#include <GL/freeglut.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Estado de interface privado; nao faz parte da camera ou da cinematica. */
static int mostrarEixos = 0; /* X revela os eixos quando forem uteis. */
static int arrastando = 0;
static int mouseX = 0, mouseY = 0;
static int tempoAnterior;
static double acumulador;
static int larguraJanela = 1000, alturaJanela = 700;

static void ProjetarCena(void)
{
    int largura = LarguraCena(larguraJanela);
    double aspecto = (double)largura / alturaJanela;
    double campo = aspecto < 1 ? 2 * atan(0.41421356237 / aspecto) * 57.295779513 : 45;
    glViewport(0, 0, largura, alturaJanela);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(campo, aspecto, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

static void AtualizarTitulo(void)
{
    Braco braco = ObterEstadoBraco();
    char titulo[512];
    snprintf(
        titulo, sizeof(titulo),
        "Braco 3D | Base %.0f | Ombro %.0f | Cotovelo %.0f | Punho %.0f | Lateral %.0f | Giro %.0f | Garra %.2f | H: ajuda%s",
        braco.base, braco.ombro, braco.cotovelo, braco.punho, braco.desvioPunho, braco.giroPunho,
        braco.abertura, MovimentoBloqueado() ? " | BLOQUEADO: colisao" : "");
    glutSetWindowTitle(titulo);
}

static void RenderizarQuadro(void)
{
    const GLfloat posicaoLuz[4] = {3.0f, 8.0f, 5.0f, 1.0f};
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, larguraJanela, alturaJanela);
    DesenharFundo();
    ProjetarCena();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    AplicarCamera();
    /* Definida depois da camera: luz fixa nas coordenadas do mundo. */
    glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz);
    DesenharCena();
    {
        Braco pose = ObterEstadoBraco();
        DesenharBraco(&pose);
    }
    if (mostrarEixos)
        DesenharEixos();
    DesenharPainel(larguraJanela, alturaJanela);
}

static void Desenhar(void)
{
    RenderizarQuadro();
    glutSwapBuffers();
}

static void Redimensionar(int largura, int altura)
{
    if (largura < 1)
        largura = 1;
    if (altura < 1)
        altura = 1;
    larguraJanela = largura;
    alturaJanela = altura;
    ProjetarCena();
}

static void Teclado(unsigned char tecla, int x, int y)
{
    (void)x;
    (void)y;
    if (tecla >= 'A' && tecla <= 'Z')
        tecla += 'a' - 'A';
    switch (tecla) {
        case 27:
            exit(EXIT_SUCCESS);
        case ' ':
            CancelarMovimento();
            (void)AlternarSegurar();
            break;
        case 'b':
            CancelarMovimento();
            ReiniciarBraco();
            break;
        case 't':
            CancelarMovimento();
            (void)AproximarBlocoSelecionado();
            break;
        case 'y':
            CancelarMovimento();
            (void)TransportarParaDestino();
            break;
        case 'c':
            CancelarMovimento();
            break;
        case 'v':
            CancelarMovimento();
            (void)AlternarLimitesFk();
            break;
        case 'r':
            ReiniciarCamera();
            break;
        case 'x':
            mostrarEixos = !mostrarEixos;
            break;
        case 'h':
            AlternarPainel();
            break;
        case '+':
        case '=':
            AproximarCamera(-0.5f);
            break;
        case '-':
        case '_':
            AproximarCamera(0.5f);
            break;
        default:
            if (DefinirTeclaBraco(tecla, 1) && MovimentoAutomaticoAtivo()) {
                CancelarMovimento();
                (void)DefinirTeclaBraco(tecla, 1);
            }
            break;
    }
    AtualizarTitulo();
    glutPostRedisplay();
}

static void SoltarTecla(unsigned char tecla, int x, int y)
{
    (void)x;
    (void)y;
    (void)DefinirTeclaBraco(tecla, 0);
}

static void EntradaMouse(int estado)
{
    if (estado == GLUT_LEFT)
        LiberarControlesBraco();
}

static void Visibilidade(int estado)
{
    if (estado != GLUT_VISIBLE)
        LiberarControlesBraco();
    tempoAnterior = glutGet(GLUT_ELAPSED_TIME);
    acumulador = 0;
}

static void AtualizarTempo(int valor)
{
    int agora = glutGet(GLUT_ELAPSED_TIME);
    double dt = (agora - tempoAnterior) / 1000.0;
    const double passo = 1.0 / FREQUENCIA_SIMULACAO;
    (void)valor;
    tempoAnterior = agora;
#ifdef _WIN32
    /* GLUT nao fornece evento de foco de teclado: evita tecla presa no Alt+Tab. */
    if (GetForegroundWindow() != WindowFromDC(wglGetCurrentDC())) {
        LiberarControlesBraco();
        dt = 0;
        acumulador = 0;
    }
#endif
    if (dt < 0)
        dt = 0;
    if (dt > 0.1)
        dt = 0.1;
    acumulador += dt;
    while (acumulador >= passo) {
        if (MovimentoAutomaticoAtivo())
            AtualizarMovimento((float)passo);
        else
            AtualizarBraco((float)passo);
        acumulador -= passo;
    }
    AtualizarTitulo();
    glutPostRedisplay();
    glutTimerFunc(8, AtualizarTempo, 0);
}

static void TeclasEspeciais(int tecla, int x, int y)
{
    (void)x;
    (void)y;
    switch (tecla) {
        case GLUT_KEY_LEFT:
            Orbitar(-5, 0);
            break;
        case GLUT_KEY_RIGHT:
            Orbitar(5, 0);
            break;
        case GLUT_KEY_UP:
            Orbitar(0, 5);
            break;
        case GLUT_KEY_DOWN:
            Orbitar(0, -5);
            break;
        case GLUT_KEY_PAGE_UP:
            AlterarAlturaAlvoMouse(0.1f);
            break;
        case GLUT_KEY_PAGE_DOWN:
            AlterarAlturaAlvoMouse(-0.1f);
            break;
        case GLUT_KEY_F5:
            (void)IniciarDemonstracao();
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

/* Desfaz a projecao em dois pontos para obter o raio do clique no mundo. */
static int CalcularRaioMouse(int x, int y, float origem[3], float direcao[3])
{
    GLdouble modelo[16], projecao[16], perto[3], longe[3];
    GLint viewport[4];
    ProjetarCena();
    glLoadIdentity();
    AplicarCamera();
    glGetDoublev(GL_MODELVIEW_MATRIX, modelo);
    glGetDoublev(GL_PROJECTION_MATRIX, projecao);
    glGetIntegerv(GL_VIEWPORT, viewport);
    if (!gluUnProject(x, alturaJanela - 1 - y, 0, modelo, projecao, viewport, &perto[0], &perto[1],
                      &perto[2]) ||
        !gluUnProject(x, alturaJanela - 1 - y, 1, modelo, projecao, viewport, &longe[0], &longe[1],
                      &longe[2]))
        return 0;
    for (int i = 0; i < 3; ++i) {
        origem[i] = (float)perto[i];
        direcao[i] = (float)(longe[i] - perto[i]);
    }
    return 1;
}

static void MoverParaClique(const float origem[3], const float direcao[3])
{
    /* Intersecao com Y=0 fornece X/Z; PgUp/PgDn definem a altura do alvo. */
    if (fabsf(direcao[1]) <= 1e-6f)
        return;
    float t = -origem[1] / direcao[1];
    if (t < 0)
        return;
    CancelarMovimento();
    (void)MoverParaAlvo(
        AlvoVertical(origem[0] + t * direcao[0], AlturaAlvoMouse(), origem[2] + t * direcao[2], 0));
}

/* Compartilhada pelo callback e pelo teste real de projecao e picking. */
static void CliqueNaCena(int x, int y, int modificadores)
{
    float origem[3], direcao[3];
    Braco pose = ObterEstadoBraco();
    if (CalcularRaioMouse(x, y, origem, direcao)) {
        if (modificadores & GLUT_ACTIVE_CTRL)
            MoverParaClique(origem, direcao);
        else {
            if (BlocoCapturado() < 0)
                CancelarMovimento();
            (void)SelecionarPorRaio(&pose, origem, direcao);
        }
    }
    glutPostRedisplay();
}

static void Mouse(int botao, int estado, int x, int y)
{
    if (botao == GLUT_LEFT_BUTTON && estado == GLUT_DOWN) {
        if (CliquePainel(x, y, larguraJanela, alturaJanela)) {
            glutPostRedisplay();
            return;
        }
        if (x < LarguraCena(larguraJanela))
            CliqueNaCena(x, y, glutGetModifiers());
        return;
    }
    if (botao == GLUT_RIGHT_BUTTON && (estado == GLUT_UP || x < LarguraCena(larguraJanela))) {
        arrastando = (estado == GLUT_DOWN);
        mouseX = x;
        mouseY = y;
    }
}

static void ArrastarMouse(int x, int y)
{
    if (!arrastando)
        return;
    Orbitar((x - mouseX) * 0.3f, (mouseY - y) * 0.3f);
    mouseX = x;
    mouseY = y;
    glutPostRedisplay();
}

static void Inicializar(void)
{
    const GLfloat ambiente[4] = {0.25f, 0.25f, 0.25f, 1};
    const GLfloat difusa[4] = {0.85f, 0.85f, 0.85f, 1};
    const GLfloat especular[4] = {0.25f, 0.25f, 0.25f, 1};
    glClearColor(COR_FUNDO_ALTO, 1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    /* Corrige o comprimento das normais apos escalas nao uniformes. */
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, difusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, especular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, especular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32);
    ReiniciarCamera();
    ReiniciarBraco();
}

int ExecutarAplicacao(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("Braco robotico - FK parcial");
    Inicializar();
    AtualizarTitulo();
    glutDisplayFunc(Desenhar);
    glutReshapeFunc(Redimensionar);
    glutKeyboardFunc(Teclado);
    glutKeyboardUpFunc(SoltarTecla);
    glutIgnoreKeyRepeat(1);
    glutEntryFunc(EntradaMouse);
    glutVisibilityFunc(Visibilidade);
    glutSpecialFunc(TeclasEspeciais);
    glutMouseFunc(Mouse);
    glutMotionFunc(ArrastarMouse);
    tempoAnterior = glutGet(GLUT_ELAPSED_TIME);
    glutTimerFunc(8, AtualizarTempo, 0);
    glutMainLoop();
    return EXIT_SUCCESS;
}
