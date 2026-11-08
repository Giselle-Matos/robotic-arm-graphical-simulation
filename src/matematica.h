#ifndef MATEMATICA_H
#define MATEMATICA_H

#define PI 3.14159265358979323846f

/* Funcao pura, compartilhada pelos controles da camera e do braco. */
static inline float Limitar(float valor, float minimo, float maximo)
{
    if (valor < minimo)
        return minimo;
    if (valor > maximo)
        return maximo;
    return valor;
}

#endif
