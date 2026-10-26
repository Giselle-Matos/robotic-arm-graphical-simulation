/*
 * Braco robotico - Etapa 2c: punho e garra.
 * Windows / GCC / FreeGLUT / OpenGL classico.
 * Compilar:
 * .\compilar.ps1
 * Executar no PowerShell: .\build\robotic_arm.exe
 *
 * Setas: orbitar | +/-: zoom | botao direito + arrastar: orbitar
 * X: mostrar/ocultar eixos | R: restaurar camera | Esc: sair
 *
 * Convencao: Y para cima; bancada no plano XZ, superficie em Y=0.
 * A/D: base | W/S: ombro | B: restaurar braco (R restaura camera).
 * Q/E: cotovelo | J/L: punho | F/G: fechar/abrir garra.
 * FK discreta; ainda sem captura, colisao ou animacao temporal.
 */
#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define PI 3.14159265358979323846f

typedef struct {
    float azimute, elevacao, distancia;
    float alvoX, alvoY, alvoZ;
} Camera;

/* Comprimentos em unidades da cena; angulos em graus.
 * Base central, afastada dos blocos e da area de destino.
 * Ombro: 0 = vertical; positivo inclina para +X local.
 * Limite de 70 graus mantem este segmento acima da bancada.
 */
#define ALTURA_BASE 0.6f
#define COMPRIMENTO_BRACO 2.6f
#define COMPRIMENTO_ANTEBRACO 2.0f
#define ELEVACAO_OMBRO 0.5f
#define LIMITE_COTOVELO 135.0f
#define LIMITE_BASE 180.0f
#define LIMITE_OMBRO 70.0f
#define PASSO_JUNTA 5.0f
#define LIMITE_PUNHO 90.0f
#define COMPRIMENTO_PUNHO 0.45f
#define ALTURA_PALMA 0.18f
#define COMPRIMENTO_DEDO 0.9f
#define ESPESSURA_DEDO 0.12f
#define ABERTURA_MAXIMA 1.2f
#define PASSO_ABERTURA 0.1f

typedef struct {
    float base;
    float ombro;
    float cotovelo; /* Relativo ao braco: zero deixa os segmentos alinhados. */
    float punho; /* Inclinacao relativa ao antebraco, em graus. */
    float abertura; /* Distancia livre entre faces internas dos dedos. */
} Braco;

static Braco braco = {0.0f, 25.0f, 60.0f, 45.0f, ABERTURA_MAXIMA};
static Camera camera;

static void AtualizarTitulo(void)
{
    char titulo[256];
    snprintf(titulo, sizeof(titulo),
             "FK parcial | Base: %.0f (A/D) | Ombro: %.0f (W/S) | Cotovelo: %.0f (Q/E) | Punho: %.0f (J/L) | Garra: %.1f (F/G) | B: braco | R: camera",
             braco.base, braco.ombro, braco.cotovelo, braco.punho, braco.abertura);
    glutSetWindowTitle(titulo);
}

static void ReiniciarBraco(void)
{
    braco.base = 0.0f;
    braco.ombro = 25.0f;
    braco.cotovelo = 60.0f;
    braco.punho = 45.0f;
    braco.abertura = ABERTURA_MAXIMA;
}
static int mostrarEixos = 1;
static int arrastando = 0;
static int mouseX = 0, mouseY = 0;

static float Limitar(float valor, float minimo, float maximo)
{
    if (valor < minimo) return minimo;
    if (valor > maximo) return maximo;
    return valor;
}

static void ReiniciarCamera(void)
{
    camera.azimute = 35.0f;
    camera.elevacao = 30.0f;
    camera.distancia = 15.0f;
    camera.alvoX = 0.0f;
    camera.alvoY = 0.5f;
    camera.alvoZ = 0.0f;
}

static void Orbitar(float horizontal, float vertical)
{
    camera.azimute = fmodf(camera.azimute + horizontal, 360.0f);
    /* Evita alinhar a direcao de visao com o vetor vertical. */
    camera.elevacao = Limitar(camera.elevacao + vertical, 10.0f, 80.0f);
}

