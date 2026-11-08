#include "juntas.h"
#include "braco_config.h"
#include "simulacao_config.h"
#include <math.h>

float ObterAngulo(const Braco *pose, Junta junta)
{
    switch (junta) {
        case JUNTA_BASE:
            return pose->base;
        case JUNTA_OMBRO:
            return pose->ombro;
        case JUNTA_COTOVELO:
            return pose->cotovelo;
        case JUNTA_PUNHO:
            return pose->punho;
        case JUNTA_DESVIO:
            return pose->desvioPunho;
        case JUNTA_GIRO:
            return pose->giroPunho;
        default:
            return 0;
    }
}
void DefinirAngulo(Braco *pose, Junta junta, float graus)
{
    switch (junta) {
        case JUNTA_BASE:
            pose->base = graus;
            break;
        case JUNTA_OMBRO:
            pose->ombro = graus;
            break;
        case JUNTA_COTOVELO:
            pose->cotovelo = graus;
            break;
        case JUNTA_PUNHO:
            pose->punho = graus;
            break;
        case JUNTA_DESVIO:
            pose->desvioPunho = graus;
            break;
        case JUNTA_GIRO:
            pose->giroPunho = graus;
            break;
        default:
            break;
    }
}
float AnguloCircular(float graus)
{
    float valor = fmodf(graus, 360);
    return valor < 0 ? valor + 360 : valor;
}
float AnguloAssinado(float graus)
{
    float valor = AnguloCircular(graus);
    return valor > 180 ? valor - 360 : valor;
}
float DiferencaAngular(float inicio, float fim)
{
    return AnguloAssinado(fim - inicio);
}
void NormalizarPose(Braco *pose)
{
    for (int junta = 0; junta < TOTAL_JUNTAS; ++junta)
        DefinirAngulo(pose, (Junta)junta, AnguloCircular(ObterAngulo(pose, (Junta)junta)));
}
int PoseDentroDosLimites(const Braco *pose)
{
    for (int junta = 0; junta < TOTAL_JUNTAS; ++junta)
        if (!isfinite(ObterAngulo(pose, (Junta)junta)))
            return 0;
    return fabsf(AnguloAssinado(pose->ombro)) <= IK_LIMITE_OMBRO + 0.001f &&
           fabsf(AnguloAssinado(pose->cotovelo)) <= IK_LIMITE_COTOVELO + 0.001f &&
           fabsf(AnguloAssinado(pose->punho)) <= IK_LIMITE_PUNHO + 0.001f &&
           fabsf(AnguloAssinado(pose->desvioPunho)) <= IK_LIMITE_DESVIO + 0.001f &&
           isfinite(pose->abertura) && pose->abertura >= 0 && pose->abertura <= ABERTURA_MAXIMA &&
           isfinite(pose->centroGarra) &&
           fabsf(pose->centroGarra) <= CAPTURA_DESLOCAMENTO_MAXIMO + 0.001f;
}
Braco InterpolarPose(const Braco *inicio, const Braco *fim, float t)
{
    Braco resultado = *inicio;
    for (int junta = 0; junta < TOTAL_JUNTAS; ++junta) {
        float a = ObterAngulo(inicio, (Junta)junta), b = ObterAngulo(fim, (Junta)junta);
        DefinirAngulo(&resultado, (Junta)junta, a + t * (b - a));
    }
    resultado.abertura = inicio->abertura + t * (fim->abertura - inicio->abertura);
    resultado.centroGarra = inicio->centroGarra + t * (fim->centroGarra - inicio->centroGarra);
    return resultado;
}
Braco DesenrolarPose(const Braco *inicio, const Braco *fim)
{
    Braco resultado = *fim;
    for (int junta = 0; junta < TOTAL_JUNTAS; ++junta) {
        float a = ObterAngulo(inicio, (Junta)junta);
        DefinirAngulo(&resultado, (Junta)junta,
                      a + DiferencaAngular(a, ObterAngulo(fim, (Junta)junta)));
    }
    return resultado;
}

int TrajetoDentroDosLimites(const Braco *inicio, const Braco *fim)
{
    if (!PoseDentroDosLimites(inicio) || !PoseDentroDosLimites(fim))
        return 0;
    float maior = 0;
    for (int j = 0; j < TOTAL_JUNTAS; ++j)
        maior = fmaxf(maior, fabsf(ObterAngulo(fim, (Junta)j) - ObterAngulo(inicio, (Junta)j)));
    /* O programa valida pequenos movimentos ou equivalentes proximos.
     * Nao aceita comandos externos com milhares de voltas em uma chamada. */
    if (maior > 3600)
        return 0;
    int amostras = (int)ceilf(maior / 0.5f) + 1;
    for (int i = 0; i <= amostras; ++i) {
        Braco pose = InterpolarPose(inicio, fim, (float)i / amostras);
        if (!PoseDentroDosLimites(&pose))
            return 0;
    }
    return 1;
}
