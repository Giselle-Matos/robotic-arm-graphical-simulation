# Acompanhamento

Atualizado em 26/10/2026.

| Etapa | Estado | Tarefas e criterio de conclusao |
| --- | --- | --- |
| 0 - Preparacao | Concluida | Inspecionar, documentar compilacao reproduzivel e validar execucao. |
| 1 - Cena | Concluida | Validar visualmente cena, camera, luz, profundidade e redimensionamento. |
| 2 - Braco e FK | Em andamento | Hierarquia, pivos, limites, controles, animacao temporal e transformacao global da garra coerentes. |
| 3 - Manipulacao | Pendente | Selecao, destaque, captura geometrica, transporte e soltura sobre apoio valido. |
| 4 - Colisoes e FK completa | Pendente | Bloquear movimentos invalidos, preservar pose valida e testar tarefa completa. |
| 5 - IK | Pendente | Apos FK completa: alvos, limites, degeneracoes e colisoes tratados. |
| 6 - Apresentacao | Pendente | Conferir requisitos, documentar e demonstrar fluxo completo. |

## Verificacoes realizadas

- main.c corresponde a cena inicial; revisadas faces, normais, isolamento
  de escalas, camera, projecao e luz. Nenhuma alteracao no codigo necessaria nesta inspecao.
- Nao encontrados AGENTS.md no projeto ou ancestrais verificados,
  PDF do enunciado ou perspectiva_com_luz.c na pasta da disciplina.
- Git: main.c e README.md sem alteracoes iniciais; robotic_arm.exe
  preexistente nao rastreado, preservado. Consultas com safe.directory
  limitado ao comando, sem modificar configuracao global.
- GCC TDM 10.3.0 x64: verificacao interrompida por GL/glut.h ausente.
- FreeGLUT x64 encontrada em C:\msys64\ucrt64 com cabecalhos, bibliotecas e DLL.
- Verificacao sintatica com cabecalhos explicitos passou sem avisos.
- GCC UCRT64 16.2.0: compilacao e link com -Wall -Wextra passaram sem avisos.
  Necessario adicionar UCRT64 ao PATH do processo para carregar auxiliares do GCC.
- Novo executavel e DLL em build. Executavel antigo da raiz preservado.
- Cena inicial validada visualmente pelo usuario em 26/10/2026 (controles e visualizacao OK).

## Etapa 2a - Base e ombro

Implementados base em (0,0,-1.5), pedestal de altura 0.6, braco de comprimento
2.6 e marcador do futuro cotovelo. Base gira em Y; ombro em Z local.
Limites escolhidos: base -180 a 180 graus; ombro -70 a 70 graus para manter
o segmento acima da bancada. Passos discretos de 5 graus nesta subetapa.
Angulos exibidos no titulo; B restaura braco, R continua restaurando camera.
Escalas isoladas em Caixa; ainda sem animacao temporal nem colisao.

Validacao visual desta nova geometria pendente. Testar limites, independencia
da camera e movimento do marcador azul acompanhando as duas articulacoes.
Proximo passo: antebraco e cotovelo; depois animacao temporal e transformacoes
globais para a logica. Etapa 2 completa continua em andamento.
Enunciado original ainda precisa ser conferido quando disponivel.

Verificacao da etapa 2a: compilacao e link via compilar.ps1 passaram sem avisos (-Wall -Wextra).

## Etapa 2b - Cotovelo e antebraco

Imagem enviada pelo usuario compativel com a etapa 2a: cena, cores e inclinacao
esperadas. Junta do ombro parcialmente embutida no pedestal; elevado seu pivo
em 0.5 com suporte. Pequenas marcas junto ao destino compativeis com eixo
em Y=0.02 atravessando marcador de topo Y=0.024: eixos elevados para Y=0.04.
Bordas serrilhadas permanecem como melhoria visual futura.

Adicionados cotovelo relativo (-135 a 135 graus, Q/E), antebraco de 2.0 e
marcador do futuro punho. Pose inicial: base 0, ombro 25, cotovelo 60.
Sem colisao nesta subetapa; limites permitem interpenetracoes em certas poses.
Validacao visual e testes de hierarquia desta nova geometria pendentes.
Proximo passo: punho e garra; animacao temporal e calculo de transformacoes
globais continuam pendentes para concluir a etapa 2.
Verificacao: executavel padrao bloqueado durante link (possivelmente aberto). Compilacao e link para build/robotic_arm_etapa2b.exe passaram sem avisos com -Wall -Wextra. Validacao visual pendente.

## Etapa 2c - Punho e garra

Etapa 2b validada pelo usuario. Adicionados punho relativo de -90 a 90 graus
(J/L), palma e dois dedos paralelos (F/G). Abertura livre de 0 a 1.2 unidades;
centros dos dedos em +/- (abertura + espessura) / 2. Espessura de 0.12,
comprimento de 0.9, punho de 0.45. Pose inicial do punho: 45 graus.
B restaura todas as juntas e a abertura. Angulos e abertura no titulo.
Validacao visual desta subetapa pendente, conforme testes no README.
Captura e colisoes permanecem nas etapas posteriores.
Proximo passo: animacao temporal e transformacoes globais da garra para a logica.
Etapa 2 ainda em andamento.
Compilacao e link para build/robotic_arm_etapa2c.exe passaram sem avisos (-Wall -Wextra). Usado nome proprio para preservar execucoes anteriores abertas.
