# Set dependency variables here:
$Env:THIRD_PARTY_EIGEN = (Resolve-Path -Path "..\3rdParty\eigen-3.4.0")
$Env:THIRD_PARTY_FMT = (Resolve-Path -Path "..\3rdParty\fmt-9.1.0")

$Env:ADSK_MAYA_SDK_2022 = "C:\Program Files\Autodesk\Maya2022\devkit\devkitBase"
$Env:ADSK_MAYA_SDK_2023 = "C:\Program Files\Autodesk\Maya2023\devkit\devkitBase"

$Env:PYBIND11_37 = (Resolve-Path -Path "..\.venvs\37\.venv\Lib\site-packages\pybind11")
$Env:PYBIND11_39 = (Resolve-Path -Path "..\.venvs\39\.venv\Lib\site-packages\pybind11")

$Env:PYTHON_37 = (Resolve-Path -Path "..\..\..\..\python\libs\.pyenv\pyenv-win\versions\3.7.9")
$Env:PYTHON_39 = (Resolve-Path -Path "..\..\..\..\python\libs\.pyenv\pyenv-win\versions\3.9.13")

.\skin_plus_plus.sln
