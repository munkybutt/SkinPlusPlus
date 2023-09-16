# Set dependency variables here:
$Env:THIRD_PARTY_EIGEN = (Resolve-Path -Path "..\3rdParty\eigen-3.4.0")
$Env:THIRD_PARTY_FMT = (Resolve-Path -Path "..\3rdParty\fmt-10.1.1")

$Env:ADSK_MAYA_SDK_2022 = "C:\Program Files\Autodesk\Maya2022\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2023 = "C:\Program Files\Autodesk\Maya2023\devkit\devkitBase"

$Env:PYBIND11_37 = (Resolve-Path -Path "..\.venvs\py379\Lib\site-packages\pybind11")
$Env:PYBIND11_39 = (Resolve-Path -Path "..\.venvs\py397\Lib\site-packages\pybind11")

$Env:PYTHON_37 = (Resolve-Path -Path "C:\Users\Sheaky\.pyenv\pyenv-win\versions\3.7.9")
$Env:PYTHON_39 = (Resolve-Path -Path "C:\Users\Sheaky\.pyenv\pyenv-win\versions\3.9.7")

.\skin_plus_plus.sln
