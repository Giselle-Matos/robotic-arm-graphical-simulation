#ifndef GEOMETRIA_H
#define GEOMETRIA_H

/* Desenha caixa centrada em (x,y,z). Dimensoes totais, cor RGB entre 0 e 1.
 * Requer MODELVIEW ativo; preserva a matriz de transformacao do chamador. */
void Caixa(float x, float y, float z, float largura, float altura, float profundidade, float r,
           float g, float b);

/* Caixa com 6 faces, 12 chanfros de aresta e 8 cantos triangulares.
 * Permanece inteiramente dentro das mesmas dimensoes totais de Caixa. */
void CaixaChanfrada(float x, float y, float z, float largura, float altura, float profundidade,
                    float chanfro, float r, float g, float b);
/* Cilindro centrado na origem, ao longo de Z, com tampas e normais proprias. */
void CilindroZ(float raio, float comprimento, int lados, float r, float g, float b);

#endif
