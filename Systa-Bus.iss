; Build the requested Windows variant with build.ps1 before compiling this script.
#ifndef BuildArch
  #define BuildArch "x64"
#endif
#if BuildArch == "x64"
  #define AppIdValue "{{ACB9122A-5C2C-44EF-AD85-F2EE230A5689}"
  #define DisplayName "Systa-Bus (64 Bit)"
#elif BuildArch == "x86"
  #define AppIdValue "{{EC43869D-B233-48D6-87CF-9EC558701DEE}"
  #define DisplayName "Systa-Bus (32 Bit)"
#else
  #error BuildArch must be x64 or x86.
#endif
#define BuildDir "build\windows-" + BuildArch
#define AppExe "Systa_Bus.exe"
#define SetupVersion GetFileVersion(SourcePath + BuildDir + "\" + AppExe)
#if SetupVersion == ""
  #error Build the Systa-Bus release executable before compiling the installer.
#endif

[Setup]
AppId={#AppIdValue}
AppName={#DisplayName}
AppVersion={#SetupVersion}
AppVerName={#DisplayName} {#SetupVersion}
AppPublisher=Ralf Tralow
VersionInfoVersion={#SetupVersion}
VersionInfoDescription={#DisplayName} Setup
DefaultDirName={localappdata}\Systa-Bus\{#BuildArch}
DefaultGroupName={#DisplayName}
AllowNoIcons=yes
; Inno Setup added lowest in 5.3.7. The legacy compiler uses none.
#if Ver >= 0x05030700
PrivilegesRequired=lowest
#else
PrivilegesRequired=none
#endif
#if BuildArch == "x64"
ArchitecturesAllowed=x64
ArchitecturesInstallIn64BitMode=x64
#else
ArchitecturesAllowed=x86 x64
#endif
SetupIconFile=images\logo.ico
UninstallDisplayIcon={app}\{#AppExe}
OutputDir=installer
OutputBaseFilename=Systa-Bus-Setup-{#SetupVersion}-{#BuildArch}
Compression=lzma
SolidCompression=yes

[Languages]
Name: "german"; MessagesFile: "compiler:Languages\German.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "{#BuildDir}\{#AppExe}"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\*.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\platforms\qwindows.dll"; DestDir: "{app}\platforms"; Flags: ignoreversion
#if BuildArch == "x86"
Source: "{#BuildDir}\qt.conf"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#BuildDir}\bearer\*.dll"; DestDir: "{app}\bearer"; Flags: ignoreversion skipifsourcedoesntexist
#endif
Source: "{#BuildDir}\generic\*.dll"; DestDir: "{app}\generic"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\iconengines\*.dll"; DestDir: "{app}\iconengines"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\imageformats\*.dll"; DestDir: "{app}\imageformats"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\networkinformation\*.dll"; DestDir: "{app}\networkinformation"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\printsupport\*.dll"; DestDir: "{app}\printsupport"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\styles\*.dll"; DestDir: "{app}\styles"; Flags: ignoreversion skipifsourcedoesntexist
Source: "{#BuildDir}\tls\*.dll"; DestDir: "{app}\tls"; Flags: ignoreversion skipifsourcedoesntexist
Source: "README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "ReleaseNotes.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "THIRD-PARTY-NOTICES.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "qcustomplot\GPL.txt"; DestDir: "{app}\licenses"; DestName: "QCustomPlot-GPL-3.0.txt"; Flags: ignoreversion
Source: "licenses\*.txt"; DestDir: "{app}\licenses"; Flags: ignoreversion

[Icons]
Name: "{group}\{#DisplayName}"; Filename: "{app}\{#AppExe}"; WorkingDir: "{app}"
