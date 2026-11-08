# Simulador de braco robotico

Trabalho de Computação Gráfica: C11, OpenGL clássico, GLU e FreeGLUT, sem engine.

## Executar a versão atual

Execute a versão completa, não os executáveis antigos da raiz ou de etapas anteriores:

```powershell
.\build\robotic_arm_revisado.exe
```

Fluxo recomendado: **clique no bloco 01 → T → espere o contorno verde → Espaço
→ Y → espere chegar ao destino → Espaço**. Repita para os blocos 02 e 03.
O painel mostra **Destino 3/3 / Tarefa concluída** quando todos estiverem apoiados
inteiramente na região verde. **F5** reinicia e demonstra o fluxo completo dos
três blocos usando os mesmos cálculos, colisões e movimentos do modo manual.
**C** ou uma tecla de junta cancela o movimento automático.

Materiais para a apresentação:

- [Texto dissertativo: funcionamento completo e cinemática inversa](FUNCIONAMENTO_DO_CODIGO.md)
- [Texto de estudo em PDF, com marcadores por assunto](FUNCIONAMENTO_DO_CODIGO.pdf)
- [Roteiro e alterações ao vivo](APRESENTACAO.md)
- [Slides em PDF](APRESENTACAO.pdf)
- [Validações executadas e checklist](VALIDACAO.md)
- [Planejamento e estado final](PLANEJAMENTO.md)

## Compilar e testar

Ambiente existente: MSYS2 UCRT64 em `C:\msys64\ucrt64`, GCC e FreeGLUT x64.
Os scripts usam esse compilador explicitamente e restauram o PATH ao terminar.

```powershell
.\compilar.ps1 -NomeExecutavel robotic_arm_revisado -Executar
.\testar.ps1
# Opcional: teste real de renderização/controles, em janela oculta:
.\testar_interface.ps1
```

Se o executável estiver aberto, feche a janela antes de recompilá-lo ou escolha
outro `-NomeExecutavel`. A DLL idêntica já instalada não é sobrescrita.
Para outra instalação, todos os scripts aceitam `-DiretorioUcrt 'D:\msys64\ucrt64'`.
Fontes e saída são resolvidos pela pasta do script; podem ser chamados por caminho
absoluto. C11 x64 com `-Wall -Wextra -Wpedantic`; testes também usam `-Werror`.
Não basta compilar apenas main.c. O diretório build é ignorado pelo Git.
`fontes.ps1` centraliza as listas usadas pelos três scripts, separando lógica
e desenho. O estilo de C está registrado em `.clang-format`; o formatador não
é necessário para compilar ou executar.

## Revisão de legibilidade

Matrizes e referencial da garra têm uma única implementação compartilhada.
A seleção usa a interseção de raio do módulo de colisões; o mouse separa
conversão de coordenadas e intenção do usuário. A IK separa a solução do punho
da posição dos elos, e o movimento separa execução de trechos e demonstração.
Peças são identificadas pelo enum, sem grupos ou contadores redundantes.
Parâmetros de geometria e amostragem estão nos cabeçalhos de configuração.

Comportamento e visual preservados: testes completos aprovados e sete capturas
OpenGL idênticas às da versão anterior no mesmo ambiente. O texto de estudo
explica a matemática, o fluxo entre módulos e como alterar o código.

## Acabamento visual (08/11/2026)

Visual minimalista em **grafite, verde e cobre suave**, centralizado em
`src/visual_config.h`. Plataforma lisa com uma borda discreta, blocos com chanfros
e uma única identificação no topo, destino sem marcações decorativas e fundo
com degradê sutil. O braço e a garra usam superfícies contínuas e acabamento
acetinado, mantendo apenas trilhos, cursores e apoios essenciais ao mecanismo.

O painel usa tipografia Helvetica, atalhos alinhados em duas colunas e uma
barra de progresso discreta. Todos os controles continuam disponíveis. Os eixos
começam ocultos; X os mostra quando necessário. Cobre aparece somente em pequenos
destaques e no bloco 01. Veja [a garra em detalhe](docs/08_garra.png).

Dimensões, captura, IK e colisões permanecem iguais. As simplificações são visuais.

## Controles

