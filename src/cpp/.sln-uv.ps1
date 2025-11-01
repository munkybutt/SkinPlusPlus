# Set dependency variables here:
$devFolder = Resolve-Path -Path "$PSScriptRoot\..\..\.dev"
Write-Host $devFolder
Set-Location "$devFolder\ThirdParty"
$Env:THIRD_PARTY_EIGEN = (Resolve-Path -Path "./eigen")
$Env:THIRD_PARTY_FMT = (Resolve-Path -Path "./fmt")

$Env:ADSK_MAYA_SDK_2022 = "C:\Program Files\Autodesk\Maya2022\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2023 = "C:\Program Files\Autodesk\Maya2023\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2024 = "C:\Program Files\Autodesk\Maya2024\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2025 = "C:\Program Files\Autodesk\Maya2025\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2026 = "C:\Program Files\Autodesk\Maya2026\devkit\devkitBase"

$installedVersions = uv python list --only-installed
foreach ($version in $installedVersions) {
    $version = $version.Split("-")[1]
    Write-Host $version
    uv python pin $version
    $pythonPath = uv python find $version
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
