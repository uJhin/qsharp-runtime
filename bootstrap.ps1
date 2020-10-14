# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

$ErrorActionPreference = 'Stop'

Push-Location (Join-Path $PSScriptRoot "src/Simulation/CsharpGeneration")
    .\FindNuspecReferences.ps1
Pop-Location

Push-Location (Join-Path $PSScriptRoot "src/Simulation/Simulators")
    .\FindNuspecReferences.ps1
Pop-Location

# install prereqs for native builds
if ($Env:ENABLE_NATIVE -ne "false") {
    if (-not (Test-Path Env:AGENT_OS) -or ($Env:AGENT_OS.StartsWith("Win"))) {
        choco install llvm
        choco install ninja
    } else {
        #brew install llvm # llvm should be already installed
        brew install ninja
    }
} else {
    Write-Host "Skipping installing prerequisites for native because ENABLE_NATIVE variable set to: $Env:ENABLE_NATIVE."
}

# bootstrap native folder
if ($Env:ENABLE_NATIVE -ne "false") {
    ## Run the right script based on the OS.
    if (-not (Test-Path Env:AGENT_OS) -or ($Env:AGENT_OS.StartsWith("Win"))) {
        .\bootstrap.cmd
    } else {
        .\bootstrap.sh
    }
} else {
    Write-Host "Skipping native. ENABLE_NATIVE variable set to: $Env:ENABLE_NATIVE."
}