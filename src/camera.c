#include "camera.h"
#include "matematica.h"
#include <GL/glut.h>
#include <math.h>

typedef struct {
    float azimute, elevacao, distancia;
    float alvoX, alvoY, alvoZ;
} Camera;

/* Estado privado: a aplicacao usa comandos, sem editar campos diretamente. */
static Camera camera;

void AproximarCamera(float incremento)
{
    camera.distancia = Limitar(camera.distancia + incremento, 6.0f, 30.0f);
}

void ReiniciarCamera(void)
{
    camera.azimute = 35.0f;
    camera.elevacao = 30.0f;
    camera.distancia = 15.0f;
    camera.alvoX = 0.0f;
    camera.alvoY = 0.5f;
    camera.alvoZ = 0.0f;
}

void Orbitar(float horizontal, float vertical)
{
    camera.azimute = fmodf(camera.azimute + horizontal, 360.0f);
    /* Evita alinhar a direcao de visao com o vetor vertical. */
    camera.elevacao = Limitar(camera.elevacao + vertical, 10.0f, 80.0f);
}

void AplicarCamera(void)
{
    float a = camera.azimute * PI / 180.0f;
    float e = camera.elevacao * PI / 180.0f;
    float horizontal = camera.distancia * cosf(e);
    gluLookAt(camera.alvoX + horizontal * sinf(a), camera.alvoY + camera.distancia * sinf(e),
              camera.alvoZ + horizontal * cosf(a), camera.alvoX, camera.alvoY, camera.alvoZ, 0.0,
              1.0, 0.0);
}