| Entrada | Ação |
| --- | --- |
| Clique esquerdo | Selecionar primeiro bloco visível; clique vazio desfaz seleção |
| T | IK aproxima a garra do bloco selecionado |
| Espaço | Pegar ou soltar sobre um apoio válido |
| Y | IK transporta a carga até uma vaga livre no destino |
| Ctrl + clique na cena | Alvo IK: X/Z no plano da bancada |
| PgUp / PgDn | Altura Y desse alvo, mostrada no painel |
| F5 | Reiniciar e demonstrar a entrega dos três blocos |
| C ou tecla de junta | Cancelar movimento automático |
| A / D | Base: esquerda / direita |
| W / S | Ombro: levantar / baixar |
| Q / E | Cotovelo: levantar / baixar antebraço |
| J / L | Punho: levantar / baixar a pinça |
| I / K | Inclinar a pinça para frente / trás |
| O / P | Giro anti-horário / horário, visto da ponta para a palma |
| F / G | Fechar / abrir manualmente a pinça livre |
| V | Alternar limites mecânicos das juntas em FK |
| B | Reiniciar braço, blocos, seleção, carga e tarefa |
| Setas / botão direito + arrastar | Orbitar a câmera |
| + ou = / − ou _ | Zoom |
| R / X | Restaurar câmera / alternar eixos |
| H / X no painel | Fechar ou abrir a ajuda lateral |
| Esc | Encerrar |

Segure as teclas de junta; solte para parar. Repetição do teclado é ignorada.
Maiúsculas e minúsculas têm o mesmo sentido em todos os controles. Shift+O
agora faz a mesma ação de o. F/G não pegam nem soltam; a carga continua presa sem manter teclas pressionadas. Câmera pode
ser movimentada durante a animação. Perder foco pausa a integração no Windows
sem recuperar o tempo passado; controles mantidos são liberados.

Cada tecla gira uma junta no referencial local do braço. Orbitar a câmera muda
a aparência desse giro. T e Y permitem o posicionamento automático.

## Estados e pega assistida

Contorno claro: selecionado; verde-claro: captura permitida; cobre: transportando.
Seleção pelo mouse e captura são etapas distintas. Picking usa raio/OBB e
considera oclusão por braço/bancada. Orbite a câmera se outro objeto ocultar o alvo.

Não é necessário fechar perfeitamente nem centralizar o bloco com precisão.
A região de captura exige sobreposição útil longitudinal/de profundidade,
abertura realizável e ausência de contato proibido com palma, braço e obstáculos.
Os dedos podem deslizar lateralmente até 0,30 unidade, com ajuste gradual da
abertura e do cursor; o objeto não é teleportado nem reposicionado ao agarrar.
Os parâmetros estão em `src/simulacao_config.h`.

A relação é calculada uma vez: `relativa = inversa(garra) * mundoBloco`.
Cada pose aplica `mundoBloco = garraAtual * relativa`. O contato com os dois dedos
é intencional. A carga continua colidindo com palma, outras partes do braço,
bancada e demais blocos. Uma captura negada preserva completamente o estado.

## Apoio, soltura e destino

Apoio válido exige a pegada inteira sobre a bancada ou sobre a face superior de
outro bloco; apoio parcial, ar, ocupação e atravessamento são rejeitados.
A tolerância padrão de altura é 0,12 unidade e a inclinação admitida para um
pequeno assentamento é 15°. A correção é testada em 64 amostras antes de confirmar.
Se os dedos ainda interceptarem a pose assentada, abre antes de remover o vínculo.

O destino é uma região decorativa sobre a bancada, sem espessura física adicional.
Só conta um bloco solto, apoiado e inteiramente dentro da área verde, com margem.
Pegar novamente um bloco remove sua contagem. Y escolhe uma vaga livre da grade;
a tarefa termina quando os três blocos estão entregues. Não há queda física,
força de atrito ou simulação de tombamento; essas simplificações são explícitas.

## IK e planejamento

O alvo é um referencial completo (posição + orientação) da garra. O solver puro,
sem estado e sem OpenGL, faz:

1. Subtrair o deslocamento garra–punho ao longo do Y local.
2. Resolver base e dois segmentos planares pela lei dos cossenos.
3. Testar até quatro ramos (cotovelo positivo/negativo e base oposta).
4. Decompor a orientação restante em rotações Z–X–Y do punho.
5. Rejeitar limites e tratar polos/alvos inválidos ou fora do alcance.

