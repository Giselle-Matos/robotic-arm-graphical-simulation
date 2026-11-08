#include "transformacao.h"
#include "matematica.h"
#include <math.h>

Matriz Identidade(void)
{
    // clang-format off
    Matriz m = {{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    }};
    // clang-format on
    return m;
}

Matriz MultiplicarMatrizes(Matriz a, Matriz b)
{
    Matriz resultado = {{0}};
    for (int coluna = 0; coluna < 4; ++coluna)
        for (int linha = 0; linha < 4; ++linha)
            for (int k = 0; k < 4; ++k)
                resultado.v[4 * coluna + linha] += a.v[4 * k + linha] * b.v[4 * coluna + k];
    return resultado;
}

Matriz InversaRigida(Matriz m)
{
    Matriz r = Identidade();
    /* R inversa = R transposta; posicao inversa = -R transposta * posicao. */
    for (int coluna = 0; coluna < 3; ++coluna)
        for (int linha = 0; linha < 3; ++linha)
            r.v[4 * coluna + linha] = m.v[4 * linha + coluna];
    for (int linha = 0; linha < 3; ++linha)
        r.v[12 + linha] =
            -(r.v[linha] * m.v[12] + r.v[4 + linha] * m.v[13] + r.v[8 + linha] * m.v[14]);
    return r;
}

void Transladar(Matriz *m, float x, float y, float z)
{
    int i;
    for (i = 0; i < 4; ++i)
        m->v[12 + i] += x * m->v[i] + y * m->v[4 + i] + z * m->v[8 + i];
}

void Rotacionar(Matriz *m, float graus, float x, float y, float z)
{
    float a = graus * PI / 180.0f;
    float c = cosf(a), s = sinf(a), t = 1 - c;
    /* Cada linha do inicializador abaixo armazena uma coluna da matriz. */
    // clang-format off
    Matriz r = {{
        t*x*x+c,   t*x*y+s*z, t*x*z-s*y, 0,
        t*x*y-s*z, t*y*y+c,   t*y*z+s*x, 0,
        t*x*z+s*y, t*y*z-s*x, t*z*z+c,   0,
        0,         0,         0,         1
    }};
    // clang-format on
    *m = MultiplicarMatrizes(*m, r);
}
