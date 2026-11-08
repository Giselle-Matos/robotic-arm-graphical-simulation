# Acompanhamento

Atualizado em 08/11/2026.

## Estado da entrega

| Item solicitado | Estado |
| --- | --- |
| Soltura válida, apoio, ocupação e destino | Implementado e testado |
| Colisões da carga e contatos de pega | Integrados e testados no trajeto completo |
| IK com orientação, alcance, limites e colisões | Implementada; solver + planejamento + execução |
| Selecionar → aproximar → agarrar → transportar → soltar | Teste completo 3/3 aprovado |
| Região de captura do bloco 01 | Ampliada com cursor lateral; regressão de 0,18 unidade aprovada |
| Código legível e parâmetros adaptáveis | Módulos separados, enums e cabeçalhos de configuração |
| Material de apresentação | Slides, roteiro e FUNCIONAMENTO_DO_CODIGO.md/.pdf |
| Validação humana no desktop | Checklist em VALIDACAO.md para o usuário |

## Como usar

Executável atual: build/robotic_arm_revisado.exe.
Clique no bloco, T, Espaço, Y, Espaço. F5 reinicia e executa os três blocos.
Ctrl+clique indica alvo; PgUp/PgDn ajustam altura. C/teclas de junta cancelam IK.
V ativa limites FK para apresentação; a IK sempre respeita limites.

## Estrutura consolidada

- braco_config.h: dimensões, velocidades e passo temporal.
- simulacao_config.h: tolerâncias de captura/apoio, limites e parâmetros IK.
- juntas.c: ângulos, equivalentes, limites e interpolação compartilhados.
- ik.c: solver analítico puro, validação por FK e tratamento de singularidades.
- movimento.c: corredores, validação prévia, execução e demonstração por estados.
- manipulacao.c: seleção, pega explícita e vínculo relativo.
- apoio.c: assentamento testado, apoio completo, destino e vagas.
- colisao.c: pares explícitos de montagem, núcleos nas juntas e carga contra cena.
- aplicacao.c/painel.c: controles e indicação clara dos estados.
- fontes.ps1: lista única de fontes para compilação e verificações.
- .clang-format: estilo de formatação C, sem dependência de execução.

## Verificações

Ver VALIDACAO.md para comandos, resultados, limites e checklist.
Testes incluem 1.000 poses de colisão, 400 FK→IK, pega deslocada do bloco 01,
assentamento e tarefa completa com checagem por passo. Teste OpenGL confirma
cliques/projeção, T/Espaço/Y/F5, 3/3, cancelamento e painel.

## Enunciado e escolhas

PDF trabalho_final_2026-2 (1).pdf lido integralmente. Seleção/destaque, FK,
animação, câmera, iluminação, hierarquia e colisões implementados. IK opcional
do enunciado também implementada. Modo V aplica limites de ombro/cotovelo/punho/
desvio; base e giro são rotativos contínuos. FK inicia livre para preservar a
preferência anterior. Para iniciar limitado, LIMITES_FK_INICIAIS = 1.

## Limitações explícitas

Sem forças/atrito/queda; geometria por caixas; colisão amostrada e encaixes
simplificados; planejamento por corredores não completo. Falhas de alcance,
limite e colisão não alteram a pose validada. Não há pendência funcional nos
quatro itens desta entrega; resta a conferência humana e estudo para apresentar.

## Histórico

26/10: cena, articulações, pinça e refatoração modular.
07/11: movimento temporal, rotações livres, AABB e recuperação dos testes.
Depois: captura/transporte, painel, giro inverso e ajustes de contato.
Testes humanos mostraram precisão excessiva; substituída captura por fechamento
por seleção + Espaço. Entrega atual completa apoio/destino, IK, validação e
material para explicar e modificar a simulação em apresentação.

## Estado visual e revisão de código — 08/11/2026

Tema minimalista em grafite, verde e cobre, blocos numerados e ajuda recolhível.
A/D = esquerda/direita; W/S = levantar/baixar; letras maiúsculas consistentes.

Revisão de todos os fontes C, cabeçalhos, testes e scripts: formatação uniforme,
remoção de duplicações matemáticas e de metadados, etapas menores para mouse,
IK, árvore e animação, parâmetros compartilhados e comentários sobre decisões.
Recursos, velocidades, limites, captura e visual mantidos. As sete capturas
OpenGL são idênticas às anteriores, no mesmo ambiente. O fluxo 3/3 preserva
4.425 passos no teste de demonstração.

Texto dissertativo completo em FUNCIONAMENTO_DO_CODIGO.md e PDF, incluindo
cinemática inversa, relação entre módulos e exemplos de alterações ao vivo.
Gerador: docs/gerar_texto.py. Executável: build/robotic_arm_revisado.exe.
