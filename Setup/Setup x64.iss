#define ApplicationVersion "1.0.0.9"

[Setup]
;-- Main Setup Information
 AppName                         = Expression Calculator
 AppVerName                      = Expression Calculator {#ApplicationVersion}
 AppCopyright                    = Copyright © 1995-2014 NetworkDLS.
 DefaultDirName                  = {pf}\NetworkDLS\Expression Calculator
 DefaultGroupName                = NetworkDLS\Expression Calculator
 UninstallDisplayIcon            = {app}\ECalc.Exe
 WizardImageFile                 = ..\..\@Resources\Setup\LgSetup.bmp
 WizardSmallImageFile            = ..\..\@Resources\Setup\SmSetup.bmp
 PrivilegesRequired              = PowerUser
 Uninstallable                   = Yes
 Compression                     = ZIP/9
 OutputBaseFilename              = ExpCalc64 {#ApplicationVersion}
 MinVersion                      = 0.0,5.0
 ArchitecturesInstallIn64BitMode = x64
 ArchitecturesAllowed            = x64

;-- Windows 2000 & XP (Support Dialog)
 AppPublisher    = NetworkDLS
 AppPublisherURL = http://www.NetworkDLS.com/
 AppUpdatesURL   = http://www.NetworkDLS.com/
 AppVersion      = {#ApplicationVersion}

[Files]
 Source: "..\Release\x64\ECalc.Exe";                DestDir: "{app}"; Flags: IgnoreVersion;
 Source: "AutoUpdate.xml";                          DestDir: "{app}"; Flags: IgnoreVersion;
 Source: "..\..\@AutoUpdate\x64\AutoUpdate.Exe";    DestDir: "{app}"; Flags: RestartReplace;
;Source: "Documentation\Help.chm";                  DestDir: "{app}";

[Icons]
 Name: "{group}\Expression Calculator";          Filename: "{app}\ECalc.Exe";
;Name: "{group}\Documentation";                  Filename: "{app}\Help.chm";
 Name: "{group}\AutoUpdate Calculator";          Filename: "{app}\AutoUpdate.Exe";
 Name: "{group}\Uninstall Calculator";           Filename: "{uninstallexe}";
 Name: "{commondesktop}\Expression Calculator";  Filename: "{app}\ECalc.exe"; Tasks: DesktopIcon\Common;
 Name: "{userdesktop}\Expression Calculator";    Filename: "{app}\ECalc.exe"; Tasks: DesktopIcon\User;

[Registry]
 Root: HKLM; Subkey: "Software\NetworkDLS\Expression Calculator"; Flags: uninsdeletekey noerror;
 Root: HKLM; Subkey: "Software\NetworkDLS\Expression Calculator"; ValueType: string; ValueName: "Path"; ValueData:"{app}";

[Run]
 Filename: "{app}\ECalc.Exe"; Description: "Launch application"; Flags: postinstall nowait skipifsilent

[Tasks]
 Name: DesktopIcon;        Description: "Create a &desktop icon"; GroupDescription: "Additional icons:";
 Name: DesktopIcon\Common; Description: "For all users"; GroupDescription: "Additional icons:"; Flags: exclusive
 Name: DesktopIcon\User;   Description: "For the current user only"; GroupDescription: "Additional icons:"; Flags: exclusive unchecked

