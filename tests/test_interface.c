/* Regressao grafica opcional. Inclui a aplicacao para exercitar os callbacks
 * privados sem expor funcoes de interface ao restante do programa. */
#include "../src/aplicacao.c"
#include "../src/apoio.h"
#include "../src/colisao.h"
#include <assert.h>

static void SalvarQuadro(const char *nome)
{
    RenderizarQuadro();
    glFinish();
    int l = larguraJanela, a = alturaJanela;
    unsigned char *pixels = malloc((size_t)l * a * 3);
    assert(pixels);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, l, a, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    FILE *arquivo = fopen(nome, "wb");
    assert(arquivo);
    fprintf(arquivo, "P6\n%d %d\n255\n", l, a);
    for (int y = a - 1; y >= 0; --y)
        fwrite(pixels + (size_t)y * l * 3, 3, (size_t)l, arquivo);
    fclose(arquivo);
    free(pixels);
    assert(glGetError() == GL_NO_ERROR);
}
static void ClicarBloco(int indiceBloco)
{
    GLdouble modelo[16], projecao[16], x, y, z;
    GLint viewport[4];
    ProjetarCena();
    glLoadIdentity();
    AplicarCamera();
    glGetDoublev(GL_MODELVIEW_MATRIX, modelo);
    glGetDoublev(GL_PROJECTION_MATRIX, projecao);
    glGetIntegerv(GL_VIEWPORT, viewport);
    Peca *p = &blocos[indiceBloco];
    assert(gluProject(p->mundo.v[12], p->mundo.v[13], p->mundo.v[14], modelo, projecao, viewport,
                      &x, &y, &z));
    CliqueNaCena((int)x, alturaJanela - 1 - (int)y, 0);
    assert(BlocoSelecionado() == indiceBloco);
}
static void EsperarMovimento(void)
{
    int passos;
    for (passos = 0; passos < 60000 && MovimentoAutomaticoAtivo(); ++passos) {
        AtualizarMovimento(1.0f / 120);
        Braco pose = ObterEstadoBraco();
        assert(!PoseColide(&pose));
    }
    assert(passos < 60000);
}
int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1000, 700);
    glutCreateWindow("Validacao do simulador");
    glutHideWindow();
    Inicializar();
    Redimensionar(1000, 700);
    ClicarBloco(0);
    SalvarQuadro("build/01_selecao.ppm");
    Teclado('t', 0, 0);
    EsperarMovimento();
    Braco pose = ObterEstadoBraco();
    assert(ProntoParaPegar(&pose));
    SalvarQuadro("build/02_aproximacao.ppm");
    Teclado(' ', 0, 0);
    assert(BlocoCapturado() == 0);
    AtualizarBraco(1);
    SalvarQuadro("build/03_captura.ppm");
    Teclado('y', 0, 0);
    for (int passo = 0; passo < 2000; ++passo) {
        AtualizarMovimento(1.0f / 120);
        if (blocos[0].mundo.v[13] > 1.1f)
            break;
    }
    assert(BlocoCapturado() == 0);
    SalvarQuadro("build/04_transporte.ppm");
    EsperarMovimento();
    Teclado(' ', 0, 0);
    AtualizarBraco(1);
    assert(TotalEntregues() == 1);
    SalvarQuadro("build/05_entrega.ppm");
    TeclasEspeciais(GLUT_KEY_F5, 0, 0);
    EsperarMovimento();
    assert(TotalEntregues() == 3);
    SalvarQuadro("build/06_concluido.ppm");
    Teclado('b', 0, 0);
    assert(TotalEntregues() == 0);
    Teclado('h', 0, 0);
    ClicarBloco(2);
    Teclado('h', 0, 0);
    Redimensionar(640, 480);
    SalvarQuadro("build/07_compacto.ppm");
    Redimensionar(1000, 700);
    /* Alvo por Ctrl+clique usa o plano XZ e a altura ajustada pelo teclado. */
    TeclasEspeciais(GLUT_KEY_PAGE_UP, 0, 0);
    GLdouble modelo[16], projecao[16], x, y, z;
    GLint viewport[4];
    ProjetarCena();
    glLoadIdentity();
    AplicarCamera();
    glGetDoublev(GL_MODELVIEW_MATRIX, modelo);
    glGetDoublev(GL_PROJECTION_MATRIX, projecao);
    glGetIntegerv(GL_VIEWPORT, viewport);
    assert(gluProject(2.3, 0, .8, modelo, projecao, viewport, &x, &y, &z));
    CliqueNaCena((int)x, alturaJanela - 1 - (int)y, GLUT_ACTIVE_CTRL);
    assert(MovimentoAutomaticoAtivo());
    EsperarMovimento();
    pose = ObterEstadoBraco();
    Matriz garra = CalcularReferencialGarra(&pose);
    assert(fabsf(garra.v[13] - AlturaAlvoMouse()) < .003f);
    Teclado('t', 0, 0);
    assert(MovimentoAutomaticoAtivo());
    Teclado('a', 0, 0);
    assert(!MovimentoAutomaticoAtivo());
    SoltarTecla('a', 0, 0);
    Teclado('v', 0, 0);
    assert(LimitesFkAtivos());
    Teclado('v', 0, 0);
    glutDestroyWindow(glutGetWindow());
    puts("OK: mouse/projecao, T, Espaco, Y, entrega, F5 3/3, cancelamento, limites e painel.");
    return 0;
}