Os limites IK sempre são respeitados. Mantendo a preferência anterior do usuário,
FK inicia circular; **V** ativa os limites para a apresentação. Base/giro são juntas
rotativas contínuas; ombro/cotovelo/punho/desvio têm intervalos configuráveis.
Para iniciar com limites FK, altere `LIMITES_FK_INICIAIS` para 1. Não é possível
ativá-los em uma pose já fora dos limites: use B antes. O PDF do professor exige
limites por articulação; apresente o modo limitado e explique as juntas contínuas.

O planejador tenta corredores de altura, retirando a carga para cima, recolhendo
o braço, girando e descendo até o alvo. As soluções próximas da pose anterior são
priorizadas. Cada trecho é verificado antes da execução e novamente a cada passo.
Abrir a pinça perto de outra vaga ocupada pode colidir: por isso, levanta antes de
abrir completamente. Um alvo inalcançável, limite ou caminho bloqueado nunca causa
teletransporte. O painel distingue esses casos.

Esse planejamento é deliberadamente pequeno e legível: não é uma busca completa
no espaço de configurações. Pode negar um caminho que exigiria outra manobra;
nesses casos use C, reposicione em FK e tente novamente. O mouse fornece posição
com orientação vertical; T/Y usam os referenciais adequados à pega e colocação.

## Arquitetura e matemática

| Arquivo em src | Responsabilidade |
| --- | --- |
| aplicacao | Eventos, câmera/projeção, relógio, callbacks e mouse → raio |
| braco | Pose privada; única escrita externa por AplicarPoseValidada |
| braco_config / simulacao_config | Dimensões e parâmetros de comportamento |
| juntas | Ângulos, equivalentes circulares, limites e interpolação |
| cinematica / transformacao | FK, matrizes globais e álgebra de referenciais |
| ik | Solver analítico puro para posição e orientação |
| movimento | Planejamento, execução temporal e máquina de estados da demonstração |
| manipulacao | Seleção, estado da carga e vínculo relativo |
| apoio | Validação de apoio/assentamento, destino e vagas |
| aabb / colisao | Árvore BVH de AABBs, OBB/SAT e trajetos |
| braco_render / geometria | Desenho sem modificar a pose |
| cena_dados / cena | Dados compartilhados, blocos mutáveis e desenho/destaque |
| camera / painel | Câmera orbital e instruções recolhíveis |

Y para cima, bancada em XZ com topo Y=0, base em (0,0,−1,5), ombro em Y=1,1.
Matrizes coluna-principal com pós-multiplicação: mover ao pivô, girar localmente.
Base gira em Y; ombro/cotovelo/punho usam −Z, desvio X e giro Y. Desenho e colisão
usam as mesmas matrizes/dimensões. Normais por face, GL_NORMALIZE, luz fixa no
mundo, profundidade, perspectiva e double buffering. Sem sombras projetadas.

Relógio com passo fixo de 1/240 s; juntas 45°/s e abertura/cursor 0,6 unidade/s.
Pausas de interface acima de 100 ms descartam o excedente para evitar saltos.
A árvore AABB poda pares distantes e o SAT testa 15 eixos das caixas orientadas.
Amostragem adaptativa limita soma angular a 0,25° e abertura/cursor a 0,005 por
amostra. Não é CCD matemática. Encaixes de pivôs são exceções explícitas; pares de
segmentos vizinhos usam núcleos reduzidos apenas para permitir a região da junta.
Arestas e extremidades completas continuam sendo usadas contra a cena e a carga.

## Evidências e limites de validação

`testar.ps1`: controles isolados, 1.000 poses AABB × busca exaustiva, matrizes/FK,
pega deslocada do bloco 01, apoio e ocupação, 400 FK→IK com orientação, polos,
alvos inválidos/inalcançáveis, limites, cancelamento e entrega dos três blocos.
Cada passo da tarefa verifica pose válida e ausência de colisão.

`testar_interface.ps1`: OpenGL real em janela oculta, mouse/projeção, T, Espaço,
Y, F5, contagem 3/3, cancelamento, limites e painel compacto. Gera PPM em build;
as renderizações usadas nos slides estão em docs. Isso verifica código e desenho;
a experiência com seu teclado/mouse ainda deve ser conferida pelo checklist.



```bash
gcc ".\main.c" -o ".\main.exe" -lfreeglut -lopengl32 -lglu32 -lm
```