static void AplicarCamera(void)
{
    float a = camera.azimute * PI / 180.0f;
    float e = camera.elevacao * PI / 180.0f;
    float horizontal = camera.distancia * cosf(e);
    gluLookAt(camera.alvoX + horizontal * sinf(a),
              camera.alvoY + camera.distancia * sinf(e),
              camera.alvoZ + horizontal * cosf(a),
              camera.alvoX, camera.alvoY, camera.alvoZ,
              0.0, 1.0, 0.0);
}

/* Cubo unitario centrado na origem, com uma normal por face. */
static void CuboUnitario(void)
{
    static const GLfloat vertices[8][3] = {
        {-0.5f,-0.5f,-0.5f}, {0.5f,-0.5f,-0.5f},
        {0.5f,0.5f,-0.5f}, {-0.5f,0.5f,-0.5f},
        {-0.5f,-0.5f,0.5f}, {0.5f,-0.5f,0.5f},
        {0.5f,0.5f,0.5f}, {-0.5f,0.5f,0.5f}
    };
    static const int faces[6][4] = {
        {4,5,6,7}, {1,0,3,2}, {5,1,2,6},
        {0,4,7,3}, {7,6,2,3}, {0,1,5,4}
    };
    static const GLfloat normais[6][3] = {
        {0,0,1}, {0,0,-1}, {1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}
    };
    int face, vertice;
    glBegin(GL_QUADS);
    for (face = 0; face < 6; ++face) {
        glNormal3fv(normais[face]);
        for (vertice = 0; vertice < 4; ++vertice)
            glVertex3fv(vertices[faces[face][vertice]]);
    }
    glEnd();
}

/* A escala afeta somente este objeto, pois a matriz e restaurada ao final. */
static void Caixa(float x, float y, float z,
                  float largura, float altura, float profundidade,
                  float r, float g, float b)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(largura, altura, profundidade);
    glColor3f(r, g, b);
    CuboUnitario();
    glPopMatrix();
}

static void DesenharEixos(void)
{
    /* Pequeno deslocamento acima da bancada evita sobreposicao de superficies. */
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1,0,0); glVertex3f(0,0.04f,0); glVertex3f(2,0.04f,0);
    glColor3f(0,0.65f,0); glVertex3f(0,0.04f,0); glVertex3f(0,2,0);
    glColor3f(0,0.3f,1); glVertex3f(0,0.04f,0); glVertex3f(0,0.04f,2);
    glEnd();
    glEnable(GL_LIGHTING);
}

/* Referencial da palma: dedos crescem para +Y, abertura ao longo de X.
 * O centro de cada dedo fica a (abertura + espessura) / 2 da origem.
 * Assim abertura mede o vao real, e nao a distancia entre centros. */
static void DesenharGarra(void)
{
    float afastamento = (braco.abertura + ESPESSURA_DEDO) / 2;
    float centroY = ALTURA_PALMA / 2 + COMPRIMENTO_DEDO / 2;
    Caixa(0, 0, 0, ABERTURA_MAXIMA + 2 * ESPESSURA_DEDO,
          ALTURA_PALMA, 0.36f, 0.28f, 0.31f, 0.36f);
    Caixa(-afastamento, centroY, 0,
          ESPESSURA_DEDO, COMPRIMENTO_DEDO, 0.30f, 0.72f, 0.76f, 0.81f);
    Caixa(afastamento, centroY, 0,
          ESPESSURA_DEDO, COMPRIMENTO_DEDO, 0.30f, 0.72f, 0.76f, 0.81f);
}

