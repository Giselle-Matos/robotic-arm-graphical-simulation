#include "braco.h"
#include "braco_config.h"
#include "simulacao_config.h"
#include "juntas.h"
#include "matematica.h"
#include "colisao.h"
#include "manipulacao.h"
#include <math.h>
#include <string.h>

static Braco braco;
static int bloqueado;
static int limitesFk = LIMITES_FK_INICIAIS;
static unsigned char pressionadas[256];

void LiberarControlesBraco(void)
{
    memset(pressionadas, 0, sizeof pressionadas);
}
int MovimentoBloqueado(void)
{
    return bloqueado;
}
int LimitesFkAtivos(void)
{
    return limitesFk;
}
Braco ObterEstadoBraco(void)
{
    return braco;
}

void ReiniciarBraco(void)
{
    braco = (Braco){.ombro = 25, .cotovelo = 60, .punho = 45, .abertura = ABERTURA_MAXIMA};
    bloqueado = 0;
    LiberarControlesBraco();
    ReiniciarManipulacao();
    ReiniciarCena();
}

int AlternarLimitesFk(void)
{
    if (!limitesFk && !PoseDentroDosLimites(&braco))
        return 0;
    limitesFk = !limitesFk;
    return 1;
}

int DefinirTeclaBraco(unsigned char tecla, int pressionada)
{
    if (tecla >= 'A' && tecla <= 'Z')
        tecla += 'a' - 'A';
    if (tecla == 0 || !strchr("adwsqejlfgikop", tecla))
        return 0;
    if (pressionada && (tecla == 'f' || tecla == 'g'))
        CancelarAjusteManual();
    pressionadas[tecla] = pressionada != 0;
    return 1;
}

int AplicarPoseValidada(const Braco *proxima, int exigirLimites)
{
    /* A escrita acontece somente depois de validar o percurso inteiro. */
    if ((exigirLimites && !TrajetoDentroDosLimites(&braco, proxima)) ||
        TrajetoColide(&braco, proxima)) {
        bloqueado = 1;
        return 0;
    }
    braco = *proxima;
    NormalizarPose(&braco);
    AtualizarManipulacao(&braco);
    bloqueado = 0;
    return 1;
}

int AlternarSegurar(void)
{
    int sucesso = AlternarCaptura(&braco);
    if (sucesso)
        bloqueado = 0;
    return sucesso;
}

static void AtualizarAbertura(float dt)
{
    Braco proxima = braco;
    float alvo = AberturaAssistida();
    int manual = pressionadas['f'] || pressionadas['g'];
    if (BlocoCapturado() >= 0 || (!manual && alvo >= 0)) {
        proxima.abertura +=
            Limitar(alvo - braco.abertura, -VELOCIDADE_ABERTURA * dt, VELOCIDADE_ABERTURA * dt);
    } else {
        proxima.abertura =
            Limitar(braco.abertura + VELOCIDADE_ABERTURA * dt *
                                         ((int)pressionadas['g'] - (int)pressionadas['f']),
                    0, ABERTURA_MAXIMA);
    }
    proxima.centroGarra += Limitar(CentroAssistido() - braco.centroGarra, -VELOCIDADE_ABERTURA * dt,
                                   VELOCIDADE_ABERTURA * dt);
    if (proxima.abertura != braco.abertura || proxima.centroGarra != braco.centroGarra)
        (void)AplicarPoseValidada(&proxima, limitesFk);
}

static void AtualizarJuntas(float dt)
{
    /* Ordem corresponde ao enum Junta, evitando seis blocos quase iguais. */
    const int sentidos[TOTAL_JUNTAS] = {
        pressionadas['a'] - pressionadas['d'], pressionadas['s'] - pressionadas['w'],
        pressionadas['e'] - pressionadas['q'], pressionadas['l'] - pressionadas['j'],
        pressionadas['k'] - pressionadas['i'], pressionadas['o'] - pressionadas['p']};
    Braco proxima = braco;
    int movendo = 0;
    for (int j = 0; j < TOTAL_JUNTAS; ++j) {
        float delta = VELOCIDADE_JUNTA * dt * sentidos[j];
        DefinirAngulo(&proxima, (Junta)j, ObterAngulo(&braco, (Junta)j) + delta);
        if (delta != 0)
            movendo = 1;
    }
    if (!movendo)
        return;
    if (AplicarPoseValidada(&proxima, limitesFk))
        return;

    /* Um componente bloqueado nao paralisa os outros controles. */
    int algumBloqueado = 0;
    for (int j = 0; j < TOTAL_JUNTAS; ++j)
        if (sentidos[j]) {
            proxima = braco;
            DefinirAngulo(&proxima, (Junta)j,
                          ObterAngulo(&braco, (Junta)j) + VELOCIDADE_JUNTA * dt * sentidos[j]);
            if (!AplicarPoseValidada(&proxima, limitesFk))
                algumBloqueado = 1;
        }
    bloqueado = algumBloqueado;
}

void AtualizarBraco(float segundos)
{
    if (!isfinite(segundos) || segundos <= 0)
        return;
    bloqueado = 0;
    while (segundos > 0) {
        float dt = fminf(segundos, PASSO_SIMULACAO);
        AtualizarAbertura(dt);
        int aberturaBloqueada = bloqueado;
        AtualizarJuntas(dt);
        bloqueado = bloqueado || aberturaBloqueada;
        segundos -= dt;
    }
}
