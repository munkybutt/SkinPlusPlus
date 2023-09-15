# Check for permissions
if (-NOT ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole] "Administrator")) {
    Write-Output "Not running as Administrator. Restarting as Administrator."
    Start-Process powershell.exe "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`"" -Verb RunAs
    exit
}

# Check if pyenv is installed
try
{
    $pyenvVersion = pyenv --version
    Write-Host "Found pyenv version: $pyenvVersion"
}
catch
{
    # pyenv is not installed, install it
    Invoke-Expression -Command "$((New-Object System.Net.WebClient).DownloadString("https://github.com/pyenv-win/pyenv-win/raw/master/pyenv-win/installer/pyenv-installer.ps1"))"
    Start-Process powershell.exe "-NoProfile -ExecutionPolicy Bypass -File `"$PSCommandPath`"" -Verb RunAs
    exit
}

# Define available Python versions as a dictionary
$pythonVersions = @{
    "3.7" = @{
        "version" = "3.7.9"
        "variable" = "SKPP_PYTHON_37"
    }
    "3.9" = @{
        "version" = "3.9.7"
        "variable" = "SKPP_PYTHON_39"
    }
    "3.10" = @{
        "version" = "3.10.8"
        "variable" = "SKPP_PYTHON_310"
    }
}

# Query the user which versions of Python they want to install
Write-Host "Select index of Python version to install:"
$index = 1
$choices = @{
    1 = "All"
}
$pythonVersions.Keys | ForEach-Object {
    $index++
    $choices.Add($index, $_)
}

$choices.Keys | ForEach-Object {
    $value = $choices[$_]
    Write-Host "[$_]: $($value)"
}

# Receive user input
$userChoice = Read-Host "Enter indices separated by spaces"


if ($userChoice -eq 1)
{
    $indices = New-Object System.Collections.Generic.List[int]
    foreach ($index in 2..$choices.Keys.Count) {
        $indices.Add($index)
    }
}
else
{
    $indices = $userChoice -split ' ' | ForEach-Object { [int]$_ }
}

Write-Host ">>> Selected indices: $indices`n"

foreach ($index in $indices)
{
    # Validate userChoice
    if ($choices.ContainsKey($index))
    {
        $selectedVersion = $pythonVersions[$choices[$index]]["version"]
        $pyVersionClean = $selectedVersion.Replace(".", "")
        # $selectedVariable = $pythonVersions[$choices[$index]]["variable"]

        Write-Host ">>> Installing: $($selectedVersion)"

        # Check if the chosen Python version is already installed
        $isInstalled = pyenv versions --bare | Select-String -Pattern "^$($selectedVersion)$"
        if ($isInstalled)
        {
            Write-Host ">>> Python version $($selectedVersion) is already installed."
        }
        else
        {
            C:\Users\Sheaky\.pyenv\pyenv-win\versions\3.9.7\versions\3.9.7\include
            # Use pyenv to install the chosen Python version
            pyenv install $($selectedVersion) -s
        }
        # Query the Python version binariy location
        pyenv shell $selectedVersion
        $pythonExe = pyenv which python
        Write-Host ">>> Found exe path: $pythonExe"
        $pythonHome = Split-Path $pythonExe -Parent
        $currentFilePath = $MyInvocation.MyCommand.Path
        $currentDirectory = Split-Path $currentFilePath -Parent
        Write-Host ">>> Creating venv: py$pyVersionClean"
        $venvPath = "$currentDirectory/.envs/py$pyVersionClean"
        $venvScriptsPath = "$currentDirectory/.envs/py$pyVersionClean/Scripts"
        & $pythonExe -m venv $venvPath
        Set-Location $venvScriptsPath
        ./activate
        Write-Host ">>> Installing pybind11 for venv: py$pyVersionClean"
        python -m pip install pybind11
        deactivate
        $pybind11IncludePath = "$venvPath/Lib/site-packages/pybind11/include"
        $pybind11Variable = "SKPP_PYBIND11_$pyVersionClean"
        [Environment]::SetEnvironmentVariable($pybind11Variable, $pybind11IncludePath, "Machine")
        Write-Host ">>> Pybind11 path $pybind11IncludePath set as environment variable: $pybind11Variable`n"
        Set-Location $currentDirectory
        $pythonVariable =  "SKPP_PYTHON_$pyVersionClean"
        [Environment]::SetEnvironmentVariable($pythonVariable, $pythonHome, "Machine")
        Write-Host ">>> Python path $pythonHome set as environment variable: $pythonVariable`n"
    }
    else
    {
        Write-Host "!!! Invalid choice: $($index) skipping..."
    }
}

[string]$userChoice = Read-Host "Define Maya SDK environment variables (Y/N)?"
if ($userChoice.ToLower() -eq "y")
{
    [Environment]::SetEnvironmentVariable(
        "SKPP_ADSK_MAYA_SDK_2022", "C:\Program Files\Autodesk\Maya2022\devkit\devkitBase", "Machine"
    )
    [Environment]::SetEnvironmentVariable(
        "SKPP_ADSK_MAYA_SDK_2023", "C:\Program Files\Autodesk\Maya2023\devkit\devkitBase", "Machine"
    )
    [Environment]::SetEnvironmentVariable(
        "SKPP_ADSK_MAYA_SDK_2024", "C:\Program Files\Autodesk\Maya2024\devkit\devkitBase", "Machine"
    )
}
pause