static void DesenharBraco(void)
{
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -1.5f);

    /* Pedestal fixo: topo em ALTURA_BASE. */
    Caixa(0, ALTURA_BASE / 2, 0, 1.2f, ALTURA_BASE, 1.2f,
          0.22f, 0.25f, 0.30f);
    glTranslatef(0, ALTURA_BASE, 0);
    glRotatef(braco.base, 0, 1, 0);
    /* Marca a orientacao da base, mesmo com o braco vertical. */
    Caixa(0.42f, 0.06f, 0, 0.3f, 0.12f, 0.25f, 0.95f, 0.65f, 0.12f);

    /* Suporte acompanha a base. Eleva o pivo para o cubo da junta nao
     * atravessar o pedestal ao inclinar (meia diagonal menor que 0.5). */
    Caixa(0, ELEVACAO_OMBRO / 2, 0, 0.4f, ELEVACAO_OMBRO, 0.4f,
          0.22f, 0.25f, 0.30f);
    glTranslatef(0, ELEVACAO_OMBRO, 0);
    /* Pivo do ombro na origem local; eixo Z acompanha a base. */
    glRotatef(-braco.ombro, 0, 0, 1);
    Caixa(0, 0, 0, 0.65f, 0.65f, 0.65f, 0.35f, 0.38f, 0.43f);
    Caixa(0, COMPRIMENTO_BRACO / 2, 0,
          0.38f, COMPRIMENTO_BRACO, 0.38f, 0.90f, 0.46f, 0.12f);

    /* Translada no referencial do braco, sem herdar a escala visual. */
    glTranslatef(0, COMPRIMENTO_BRACO, 0);
    Caixa(0, 0, 0, 0.5f, 0.5f, 0.5f, 0.25f, 0.48f, 0.70f);

    /* Rotacao relativa: apenas antebraco e futuros descendentes giram.
     * Inclinacao total no plano = ombro + cotovelo. */
    glRotatef(-braco.cotovelo, 0, 0, 1);
    Caixa(0, COMPRIMENTO_ANTEBRACO / 2, 0,
          0.30f, COMPRIMENTO_ANTEBRACO, 0.30f, 0.95f, 0.65f, 0.16f);
    glTranslatef(0, COMPRIMENTO_ANTEBRACO, 0);
    /* Junta do punho no fim do antebraco. */
    Caixa(0, 0, 0, 0.4f, 0.4f, 0.4f, 0.25f, 0.48f, 0.70f);
    /* Apenas punho e garra herdam esta rotacao local. */
    glRotatef(-braco.punho, 0, 0, 1);
    Caixa(0, COMPRIMENTO_PUNHO / 2, 0,
          0.24f, COMPRIMENTO_PUNHO, 0.24f, 0.90f, 0.46f, 0.12f);
    glTranslatef(0, COMPRIMENTO_PUNHO + ALTURA_PALMA / 2, 0);
    DesenharGarra();
    glPopMatrix();
}

static void DesenharCena(void)
{
    /* Bancada: espessura 0.3; centro em -0.15 -> topo em Y=0. */
    Caixa(0,-0.15f,0, 10,0.3f,8, 0.58f,0.62f,0.67f);

    /* Blocos apoiados: altura 0.8 -> centro em Y=0.4. */
    Caixa(-2.5f,0.4f,1.4f, 0.8f,0.8f,0.8f, 0.85f,0.25f,0.18f);
    Caixa(-1.2f,0.4f,2.1f, 0.8f,0.8f,0.8f, 0.18f,0.42f,0.85f);
    Caixa(-3.0f,0.4f,-0.4f, 0.8f,0.8f,0.8f, 0.95f,0.66f,0.12f);

    /* Marcador de destino, ligeiramente acima do tampo. */
    Caixa(2.7f,0.012f,0.8f, 2.2f,0.024f,2.2f, 0.20f,0.63f,0.39f);
    DesenharBraco();
    if (mostrarEixos) DesenharEixos();
}

static void Desenhar(void)
{
    const GLfloat posicaoLuz[4] = {3.0f,8.0f,5.0f,1.0f};
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    AplicarCamera();
    /* Definida depois da camera: luz fixa nas coordenadas do mundo. */
    glLightfv(GL_LIGHT0, GL_POSITION, posicaoLuz);
    DesenharCena();
    glutSwapBuffers();
}

static void Redimensionar(int largura, int altura)
{
    if (largura < 1) largura = 1;
    if (altura < 1) altura = 1;
    glViewport(0,0,largura,altura);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)largura / altura, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

