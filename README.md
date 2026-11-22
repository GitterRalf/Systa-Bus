# Systa-Bus

Windows-Anwendung zum Auslesen der Paradigma-Solarregelung SystaSolar Aqua 2 über eine serielle Schnittstelle. Sie zeigt Messwerte und Temperaturverläufe an, zeichnet Messdaten als CSV auf und kann sie über TCP weitergeben.

Entwickelt und gepflegt von Ralf Tralow mit Unterstützung von OpenAI Codex.

## Installation

| Installer | Qt und Compiler |
| --- | --- |
| `Systa-Bus-Setup-0.1.3.6-x64.exe` | Qt 6.11.0 mit MinGW 13.1 (64 Bit) |
| `Systa-Bus-Setup-0.1.3.6-x86.exe` | Qt 5.15.19 mit MinGW 8.1 (32 Bit) |

Beide Setups enthalten die passenden Qt- und MinGW-Laufzeitdateien. Sie lassen sich mit einem normalen Windows-Benutzerkonto ausführen und installieren standardmäßig unter `%LOCALAPPDATA%\Systa-Bus\x64` beziehungsweise `%LOCALAPPDATA%\Systa-Bus\x86`. Beide Varianten können unter 64-Bit-Windows installiert werden. Jede Variante speichert ihre eigene INI-Datei und die aufgezeichneten CSV-Dateien im Installationsordner.

## Bedienung

1. Die serielle Schnittstelle zur Anlage anschließen und unter **Einstellungen** den passenden COM-Port auswählen. Systa-Bus verwendet 9.600 Baud, 8 Datenbits, keine Parität und ein Stoppbit.
2. Die Temperaturmessungen im Diagramm verfolgen. **Start** beginnt die CSV-Aufzeichnung mit dem eingetragenen Dateinamen, **Stop** beendet sie.
3. Für die Weitergabe der Messwerte unter **Einstellungen** den TCP-Server aktivieren. Seine Voreinstellungen sind `127.0.0.1` und Port `5001`.

## Release bauen

Im Projektordner mit PowerShell ausführen.

```powershell
.\build.ps1 -Architecture x64 -Setup
.\build.ps1 -Architecture x86 -Setup
```

Benötigt werden das jeweilige Qt-MinGW-Kit mit Widgets, Serial Port, Network und Print Support sowie Inno Setup. Über `-QtKit`, `-CompilerBin` und `-InnoCompiler` lassen sich andere Installationspfade angeben. Die Anwendungs-Builds liegen unter `build\windows-x64` und `build\windows-x86`. Mit `-Setup` schreibt das Skript die Installer nach `installer`. Die Setup-Version wird aus den Windows-Versionsinformationen der Anwendung gelesen, die auf `version.h` basieren.

## Drittanbieter

Das Diagramm verwendet QCustomPlot 1.1.1 von Emanuel Eichhammer mit Anpassungen für Qt 6. Der Quellcode und die [GPL-Lizenz von QCustomPlot](qcustomplot/GPL.txt) liegen im Projekt. Hinweise zu den mitgelieferten Komponenten stehen in [THIRD-PARTY-NOTICES.txt](THIRD-PARTY-NOTICES.txt).
