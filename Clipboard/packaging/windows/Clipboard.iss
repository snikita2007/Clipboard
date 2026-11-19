; Build with Inno Setup 6.7+; see build-installer.ps1.
#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif
#ifndef PayloadDir
  #define PayloadDir "..\..\..\ClipboardApp"
#endif
#ifndef OutputDir
  #define OutputDir "..\..\.."
#endif
#ifndef Appearance
  #define Appearance "dynamic"
#endif

#ifdef VerificationBuild
  #define ProductName "Clipboard Installer Test"
  #define ProductId "{{805F60F9-DF05-4875-A196-072C0B2B404B}"
  #define SettingsKey "Software\KNGC\ClipboardInstallerTest\ApplicationSettings"
  #define RunValue "ClipboardInstallerTest"
  #define OutputName "Clipboard-Setup-Verification"
#else
  #define ProductName "Clipboard"
  #define ProductId "{{E91BDE69-AB4F-4645-BE99-4E680FB6EE63}"
  #define SettingsKey "Software\KNGC\ClipboardApp\ApplicationSettings"
  #define RunValue "ClipboardApp"
  #define OutputName "Clipboard-Setup-" + AppVersion
#endif

[Setup]
AppId={#ProductId}
AppName={#ProductName}
AppVersion={#AppVersion}
AppVerName={#ProductName} {#AppVersion}
AppPublisher=KNGC
DefaultDirName={localappdata}\Programs\{#ProductName}
DefaultGroupName={#ProductName}
PrivilegesRequired=lowest
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
MinVersion=10.0.17763
UsePreviousAppDir=yes
DisableDirPage=no
DisableWelcomePage=yes
DisableProgramGroupPage=yes
DisableReadyPage=yes
DisableFinishedPage=no
DirExistsWarning=no
AllowRootDirectory=no
AllowUNCPath=no
UninstallDisplayName={#ProductName}
UninstallDisplayIcon={app}\ClipboardApp.exe
SetupIconFile=..\..\ui\assets\app.ico
OutputDir={#OutputDir}
OutputBaseFilename={#OutputName}
Compression=lzma2/max
SolidCompression=yes
LZMANumBlockThreads=2
CloseApplications=yes
CloseApplicationsFilter=*.exe,*.dll
RestartApplications=no
SetupLogging=yes
WizardStyle=modern {#Appearance} windows11 hidebevels
#if Appearance == "dark"
WizardBackColor=#202024
#else
WizardBackColor=#f6f6f8
#endif
WizardBackColorDynamicDark=#202024
WizardImageFile=
WizardSmallImageFile=
WizardSizePercent=130,125
WizardKeepAspectRatio=no
WizardResizable=no
DefaultDialogFontName=Segoe UI
ShowLanguageDialog=no
LanguageDetectionMethod=uilanguage
VersionInfoVersion={#AppVersion}
VersionInfoProductName=Clipboard
VersionInfoDescription=Clipboard Setup

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[LangOptions]
DialogFontName=Segoe UI
DialogFontSize=10

[Messages]
russian.SetupWindowTitle=Clipboard — установка
english.SetupWindowTitle=Clipboard — Setup
russian.SelectDirLabel=
english.SelectDirLabel=
russian.SelectDirBrowseLabel=
english.SelectDirBrowseLabel=
russian.ButtonInstall=&Установить
english.ButtonInstall=&Install
russian.FinishedHeadingLabel=Clipboard готов к работе
english.FinishedHeadingLabel=Clipboard is ready
russian.FinishedLabel=Копируйте как обычно. Clipboard сохранит текст, фото и файлы и поможет передать их на другое устройство.
english.FinishedLabel=Copy as usual. Clipboard keeps your text, photos and files close, and helps send them to your other devices.
russian.FinishedRestartLabel=Clipboard установлен. Перезагрузите Windows, чтобы завершить обновление.
english.FinishedRestartLabel=Clipboard is installed. Restart Windows to finish the update.

[CustomMessages]
russian.HeroTitle=Ваш буфер обмена.%nНа всех устройствах.
english.HeroTitle=Your clipboard.%nAcross your devices.
russian.HeroDescription=Текст, фото и файлы — под рукой.%nСинхронизация по Bluetooth и Wi-Fi.
english.HeroDescription=Text, photos and files, always close.%nSync over Bluetooth and Wi-Fi.
russian.InstallSubtitle=Настройте под себя — и можно начинать.
english.InstallSubtitle=Make it yours, then you're ready to go.
russian.InstallLocation=Папка установки
english.InstallLocation=Install location
russian.Browse=Изменить…
english.Browse=Change…
russian.Startup=Запускать при входе в Windows
english.Startup=Start when I sign in to Windows
russian.StartupHint=Можно изменить позже в настройках Clipboard.
english.StartupHint=You can change this later in Clipboard settings.
russian.Desktop=Создать ярлык на рабочем столе
english.Desktop=Create a desktop shortcut
russian.Footer=Clipboard {#AppVersion}   ·   Windows 10 / 11
english.Footer=Clipboard {#AppVersion}   ·   Windows 10 / 11
russian.InstallingTitle=Устанавливаем Clipboard
english.InstallingTitle=Installing Clipboard
russian.InstallingSubtitle=Всё необходимое уже в установщике.
english.InstallingSubtitle=Everything you need is included.
russian.InstallingHint=Добавляем приложение и ярлыки.
english.InstallingHint=Adding the app and your shortcuts.
russian.ShortcutHint=Открыть историю из любого приложения
english.ShortcutHint=Open your history from any app
russian.DevicesHint=Подключение устройств — в настройках Clipboard.
english.DevicesHint=Connect your devices in Clipboard settings.
russian.Launch=Открыть Clipboard
english.Launch=Open Clipboard
russian.Finish=Готово
english.Finish=Done

[Files]
Source: "{#PayloadDir}\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "*.pdb,*.log"
Source: "..\..\ui\assets\app.png"; Flags: dontcopy

[Icons]
Name: "{userprograms}\{#ProductName}"; Filename: "{app}\ClipboardApp.exe"; WorkingDir: "{app}"; Parameters: "--show"
Name: "{userdesktop}\{#ProductName}"; Filename: "{app}\ClipboardApp.exe"; WorkingDir: "{app}"; Parameters: "--show"; Check: WantsDesktopShortcut

[Registry]
Root: HKCU; Subkey: "{#SettingsKey}"; ValueType: string; ValueName: "launchAtStartup"; ValueData: "{code:StartupSetting}"
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "{#RunValue}"; ValueData: """{app}\ClipboardApp.exe"""; Check: WantsStartup

[Run]
Filename: "{app}\ClipboardApp.exe"; Parameters: "--show"; WorkingDir: "{app}"; Description: "{cm:Launch}"; Flags: nowait postinstall skipifsilent

[Code]
var
  StartupCheck, DesktopCheck: TNewCheckBox;
  FooterText, PercentText: TNewStaticText;
  MutedColor: TColor;

function WantsStartup: Boolean;
begin
  Result := StartupCheck.Checked;
end;

function WantsDesktopShortcut: Boolean;
begin
  Result := DesktopCheck.Checked;
end;

function StartupSetting(Param: String): String;
begin
  if WantsStartup then Result := 'true' else Result := 'false';
end;

function MakeLabel(Parent: TWinControl; X, Y, Width, Height: Integer;
  Caption: String; FontSize: Integer; Bold, Muted: Boolean): TNewStaticText;
begin
  Result := TNewStaticText.Create(WizardForm);
  Result.Parent := Parent;
  Result.AutoSize := False;
  Result.SetBounds(X, Y, Width, Height);
  Result.Caption := Caption;
  Result.WordWrap := True;
  Result.ShowAccelChar := False;
  Result.Font.Size := FontSize;
  if Bold then Result.Font.Name := 'Segoe UI Semibold';
  if Muted and not HighContrastActive then begin
    Result.StyleElements := Result.StyleElements - [seFont];
    Result.Font.Color := MutedColor;
  end;
end;

procedure AddLogo(Parent: TWinControl; X, Y, Size: Integer);
var
  Logo: TBitmapImage;
begin
  Logo := TBitmapImage.Create(WizardForm);
  Logo.Parent := Parent;
  Logo.SetBounds(X, Y, Size, Size);
  Logo.Stretch := True;
  Logo.PngImage.LoadFromFile(ExpandConstant('{tmp}\app.png'));
end;

procedure InitializeWizard;
var
  W: Integer;
  Setting: String;
  LabelControl: TNewStaticText;
begin
  if IsDarkInstallMode then MutedColor := StrToColor('#b8b8c2')
  else MutedColor := StrToColor('#656570');
  ExtractTemporaryFile('app.png');

  WizardForm.PageNameLabel.Font.Name := 'Segoe UI Semibold';
  WizardForm.PageNameLabel.Font.Size := 16;
  WizardForm.PageNameLabel.Left := WizardForm.PageDescriptionLabel.Left;
  WizardForm.PageNameLabel.Height := ScaleY(28);
  WizardForm.PageDescriptionLabel.Top := WizardForm.PageNameLabel.Top + ScaleY(32);
  WizardForm.PageDescriptionLabel.Height := ScaleY(32);
  WizardForm.PageDescriptionLabel.Font.Size := 10;

  W := WizardForm.SelectDirPage.ClientWidth;
  WizardForm.SelectDirBitmapImage.Visible := False;
  WizardForm.SelectDirLabel.Visible := False;
  WizardForm.SelectDirBrowseLabel.Visible := False;
  LabelControl := MakeLabel(WizardForm.SelectDirPage, 0, ScaleY(8), W - ScaleX(140),
    ScaleY(72), CustomMessage('HeroTitle'), 24, True, False);
  LabelControl := MakeLabel(WizardForm.SelectDirPage, 0, ScaleY(88), W - ScaleX(130),
    ScaleY(48), CustomMessage('HeroDescription'), 11, False, True);
  AddLogo(WizardForm.SelectDirPage, W - ScaleX(112), ScaleY(12), ScaleX(104));

  LabelControl := MakeLabel(WizardForm.SelectDirPage, 0, ScaleY(158), W,
    ScaleY(22), CustomMessage('InstallLocation'), 10, True, False);
  WizardForm.DirEdit.SetBounds(0, ScaleY(184), W - ScaleX(108), ScaleY(30));
  WizardForm.DirBrowseButton.SetBounds(W - ScaleX(96), ScaleY(184), ScaleX(96), ScaleY(30));
  WizardForm.DirBrowseButton.Caption := CustomMessage('Browse');

  StartupCheck := TNewCheckBox.Create(WizardForm);
  StartupCheck.Parent := WizardForm.SelectDirPage;
  StartupCheck.SetBounds(0, ScaleY(236), W, ScaleY(24));
  StartupCheck.Caption := CustomMessage('Startup');
  StartupCheck.TabOrder := WizardForm.DirBrowseButton.TabOrder + 1;
  StartupCheck.Checked := False;
  if RegQueryStringValue(HKCU, '{#SettingsKey}', 'launchAtStartup', Setting) then
    StartupCheck.Checked := (CompareText(Setting, 'true') = 0) or (Setting = '1')
  else
    StartupCheck.Checked := RegValueExists(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Run', '{#RunValue}');
  Setting := ExpandConstant('{param:AUTOSTART|}');
  if Setting <> '' then StartupCheck.Checked := Setting = '1';
  LabelControl := MakeLabel(WizardForm.SelectDirPage, ScaleX(23), ScaleY(264), W - ScaleX(23),
    ScaleY(24), CustomMessage('StartupHint'), 9, False, True);

  DesktopCheck := TNewCheckBox.Create(WizardForm);
  DesktopCheck.Parent := WizardForm.SelectDirPage;
  DesktopCheck.SetBounds(0, ScaleY(304), W, ScaleY(24));
  DesktopCheck.Caption := CustomMessage('Desktop');
  DesktopCheck.TabOrder := StartupCheck.TabOrder + 1;
  DesktopCheck.Checked := FileExists(ExpandConstant('{userdesktop}\{#ProductName}.lnk'));
  Setting := ExpandConstant('{param:DESKTOPICON|}');
  if Setting <> '' then DesktopCheck.Checked := Setting = '1';
  WizardForm.DiskSpaceLabel.SetBounds(0, ScaleY(218), W, ScaleY(14));
  WizardForm.DiskSpaceLabel.Font.Size := 8;
  if not HighContrastActive then begin
    WizardForm.DiskSpaceLabel.StyleElements := WizardForm.DiskSpaceLabel.StyleElements - [seFont];
    WizardForm.DiskSpaceLabel.Font.Color := MutedColor;
  end;

  WizardForm.NextButton.Width := ScaleX(124);
  WizardForm.NextButton.Left := WizardForm.CancelButton.Left - ScaleX(12) - WizardForm.NextButton.Width;
  WizardForm.NextButton.Font.Name := 'Segoe UI Semibold';
  FooterText := MakeLabel(WizardForm, ScaleX(24), WizardForm.NextButton.Top + ScaleY(5),
    WizardForm.NextButton.Left - ScaleX(36), ScaleY(24), CustomMessage('Footer'), 9, False, True);

  WizardForm.StatusLabel.Top := ScaleY(110);
  WizardForm.ProgressGauge.SetBounds(0, ScaleY(146), WizardForm.InstallingPage.ClientWidth, ScaleY(8));
  WizardForm.FilenameLabel.Visible := False;
  PercentText := MakeLabel(WizardForm.InstallingPage, 0, ScaleY(26), W, ScaleY(64), '0%', 32, True, False);
  LabelControl := MakeLabel(WizardForm.InstallingPage, 0, ScaleY(174), W, ScaleY(40),
    CustomMessage('InstallingHint'), 11, False, True);

  WizardForm.FinishedHeadingLabel.SetBounds(ScaleX(24), ScaleY(46),
    WizardForm.FinishedPage.ClientWidth - ScaleX(170), ScaleY(70));
  WizardForm.FinishedHeadingLabel.Font.Name := 'Segoe UI Semibold';
  WizardForm.FinishedHeadingLabel.Font.Size := 24;
  WizardForm.FinishedLabel.SetBounds(ScaleX(24), ScaleY(132),
    WizardForm.FinishedPage.ClientWidth - ScaleX(175), ScaleY(80));
  WizardForm.FinishedLabel.Font.Size := 11;
  AddLogo(WizardForm.FinishedPage, WizardForm.FinishedPage.ClientWidth - ScaleX(136), ScaleY(52), ScaleX(96));
  LabelControl := MakeLabel(WizardForm.FinishedPage, ScaleX(24), ScaleY(240), W,
    ScaleY(36), 'Alt + V', 22, True, False);
  LabelControl := MakeLabel(WizardForm.FinishedPage, ScaleX(24), ScaleY(282), W,
    ScaleY(28), CustomMessage('ShortcutHint'), 11, False, True);
  LabelControl := MakeLabel(WizardForm.FinishedPage, ScaleX(24), ScaleY(322), W,
    ScaleY(30), CustomMessage('DevicesHint'), 10, False, True);
  WizardForm.RunList.SetBounds(ScaleX(24), WizardForm.FinishedPage.ClientHeight - ScaleY(44), W, ScaleY(30));
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpSelectDir then begin
    WizardForm.PageNameLabel.Caption := 'Clipboard';
    WizardForm.PageDescriptionLabel.Caption := CustomMessage('InstallSubtitle');
    WizardForm.NextButton.Caption := SetupMessage(msgButtonInstall);
  end else if CurPageID = wpInstalling then begin
    WizardForm.PageNameLabel.Caption := CustomMessage('InstallingTitle');
    WizardForm.PageDescriptionLabel.Caption := CustomMessage('InstallingSubtitle');
  end else if CurPageID = wpFinished then
    WizardForm.NextButton.Caption := CustomMessage('Finish');
end;

procedure CurInstallProgressChanged(CurProgress, MaxProgress: Integer);
begin
  if MaxProgress > 0 then
    PercentText.Caption := IntToStr(Round(100.0 * CurProgress / MaxProgress)) + '%';
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep = ssPostInstall) and not WantsStartup then
    RegDeleteValue(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Run', '{#RunValue}');
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
var
  Command: String;
begin
  if CurUninstallStep = usPostUninstall then begin
    { Remove only the Run entry pointing to this installation. Preserve settings. }
    if RegQueryStringValue(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Run', '{#RunValue}', Command) then
      if (CompareText(Command, '"' + ExpandConstant('{app}\ClipboardApp.exe') + '"') = 0)
        or (CompareText(Command, ExpandConstant('{app}\ClipboardApp.exe')) = 0) then
        RegDeleteValue(HKCU, 'Software\Microsoft\Windows\CurrentVersion\Run', '{#RunValue}');
  end;
end;
