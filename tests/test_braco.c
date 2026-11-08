/* Testa controles/tempo isoladamente com um detector de colisao controlavel.
 * A implementacao real de colisao e verificada em test_simulacao.c. */
#include "../src/braco.h"
#include "../src/braco_config.h"
#include "../src/colisao.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static int rejeitar, chamadas, rejeitarFechamento;
/* Dependencias de cena/manipulacao isoladas nesta suite de controles. */
void ReiniciarCena(void) {}
void ReiniciarManipulacao(void) {}
int BlocoCapturado(void)
{
    return -1;
}
int AlternarCaptura(const Braco *pose)
{
    (void)pose;
    return 0;
}
float AberturaAssistida(void)
{
    return -1;
}
float CentroAssistido(void)
{
    return 0;
}
void CancelarAjusteManual(void) {}
int TentarSoltarBloco(void)
{
    return 0;
}
void AtualizarManipulacao(const Braco *pose)
{
    (void)pose;
}
int TrajetoColide(const Braco *inicio, const Braco *fim)
{
    assert(fabsf(fim->base - inicio->base) <= 0.188f);
    assert(fabsf(fim->giroPunho - inicio->giroPunho) <= 0.188f);
    ++chamadas;
    return rejeitar || (rejeitarFechamento && fim->abertura < inicio->abertura);
}
static void Perto(float a, float b)
{
    assert(fabsf(a - b) < 0.02f);
}
static void ConferirPose(Braco a, Braco b)
{
    Perto(a.base, b.base);
    Perto(a.ombro, b.ombro);
    Perto(a.cotovelo, b.cotovelo);
    Perto(a.punho, b.punho);
    Perto(a.desvioPunho, b.desvioPunho);
    Perto(a.giroPunho, b.giroPunho);
    Perto(a.abertura, b.abertura);
}
static float Angulo(Braco b, int i)
{
    switch (i) {
        case 0:
            return b.base;
        case 1:
            return b.ombro;
        case 2:
            return b.cotovelo;
        case 3:
            return b.punho;
        case 4:
            return b.desvioPunho;
        default:
            return b.giroPunho;
    }
}
static void Rodar(unsigned char tecla, float tempo)
{
    assert(DefinirTeclaBraco(tecla, 1));
    AtualizarBraco(tempo);
    assert(DefinirTeclaBraco(tecla, 0));
}
int main(void)
{
    const Braco inicial = {.base = 0,
                           .ombro = 25,
                           .cotovelo = 60,
                           .punho = 45,
                           .desvioPunho = 0,
                           .giroPunho = 0,
                           .abertura = 1.2f};
    const char positivas[] = "aselko", negativas[] = "dwqjip";
    Braco a, copia;
    int i, j;
    if (LimitesFkAtivos())
        assert(AlternarLimitesFk());
    ReiniciarBraco();
    ConferirPose(ObterEstadoBraco(), inicial);
    copia = ObterEstadoBraco();
    copia.base = 123;
    Perto(copia.base, 123);
    ConferirPose(ObterEstadoBraco(), inicial);
    assert(!DefinirTeclaBraco('?', 1));
    assert(!DefinirTeclaBraco(0, 1));
    assert(!DefinirTeclaBraco('b', 1));
    assert(!DefinirTeclaBraco('r', 1));
    for (i = 0; i < 6; ++i) {
        ReiniciarBraco();
        /* Duas voltas e mais 45 graus: atravessa todos os antigos batentes. */
        Rodar((unsigned char)positivas[i], 17);
        Perto(Angulo(ObterEstadoBraco(), i), fmodf(Angulo(inicial, i) + 765, 360));
        Rodar((unsigned char)negativas[i], 17);
        a = ObterEstadoBraco();
        if (Angulo(a, i) > 359.98f)
            Rodar((unsigned char)positivas[i], 0.0001f);
        Perto(Angulo(ObterEstadoBraco(), i), Angulo(inicial, i));
    }
    ReiniciarBraco();
    Rodar('A', 1);
    a = ObterEstadoBraco();
    Perto(a.base, 45);
    ReiniciarBraco();
    DefinirTeclaBraco('a', 1);
    for (i = 0; i < 100; ++i) {
        DefinirTeclaBraco('a', 1);
        AtualizarBraco(0.01f);
    }
    ConferirPose(ObterEstadoBraco(), a); /* repeticao do SO nao altera velocidade */
    DefinirTeclaBraco('a', 0);
    AtualizarBraco(1);
    ConferirPose(ObterEstadoBraco(), a);
    DefinirTeclaBraco('a', 1);
    DefinirTeclaBraco('d', 1);
    AtualizarBraco(1);
    ConferirPose(ObterEstadoBraco(), a);
    LiberarControlesBraco();
    ReiniciarBraco();
    DefinirTeclaBraco('s', 1);
    DefinirTeclaBraco('k', 1);
    AtualizarBraco(1);
    a = ObterEstadoBraco();
    Perto(a.ombro, 70);
    Perto(a.desvioPunho, 45);
    LiberarControlesBraco();
    AtualizarBraco(1);
    ConferirPose(ObterEstadoBraco(), a);
    ReiniciarBraco();
    Rodar('O', 1);
    Perto(ObterEstadoBraco().giroPunho, 45);
    Rodar('P', 2);
    Perto(ObterEstadoBraco().giroPunho, 315);
    Rodar('o', 1);
    assert(fminf(fabsf(ObterEstadoBraco().giroPunho), fabsf(360 - ObterEstadoBraco().giroPunho)) <
           0.02f);
    Rodar('f', 5);
    Perto(ObterEstadoBraco().abertura, 0);
    Rodar('g', 5);
    Perto(ObterEstadoBraco().abertura, ABERTURA_MAXIMA);
    a = ObterEstadoBraco();
    AtualizarBraco(0);
    AtualizarBraco(-1);
    AtualizarBraco(NAN);
    AtualizarBraco(INFINITY);
    ConferirPose(ObterEstadoBraco(), a);
    ReiniciarBraco();
    rejeitarFechamento = 1;
    DefinirTeclaBraco('f', 1);
    DefinirTeclaBraco('a', 1);
    AtualizarBraco(1);
    Perto(ObterEstadoBraco().base, 45);
    Perto(ObterEstadoBraco().abertura, 1.2f);
    assert(MovimentoBloqueado());
    ReiniciarBraco();
    DefinirTeclaBraco('f', 1);
    DefinirTeclaBraco('d', 1);
    AtualizarBraco(1);
    Perto(ObterEstadoBraco().base, 315);
    Perto(ObterEstadoBraco().abertura, 1.2f);
    rejeitarFechamento = 0;
    LiberarControlesBraco();
    a = ObterEstadoBraco();
    rejeitar = 1;
    DefinirTeclaBraco('a', 1);
    j = chamadas;
    AtualizarBraco(1);
    assert(chamadas > j);
    assert(MovimentoBloqueado());
    ConferirPose(ObterEstadoBraco(), a);
    ReiniciarBraco();
    assert(!MovimentoBloqueado());
    ConferirPose(ObterEstadoBraco(), inicial);
    rejeitar = 0;
    AtualizarBraco(1);
    ConferirPose(ObterEstadoBraco(), inicial);
    assert(AlternarLimitesFk());
    assert(LimitesFkAtivos());
    Rodar('k', 4);
    Perto(ObterEstadoBraco().desvioPunho, 90);
    assert(MovimentoBloqueado());
    Rodar('i', 8);
    Perto(ObterEstadoBraco().desvioPunho, 270);
    Rodar('o', 17);
    Perto(ObterEstadoBraco().giroPunho, 45);
    ReiniciarBraco();
    assert(AlternarLimitesFk());
    puts("OK: controles, seis rotacoes livres, tempo, repeticao, soltura, rejeicao e reset.");
    return 0;
}
