#ifndef CAMERA_H
#define CAMERA_H

/* Estes comandos alteram somente o estado da camera orbital. */
void ReiniciarCamera(void);
void Orbitar(float horizontal, float vertical);
void AproximarCamera(float incremento);
/* Requer contexto OpenGL ativo e matriz MODELVIEW selecionada. */
void AplicarCamera(void);

#endif
