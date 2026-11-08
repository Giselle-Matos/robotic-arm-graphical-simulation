# Lista unica compartilhada pelo build e pelas verificacoes.
# A logica funciona sem OpenGL; os testes podem executa-la sem abrir janela.
$FontesLogica = @(
    'src/braco.c', 'src/juntas.c', 'src/cinematica.c', 'src/transformacao.c',
    'src/ik.c', 'src/movimento.c', 'src/colisao.c', 'src/aabb.c',
    'src/cena_dados.c', 'src/manipulacao.c', 'src/apoio.c'
)
$FontesGraficos = @(
    'src/braco_render.c', 'src/camera.c', 'src/cena.c',
    'src/geometria.c', 'src/painel.c'
)
