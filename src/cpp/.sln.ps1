# Set dependency variables here:
Set-Location "$PSScriptRoot\..\.3rdparty"
$Env:THIRD_PARTY_EIGEN = (Resolve-Path -Path "./eigen")
$Env:THIRD_PARTY_FMT = (Resolve-Path -Path "./fmt")

$Env:ADSK_MAYA_SDK_2022 = "C:\Program Files\Autodesk\Maya2022\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2023 = "C:\Program Files\Autodesk\Maya2023\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2024 = "C:\Program Files\Autodesk\Maya2024\devkit\devkitBase"

$installedVersions = pyenv versions --bare
foreach ($version in $installedVersions) {
    Write-Host $version
    pyenv local $version
    $pythonPath = pyenv which python
    $pythonRootPath = (Get-Item $pythonPath).Directory.FullName
    $majorMinor = $version -replace '(\d+\.\d+).*', '$1' -replace '\.', ''
    Set-Item "Env:PYTHON_$majorMinor" $pythonRootPath

    $pybindPath = Resolve-Path -Path "..\.venvs\py$majorMinor\.venv\Lib\site-packages\pybind11" -ErrorAction SilentlyContinue
    if ($pybindPath) {
        Write-Host "Setting PyBind11 path: $pybindPath"
        Set-Item "Env:PYBIND11_$majorMinor" $pybindPath
    }
}

Set-Location $PSScriptRoot
.\skin_plus_plus.sln
