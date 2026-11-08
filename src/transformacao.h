#ifndef TRANSFORMACAO_H
#define TRANSFORMACAO_H

/* Matriz 4x4 coluna-principal, compativel com glMultMatrixf. Sem dependencia GL.
 * v[4*coluna+linha]; colunas 0..2 = eixos locais, v[12..14] = posicao.
 * Matrizes de referenciais nunca incluem escala: tamanho pertence a Peca. */
typedef struct {
    float v[16];
} Matriz;
Matriz Identidade(void);
Matriz MultiplicarMatrizes(Matriz a, Matriz b);
/* Inversa somente de transformacao rigida (rotacao ortonormal + translacao). */
Matriz InversaRigida(Matriz m);
/* Ambas pos-multiplicam: os deslocamentos/eixos sao locais a m. */
void Transladar(Matriz *m, float x, float y, float z);
/* Eixo unitario. Pos-multiplica no referencial local (formula de Rodrigues). */
void Rotacionar(Matriz *m, float graus, float x, float y, float z);

#endif
