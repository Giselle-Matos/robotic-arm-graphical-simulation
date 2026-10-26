param([switch]$Executar)
$ErrorActionPreference = 'Stop'
$compilador = 'C:\msys64\ucrt64\bin\gcc.exe'
$dll = 'C:\msys64\ucrt64\bin\libfreeglut.dll'
if (!(Test-Path -LiteralPath $compilador) -or !(Test-Path -LiteralPath $dll)) {
    throw 'GCC e FreeGLUT UCRT64 esperados em C:\msys64\ucrt64.'
}
$caminhoAnterior = $env:PATH
Push-Location -LiteralPath $PSScriptRoot
try {
    $env:PATH = 'C:\msys64\ucrt64\bin;' + $env:PATH
    New-Item -ItemType Directory -Path build -Force | Out-Null
    & $compilador -m64 main.c -o build\robotic_arm.exe -Wall -Wextra -lfreeglut -lopengl32 -lglu32 -lm
    if ($LASTEXITCODE -ne 0) { throw 'Falha na compilacao.' }
    Copy-Item -LiteralPath $dll -Destination build\libfreeglut.dll -Force
    if ($Executar) { & .\build\robotic_arm.exe }
} finally {
    $env:PATH = $caminhoAnterior
    Pop-Location
}
