param([string]$DiretorioUcrt = 'C:\msys64\ucrt64')
$ErrorActionPreference = 'Stop'
$compilador = Join-Path $DiretorioUcrt 'bin\gcc.exe'
if (!(Test-Path -LiteralPath $compilador)) { throw "GCC ausente em $DiretorioUcrt." }
$caminhoAnterior = $env:PATH
Push-Location -LiteralPath $PSScriptRoot
try {
    $env:PATH = (Join-Path $DiretorioUcrt 'bin') + ';' + $env:PATH
    New-Item -ItemType Directory -Path build -Force | Out-Null
    . (Join-Path $PSScriptRoot 'fontes.ps1')
    $verificacoes = @(
        # Controles usam colisoes simuladas para isolar o teclado e o tempo.
        @{ Nome = 'braco'; Fontes = @('src/braco.c', 'src/juntas.c') },
        @{ Nome = 'simulacao'; Fontes = $FontesLogica },
        @{ Nome = 'ik'; Fontes = $FontesLogica }
    )
    foreach ($teste in $verificacoes) {
        $nome = $teste.Nome
        $saida = "build/test_$nome.exe"
        $fontes = @("tests/test_$nome.c") + $teste.Fontes
        & $compilador -std=c11 -m64 -Wall -Wextra -Wpedantic -Werror @fontes -o $saida -lm
        if ($LASTEXITCODE -ne 0) { throw "Falha ao compilar testes: $nome." }
        & (Join-Path $PSScriptRoot $saida)
        if ($LASTEXITCODE -ne 0) { throw "Falha nos testes: $nome." }
    }
} finally {
    $env:PATH = $caminhoAnterior
    Pop-Location
}
