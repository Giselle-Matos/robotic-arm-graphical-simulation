#ifndef BRACO_H
#define BRACO_H

/* Angulos em graus, relativos ao pai. Abertura em unidades da cena. */
typedef struct {
    float base;
    float ombro;
    float cotovelo;
    float punho;
    float desvioPunho; /* I/K: rotacao no X local apos inclinacao. */
    float giroPunho;   /* O/P: sentidos opostos no Y local, sem excecao para Shift. */
    float abertura;    /* Vao livre entre as faces internas dos dedos. */
    float centroGarra; /* Cursor lateral dos dedos; assistencia, sem mover o bloco. */
} Braco;

void ReiniciarBraco(void);
/* Retorna copia do estado; leitura nao permite alterar as juntas internas. */
Braco ObterEstadoBraco(void);
/* Registra tecla pressionada/solta. Repeticoes sao idempotentes.
 * Maiusculas e minusculas sao aliases para todos os controles. */
int DefinirTeclaBraco(unsigned char tecla, int pressionada);
void LiberarControlesBraco(void);
/* Integra segundos em subpassos; angulos armazenados em [0,360).
 * FK inicia circular; V ativa os limites de simulacao_config.h.
 * Fechamento e juntas sao validados separadamente. Se o movimento conjunto
 * colide, tenta eixos individuais, sempre preservando poses validas. */
void AtualizarBraco(float segundos);
int AlternarSegurar(void); /* Espaco: pega/solta o selecionado. */
/* Unico ponto de escrita da pose externa: valida limites e TODO o trajeto. */
int AplicarPoseValidada(const Braco *pose, int exigirLimites);
int LimitesFkAtivos(void);
int AlternarLimitesFk(void);
/* 1 se alguma componente foi rejeitada por colisao/limite; reset limpa aviso. */
int MovimentoBloqueado(void);

#endif
