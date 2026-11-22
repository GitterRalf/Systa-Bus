param(
    [ValidateSet('x64', 'x86')]
    [string]$Architecture = 'x64',
    [string]$QtKit,
    [string]$CompilerBin,
    [switch]$Setup,
    [string]$InnoCompiler = 'C:/Program Files (x86)/Inno Setup 5/ISCC.exe'
)

$ErrorActionPreference = 'Stop'
if ($Architecture -eq 'x86') {
    if (!$QtKit) { $QtKit = 'C:/Qt/Qt5.15.19/mingw81_32' }
    if (!$CompilerBin) { $CompilerBin = 'C:/Qt/Qt5.13.2/Tools/mingw810_32/bin' }
    $qtMajor = 5
    $compilerTarget = 'i686-w64-mingw32'
} else {
    if (!$QtKit) { $QtKit = 'C:/Qt/Qt6/6.11.0/mingw_64' }
    if (!$CompilerBin) { $CompilerBin = 'C:/Qt/Tools/mingw13_1_0/mingw64/bin' }
    $qtMajor = 6
    $compilerTarget = 'x86_64-w64-mingw32'
}
$QtKit = (Resolve-Path -LiteralPath $QtKit).Path
$CompilerBin = (Resolve-Path -LiteralPath $CompilerBin).Path
if ($Setup) { $InnoCompiler = (Resolve-Path -LiteralPath $InnoCompiler).Path }

$previousPath = $env:Path
$previousShell = $env:SHELL
# Keep MSYS tools from Git out of the native MinGW build.
$env:Path = "$CompilerBin;$QtKit/bin;$env:SystemRoot\System32;$env:SystemRoot"
$env:SHELL = Join-Path $env:SystemRoot 'System32/cmd.exe'

try {
    $compiler = Join-Path $CompilerBin 'g++.exe'
    if ((& $compiler -dumpmachine) -ne $compilerTarget) {
        throw "Für $Architecture wird MinGW für $compilerTarget benötigt."
    }
    $qmake = Join-Path $QtKit 'bin/qmake.exe'
    $qtVersion = & $qmake -query QT_VERSION
    if ($LASTEXITCODE -ne 0 -or $qtVersion -notlike "$qtMajor.*") {
        throw "Für $Architecture wird ein Qt-$qtMajor-Kit benötigt."
    }

    $build = New-Item -ItemType Directory -Path (Join-Path $PSScriptRoot "build/windows-$Architecture") -Force
    $project = Join-Path $PSScriptRoot 'Systa_Bus.pro'
    Push-Location -LiteralPath $build.FullName
    try {
        & $qmake $project 'CONFIG+=release' 'CONFIG-=debug' 'CONFIG-=debug_and_release'
        if ($LASTEXITCODE -ne 0) { throw "qmake-Konfiguration fehlgeschlagen ($LASTEXITCODE)." }

        & "$CompilerBin/mingw32-make.exe" -j4
        if ($LASTEXITCODE -ne 0) { throw "Release-Build fehlgeschlagen ($LASTEXITCODE)." }

        $exe = Join-Path $build.FullName 'Systa_Bus.exe'
        if ($qtMajor -eq 6) {
            & "$QtKit/bin/windeployqt.exe" --release --no-translations --no-system-d3d-compiler --no-opengl-sw $exe
            if ($LASTEXITCODE -ne 0) { throw "Qt-Bereitstellung fehlgeschlagen ($LASTEXITCODE)." }
        } else {
            # This Qt 5 MinGW kit includes debug info in its release plugins.
            # windeployqt misidentifies them and skips qwindows.
            foreach ($module in 'Core', 'Gui', 'Network', 'PrintSupport', 'SerialPort', 'Svg', 'Widgets') {
                Copy-Item -LiteralPath "$QtKit/bin/Qt5$module.dll" -Destination $build.FullName
            }
            foreach ($dll in 'libgcc_s_dw2-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll') {
                Copy-Item -LiteralPath "$CompilerBin/$dll" -Destination $build.FullName
            }
            Set-Content -LiteralPath (Join-Path $build.FullName 'qt.conf') -Encoding ascii -Value '[Paths]', 'Prefix=.', 'Plugins=.'
            foreach ($group in 'bearer', 'iconengines', 'imageformats', 'printsupport', 'styles') {
                $pluginDir = Join-Path $QtKit "plugins/$group"
                if (Test-Path -LiteralPath $pluginDir -PathType Container) {
                    $target = New-Item -ItemType Directory -Path (Join-Path $build.FullName $group) -Force
                    Get-ChildItem -LiteralPath $pluginDir -File -Filter '*.dll' |
                        Copy-Item -Destination $target.FullName
                }
            }
            $platforms = New-Item -ItemType Directory -Path (Join-Path $build.FullName 'platforms') -Force
            Copy-Item -LiteralPath (Join-Path $QtKit 'plugins/platforms/qwindows.dll') -Destination $platforms.FullName
        }
    } finally {
        Pop-Location
    }

    if ($Setup) {
        & $InnoCompiler "/DBuildArch=$Architecture" (Join-Path $PSScriptRoot 'Systa-Bus.iss')
        if ($LASTEXITCODE -ne 0) { throw "Installer-Build fehlgeschlagen ($LASTEXITCODE)." }
    }
    Write-Output $exe
} finally {
    $env:Path = $previousPath
    $env:SHELL = $previousShell
}