static void Teclado(unsigned char tecla, int x, int y)
{
    (void)x; (void)y;
    switch (tecla) {
        case 27: exit(EXIT_SUCCESS);
        case 'r': case 'R': ReiniciarCamera(); break;
        case 'b': case 'B': ReiniciarBraco(); break;
        case 'a': case 'A':
            braco.base = Limitar(braco.base - PASSO_JUNTA, -LIMITE_BASE, LIMITE_BASE); break;
        case 'd': case 'D':
            braco.base = Limitar(braco.base + PASSO_JUNTA, -LIMITE_BASE, LIMITE_BASE); break;
        case 'w': case 'W':
            braco.ombro = Limitar(braco.ombro + PASSO_JUNTA, -LIMITE_OMBRO, LIMITE_OMBRO); break;
        case 's': case 'S':
            braco.ombro = Limitar(braco.ombro - PASSO_JUNTA, -LIMITE_OMBRO, LIMITE_OMBRO); break;
        case 'q': case 'Q':
            braco.cotovelo = Limitar(braco.cotovelo - PASSO_JUNTA, -LIMITE_COTOVELO, LIMITE_COTOVELO); break;
        case 'e': case 'E':
            braco.cotovelo = Limitar(braco.cotovelo + PASSO_JUNTA, -LIMITE_COTOVELO, LIMITE_COTOVELO); break;
        case 'j': case 'J':
            braco.punho = Limitar(braco.punho - PASSO_JUNTA, -LIMITE_PUNHO, LIMITE_PUNHO); break;
        case 'l': case 'L':
            braco.punho = Limitar(braco.punho + PASSO_JUNTA, -LIMITE_PUNHO, LIMITE_PUNHO); break;
        case 'f': case 'F':
            braco.abertura = Limitar(braco.abertura - PASSO_ABERTURA, 0, ABERTURA_MAXIMA); break;
        case 'g': case 'G':
            braco.abertura = Limitar(braco.abertura + PASSO_ABERTURA, 0, ABERTURA_MAXIMA); break;
        case 'x': case 'X': mostrarEixos = !mostrarEixos; break;
        case '+': case '=':
            camera.distancia = Limitar(camera.distancia - 0.5f, 6, 30); break;
        case '-': case '_':
            camera.distancia = Limitar(camera.distancia + 0.5f, 6, 30); break;
        default: break;
    }
    AtualizarTitulo();
    glutPostRedisplay();
}

static void TeclasEspeciais(int tecla, int x, int y)
{
    (void)x; (void)y;
    switch (tecla) {
        case GLUT_KEY_LEFT:  Orbitar(-5,0); break;
        case GLUT_KEY_RIGHT: Orbitar(5,0); break;
        case GLUT_KEY_UP:    Orbitar(0,5); break;
        case GLUT_KEY_DOWN:  Orbitar(0,-5); break;
        default: break;
    }
    glutPostRedisplay();
}

static void Mouse(int botao, int estado, int x, int y)
{
    /* Botao esquerdo reservado para a futura selecao de objetos. */
    if (botao == GLUT_RIGHT_BUTTON) {
        arrastando = (estado == GLUT_DOWN);
        mouseX = x; mouseY = y;
    }
}

static void ArrastarMouse(int x, int y)
{
    if (!arrastando) return;
    Orbitar((x - mouseX) * 0.3f, (mouseY - y) * 0.3f);
    mouseX = x; mouseY = y;
    glutPostRedisplay();
}

static void Inicializar(void)
{
    const GLfloat ambiente[4] = {0.25f,0.25f,0.25f,1};
    const GLfloat difusa[4] = {0.85f,0.85f,0.85f,1};
    const GLfloat especular[4] = {0.25f,0.25f,0.25f,1};
    glClearColor(0.90f,0.93f,0.96f,1);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    /* Corrige o comprimento das normais apos escalas nao uniformes. */
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glLightfv(GL_LIGHT0,GL_AMBIENT,ambiente);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,difusa);
    glLightfv(GL_LIGHT0,GL_SPECULAR,especular);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,especular);
    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,32);
    ReiniciarCamera();
}

int main(int argc, char **argv)
{
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000,700);
    glutCreateWindow("Braco robotico - FK parcial");
    Inicializar();
    AtualizarTitulo();
    glutDisplayFunc(Desenhar);
    glutReshapeFunc(Redimensionar);
    glutKeyboardFunc(Teclado);
    glutSpecialFunc(TeclasEspeciais);
    glutMouseFunc(Mouse);
    glutMotionFunc(ArrastarMouse);
    glutMainLoop();
    return EXIT_SUCCESS;
}
