#include "geometria.h"
#include <GL/glut.h>
#include "matematica.h"
#include <math.h>

/* Cubo unitario centrado na origem, com uma normal por face. */
static void CuboUnitario(void)
{
    static const GLfloat vertices[8][3] = {
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
        {-0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, 0.5f, 0.5f},  {-0.5f, 0.5f, 0.5f}};
    static const int faces[6][4] = {{4, 5, 6, 7}, {1, 0, 3, 2}, {5, 1, 2, 6},
                                    {0, 4, 7, 3}, {7, 6, 2, 3}, {0, 1, 5, 4}};
    static const GLfloat normais[6][3] = {{0, 0, 1},  {0, 0, -1}, {1, 0, 0},
                                          {-1, 0, 0}, {0, 1, 0},  {0, -1, 0}};
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
void Caixa(float x, float y, float z, float largura, float altura, float profundidade, float r,
           float g, float b)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(largura, altura, profundidade);
    glColor3f(r, g, b);
    CuboUnitario();
    glPopMatrix();
}

void CaixaChanfrada(float x, float y, float z, float largura, float altura, float profundidade,
                    float chanfro, float r, float g, float b)
{
    const float metade[3] = {largura / 2, altura / 2, profundidade / 2};
    float corte = fminf(chanfro, 0.9f * fminf(metade[0], fminf(metade[1], metade[2])));
    const int sinais[4][2] = {{-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
    glPushMatrix();
    glTranslatef(x, y, z);
    glColor3f(r, g, b);

    /* Faces principais: a normal coincide com um eixo local. */
    glBegin(GL_QUADS);
    for (int eixo = 0; eixo < 3; ++eixo)
        for (int lado = -1; lado <= 1; lado += 2) {
            float normal[3] = {0, 0, 0};
            normal[eixo] = (float)lado;
            glNormal3fv(normal);
            for (int vertice = 0; vertice < 4; ++vertice) {
                float ponto[3] = {0, 0, 0};
                ponto[eixo] = lado * metade[eixo];
                ponto[(eixo + 1) % 3] = sinais[vertice][0] * (metade[(eixo + 1) % 3] - corte);
                ponto[(eixo + 2) % 3] = sinais[vertice][1] * (metade[(eixo + 2) % 3] - corte);
                glVertex3fv(ponto);
            }
        }
    /* Cada aresta une duas faces. A normal e a bissetriz de seus eixos. */
    for (int eixo = 0; eixo < 3; ++eixo) {
        int a = (eixo + 1) % 3, b = (eixo + 2) % 3;
        for (int sa = -1; sa <= 1; sa += 2)
            for (int sb = -1; sb <= 1; sb += 2) {
                float normal[3] = {0, 0, 0};
                normal[a] = sa * 0.70710678f;
                normal[b] = sb * 0.70710678f;
                glNormal3fv(normal);
                for (int vertice = 0; vertice < 4; ++vertice) {
                    float ponto[3] = {0, 0, 0};
                    int extremidade = (vertice == 0 || vertice == 3) ? -1 : 1;
                    int face = vertice < 2;
                    ponto[eixo] = extremidade * (metade[eixo] - corte);
                    ponto[a] = sa * (metade[a] - (face ? 0 : corte));
                    ponto[b] = sb * (metade[b] - (face ? corte : 0));
                    glVertex3fv(ponto);
                }
            }
    }
    glEnd();
    glBegin(GL_TRIANGLES);
    for (int sx = -1; sx <= 1; sx += 2)
        for (int sy = -1; sy <= 1; sy += 2)
            for (int sz = -1; sz <= 1; sz += 2) {
                glNormal3f(sx * 0.57735027f, sy * 0.57735027f, sz * 0.57735027f);
                glVertex3f(sx * metade[0], sy * (metade[1] - corte), sz * (metade[2] - corte));
                glVertex3f(sx * (metade[0] - corte), sy * metade[1], sz * (metade[2] - corte));
                glVertex3f(sx * (metade[0] - corte), sy * (metade[1] - corte), sz * metade[2]);
            }
    glEnd();
    glPopMatrix();
}

void CilindroZ(float raio, float comprimento, int lados, float r, float g, float b)
{
    glColor3f(r, g, b);
    for (int lado = -1; lado <= 1; lado += 2) {
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, (float)lado);
        glVertex3f(0, 0, lado * comprimento / 2);
        for (int i = 0; i <= lados; ++i) {
            float angulo = lado * 2 * PI * i / lados;
            glVertex3f(raio * cosf(angulo), raio * sinf(angulo), lado * comprimento / 2);
        }
        glEnd();
    }
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= lados; ++i) {
        float angulo = 2 * PI * i / lados, nx = cosf(angulo), ny = sinf(angulo);
        glNormal3f(nx, ny, 0);
        glVertex3f(raio * nx, raio * ny, -comprimento / 2);
        glVertex3f(raio * nx, raio * ny, comprimento / 2);
    }
    glEnd();
}
