# Set dependency variables here:
$devFolder = Resolve-Path -Path "$PSScriptRoot\..\..\.dev"
Write-Host $devFolder
Set-Location "$devFolder\ThirdParty"
$Env:THIRD_PARTY_EIGEN = (Resolve-Path -Path "./eigen")
$Env:THIRD_PARTY_FMT = (Resolve-Path -Path "./fmt")

$Env:ADSK_MAYA_SDK_2022 = "D:\Program Files\Autodesk\Maya2022\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2023 = "D:\Program Files\Autodesk\Maya2023\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2024 = "D:\Program Files\Autodesk\Maya2024\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2025 = "D:\Program Files\Autodesk\Maya2025\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2026 = "D:\Program Files\Autodesk\Maya2026\devkit\devkitBase"

$installedVersions = pyenv versions --bare
foreach ($version in $installedVersions) {
    Write-Host "-----"
    if ($version -like "*-*") {
        $version = $version.Split("-")[1]
    }
    Write-Host "version: $version"
    pyenv local $version
    $pythonPath = pyenv which python
    Write-Host $pythonPath
    $pythonRootPath = (Get-Item $pythonPath).Directory.FullName
    $majorMinor = $version -replace '(\d+\.\d+).*', '$1' -replace '\.', ''
    Set-Item "Env:PYTHON_$majorMinor" $pythonRootPath
    Write-Host $majorMinor
    $pybindPath = Resolve-Path -Path "$devFolder\venvs\py$majorMinor\.venv\Lib\site-packages\pybind11" -ErrorAction SilentlyContinue
    if ($pybindPath) {
        Write-Host "Setting PyBind11 path: $pybindPath"
        Set-Item "Env:PYBIND11_$majorMinor" $pybindPath
    }
}

Set-Location $PSScriptRoot
.\skin_plus_plus.sln
pause
