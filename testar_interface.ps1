param([string]$DiretorioUcrt = 'C:\msys64\ucrt64')
$ErrorActionPreference = 'Stop'
$compilador = Join-Path $DiretorioUcrt 'bin\gcc.exe'
$caminhoAnterior = $env:PATH
Push-Location -LiteralPath $PSScriptRoot
try {
    $env:PATH = (Join-Path $DiretorioUcrt 'bin') + ';' + $env:PATH
    New-Item -ItemType Directory -Path build -Force | Out-Null
    . (Join-Path $PSScriptRoot 'fontes.ps1')
    # test_interface inclui aplicacao.c para testar os callbacks reais.
    $fontes = @('tests/test_interface.c') + $FontesLogica + $FontesGraficos
    & $compilador -std=c11 -m64 -Wall -Wextra -Wpedantic -Werror @fontes -o build/test_interface.exe -lfreeglut -lopengl32 -lglu32 -lm
    if ($LASTEXITCODE -ne 0) { throw 'Falha no build da verificacao grafica.' }
    & .\build\test_interface.exe
    if ($LASTEXITCODE -ne 0) { throw 'Falha na verificacao grafica.' }
} finally {
    $env:PATH = $caminhoAnterior
    Pop-Location
}
