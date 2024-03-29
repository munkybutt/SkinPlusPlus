# Set dependency variables here:
$PSScriptRoot
Set-Location "$PSScriptRoot\..\3rdParty"
$Env:THIRD_PARTY_EIGEN = (Resolve-Path -Path "./eigen-3.4.0")
$Env:THIRD_PARTY_FMT = (Resolve-Path -Path "./fmt-10.2.1")

$Env:ADSK_MAYA_SDK_2022 = "C:\Program Files\Autodesk\Maya2022\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2023 = "C:\Program Files\Autodesk\Maya2023\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2024 = "C:\Program Files\Autodesk\Maya2024\devkit\devkitBase"

$installedVersions = pyenv versions --bare
foreach ($version in $installedVersions) {
    pyenv local $version
    $pythonPath = pyenv which python
    $pythonRootPath = (Get-Item $pythonPath).Directory.FullName
    $majorMinor = $version -replace '(\d+\.\d+).*', '$1' -replace '\.', ''
    New-Item -Path Env: -Name "PYTHON_$majorMinor" -Value $pythonRootPath

    $pybindPath = (Resolve-Path -Path "..\.venvs\py$majorMinor\.venv\Lib\site-packages\pybind11" -ErrorAction SilentlyContinue)
    New-Item -Path Env: -Name "PYBIND11_$majorMinor" -Value $pybindPath
}

Set-Location $PSScriptRoot
.\skin_plus_plus.sln
