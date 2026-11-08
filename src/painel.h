#ifndef PAINEL_H
#define PAINEL_H
int LarguraCena(int larguraJanela);
void AlternarPainel(void);
void DesenharPainel(int largura, int altura);
/* Coordenadas de mouse GLUT; retorna 1 quando o clique pertence a interface. */
int CliquePainel(int x, int y, int largura, int altura);
#endif
