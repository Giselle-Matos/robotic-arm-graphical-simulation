param(
    [switch]$Executar,
    [string]$NomeExecutavel = 'robotic_arm',
    [string]$DiretorioUcrt = 'C:\msys64\ucrt64'
)
$ErrorActionPreference = 'Stop'
# Um nome simples evita saidas acidentais fora da pasta build.
if ($NomeExecutavel -notmatch '^[A-Za-z0-9_-]+$') {
    throw 'NomeExecutavel deve conter apenas letras, numeros, _ ou - (sem extensao).'
}
$compilador = Join-Path $DiretorioUcrt 'bin\gcc.exe'
$dll = Join-Path $DiretorioUcrt 'bin\libfreeglut.dll'
if (!(Test-Path -LiteralPath $compilador) -or !(Test-Path -LiteralPath $dll)) {
    throw "GCC ou FreeGLUT ausentes em $DiretorioUcrt. Informe -DiretorioUcrt."
}
. (Join-Path $PSScriptRoot 'fontes.ps1')
$fontes = @('main.c', 'src/aplicacao.c') + $FontesLogica + $FontesGraficos
$caminhoAnterior = $env:PATH
Push-Location -LiteralPath $PSScriptRoot
try {
    $env:PATH = (Join-Path $DiretorioUcrt 'bin') + ';' + $env:PATH
    New-Item -ItemType Directory -Path build -Force | Out-Null
    $saida = Join-Path 'build' ($NomeExecutavel + '.exe')
    & $compilador -std=c11 -m64 -Wall -Wextra -Wpedantic @fontes -o $saida -lfreeglut -lopengl32 -lglu32 -lm
    if ($LASTEXITCODE -ne 0) {
        throw 'Falha na compilacao. Se o executavel estiver aberto, feche-o ou use -NomeExecutavel outro_nome.'
    }
    # A DLL identica pode estar carregada por outra versao ainda aberta.
    $dllDestino = Join-Path $PSScriptRoot 'build\libfreeglut.dll'
    if (!(Test-Path -LiteralPath $dllDestino) -or
        (Get-FileHash -LiteralPath $dll).Hash -ne (Get-FileHash -LiteralPath $dllDestino).Hash) {
        Copy-Item -LiteralPath $dll -Destination $dllDestino -Force
    }
    Write-Output "Compilado: $saida"
    if ($Executar) { & (Join-Path $PSScriptRoot $saida) }
} finally {
    $env:PATH = $caminhoAnterior
    Pop-Location
}
