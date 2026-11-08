#ifndef SIMULACAO_CONFIG_H
#define SIMULACAO_CONFIG_H

/* Parametros de comportamento para modificar durante a apresentacao.
 * Dimensoes fisicas do braco permanecem em braco_config.h. Unidades da cena. */
#define CAPTURA_CONTATO_MINIMO_Y 0.10f
#define CAPTURA_CONTATO_MINIMO_Z 0.06f
#define CAPTURA_DESLOCAMENTO_MAXIMO 0.30f
#define CAPTURA_FOLGA 0.01f
#define SOLTURA_DISTANCIA_APOIO 0.12f
#define SOLTURA_INCLINACAO_MAXIMA 15.0f
#define DESTINO_MARGEM 0.02f
#define SOLTURA_SUBDIVISOES 64
/* Densidade da verificacao dos trajetos; graus e unidades da cena. */
#define COLISAO_PASSO_ANGULAR 0.25f
#define COLISAO_PASSO_LINEAR 0.005f

#define IK_LIMITE_OMBRO 160.0f
#define IK_LIMITE_COTOVELO 165.0f
#define IK_LIMITE_PUNHO 170.0f
#define IK_LIMITE_DESVIO 90.0f
/* Mantem a preferencia anterior por FK circular. V alterna em execucao.
 * IK sempre respeita os limites acima, independentemente deste parametro. */
#define LIMITES_FK_INICIAIS 0
#define IK_ALTURA_TRANSITO 1.85f
#define IK_RAIO_INTERMEDIARIO 2.4f
#define IK_ALTURA_PEGA 0.25f
#define IK_PASSO_CARTESIANO 0.05f
#define IK_MAXIMO_PONTOS 256
#define IK_ERRO_POSICAO 0.002f
#define IK_ERRO_ORIENTACAO 0.002f
#define IK_ALTURA_MOUSE_INICIAL 0.70f

#endif
