#ifndef JUNTAS_H
#define JUNTAS_H
#include "braco.h"

typedef enum {
    JUNTA_BASE,
    JUNTA_OMBRO,
    JUNTA_COTOVELO,
    JUNTA_PUNHO,
    JUNTA_DESVIO,
    JUNTA_GIRO,
    TOTAL_JUNTAS
} Junta;

float ObterAngulo(const Braco *pose, Junta junta);
void DefinirAngulo(Braco *pose, Junta junta, float graus);
float AnguloCircular(float graus);
float AnguloAssinado(float graus);
/* Menor diferenca em graus: 359 -> 1 resulta em +2, nao -358. */
float DiferencaAngular(float inicio, float fim);
void NormalizarPose(Braco *pose);
int PoseDentroDosLimites(const Braco *pose);
int TrajetoDentroDosLimites(const Braco *inicio, const Braco *fim);
/* Interpolacao linear, t em [0,1]; use DesenrolarPose antes de cruzar 0/360. */
Braco InterpolarPose(const Braco *inicio, const Braco *fim, float t);
/* Escolhe os equivalentes de fim proximos de inicio, antes de interpolar. */
Braco DesenrolarPose(const Braco *inicio, const Braco *fim);
#endif
