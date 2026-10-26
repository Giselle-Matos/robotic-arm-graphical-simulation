# Simulador de braco robotico

Trabalho de Computacao Grafica em C, OpenGL classico e FreeGLUT.
Estado: FK parcial com base, ombro e braco superior; sem selecao, captura ou colisao.

## Compilar e executar no PowerShell

Ambiente encontrado e utilizado: GCC 16.2.0 e FreeGLUT x64 do MSYS2 UCRT64
em C:\msys64\ucrt64. O GCC TDM no PATH nao encontra GL/glut.h.
O script usa o compilador UCRT64 e ajusta o PATH apenas durante sua execucao,
restaurando-o ao terminar. Nao exige reinstalar dependencias.

```powershell
.\compilar.ps1
.\build\robotic_arm.exe
# Ou compilar e abrir:
.\compilar.ps1 -Executar
```

Comando de compilacao usado pelo script, com UCRT64 no PATH do processo:

```powershell
& 'C:\msys64\ucrt64\bin\gcc.exe' -m64 main.c -o build\robotic_arm.exe -Wall -Wextra -lfreeglut -lopengl32 -lglu32 -lm
```

O script cria build e copia libfreeglut.dll de C:\msys64\ucrt64\bin para
junto do novo executavel. O executavel antigo da raiz e preservado.
Dependencias continuam na instalacao existente; nao e um pacote portavel.
Nao usar as bibliotecas GLUT lib32 do TDM nesta compilacao de 64 bits.

## Controles e teste manual

- Setas ou botao direito + arrastar: orbitar; testar elevacao de 10 a 80 graus.
- + / -: zoom; testar limites. R: restaurar camera.
- X: mostrar/ocultar eixos. Esc: sair.
- Redimensionar: conferir proporcoes e profundidade.
- Conferir tres blocos apoiados, bancada e destino verde sem cintilacao.
- Orbitar para conferir oclusao e iluminacao fixa no mundo.

Convencoes: Y para cima; bancada no plano XZ com topo Y=0; unidades arbitrarias.
Blocos de lado 0,8 centrados em Y=0,4. X vermelho, Y verde, Z azul.
A projecao e definida em Redimensionar e a camera em AplicarCamera.
A luz e posicionada depois da camera para permanecer fixa no mundo.
GL_NORMALIZE corrige as normais apos escalas; Caixa restaura a matriz.

Compilacao e link passaram sem avisos; validacao visual ainda pendente.
Veja PLANEJAMENTO.md.

## FK parcial - etapa 2a

- A/D: diminuir/aumentar giro da base (-180 a 180 graus).
- W/S: aumentar/diminuir inclinacao do ombro (-70 a 70 graus).
- B: restaurar base e ombro; R continua exclusivo da camera.

Angulos no titulo da janela. Movimento discreto em passos de 5 graus;
animacao temporal sera introduzida nas proximas partes da etapa 2.
Base em (0,0,-1.5), altura 0.6; braco superior de comprimento 2.6.
Ombro zero aponta para cima; angulo positivo inclina para +X local.

Teste: girar base com ombro inclinado; o segmento e o marcador azul devem
acompanhar juntos. Alterar ombro: pedestal permanece fixo, marcador descreve
um arco ao redor do ombro. Atingir ambos os limites e testar B e R separadamente.
A cena inicial foi validada pelo usuario; esta nova geometria aguarda teste visual.

## FK parcial - etapa 2b

Q/E diminuem/aumentam o cotovelo em passos de 5 graus (-135 a 135).
B restaura tambem o cotovelo para 60 graus. Zero alinha os dois segmentos;
o angulo e relativo ao braco superior, com o mesmo sentido do ombro.
Antebraco de comprimento 2.0; ombro elevado 0.5 acima do pedestal para
reduzir interpenetracao visual da junta. Os eixos agora ficam em Y=0.04,
acima do marcador de destino (topo Y=0.024), evitando contato entre superficies.

Teste: com B, variar apenas Q/E: braco superior permanece parado e a ponta
move-se em arco centrado no cotovelo. Com cotovelo dobrado, variar W/S e A/D:
ambos os segmentos acompanham. Testar limites, B, R e cotovelo zero.
Ainda nao ha colisao: algumas combinacoes podem atravessar bancada, base
ou blocos. Limites individuais nao garantem ausencia de colisao da cadeia.
O enquadramento inicial pode cortar poses estendidas: use - para afastar.

## FK parcial - etapa 2c

- J/L: diminuir/aumentar inclinacao do punho (-90 a 90 graus).
- F/G: fechar/abrir os dedos (vao livre de 0 a 1.2, passos de 0.1).
- B: restaurar juntas, incluindo punho em 45 graus e garra aberta.

Punho de comprimento 0.45, palma de espessura 0.18 e dedos de comprimento
0.9 e espessura 0.12. O vao maximo de 1.2 supera o lado 0.8 dos blocos.
Dedos deslizam simetricamente no X local da palma; crescem no Y local.
O punho inclina em Z local, com o mesmo sentido do ombro e cotovelo.
Teste J/L: apenas punho e garra mudam de inclinacao. Teste F/G: palma
permanece fixa e dedos se aproximam/afastam simetricamente, sem se atravessar.
Teste toda a cadeia com A/D, W/S, Q/E e restaure com B.
Fechar a garra ainda nao captura objetos. Sem colisao ou animacao temporal.
