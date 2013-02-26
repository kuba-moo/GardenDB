; garden-install.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply 
; prompts the user asking them where to install, and drops a copy of example1.nsi
; there. 

;--------------------------------

; The name of the installer
Name "Garden"

; The file to write
OutFile "garden-installer.exe"

; The default installation directory
InstallDir $ProgramFiles\Garden

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Garden" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Garden (required)"

  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File garden.exe
  File libgcc_s_dw2-1.dll
  File libstdc++-6.dll
  File mingwm10.dll
  File QtCore4.dll
  File QtGui4.dll
  File QtSql4.dll
  File rose.ico
  SetOutPath $INSTDIR\imageformats
  File imageformats\qjpeg4.dll
  SetOutPath $INSTDIR\sqldrivers
  File sqldrivers\qsqlite4.dll

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\Garden" "Install_Dir" "$INSTDIR"

  WriteRegStr HKCU "Software\Classes\.grd" "" "grd_auto_file"
  WriteRegStr HKCU "Software\Classes\.grd\OpenWithProgIds" "grd_auto_file" ""

  WriteRegStr HKCU "Software\Classes\grd_auto_file" "" "Garden file"
  WriteRegStr HKCU "Software\Classes\grd_auto_file\DefaultIcon" "" "$INSTDIR\rose.ico"
  WriteRegStr HKCU "Software\Classes\grd_auto_file\shell\open\command" "" "$INSTDIR\garden.exe %1"
  
  createShortCut "$SMPROGRAMS\Ogród.lnk" "$INSTDIR\garden.exe"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Garden" "DisplayName" "Garden"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Garden" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Garden" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Garden" "NoRepair" 1
  WriteUninstaller "uninstall.exe"

SectionEnd ; end the section

;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Garden"
  DeleteRegKey HKLM SOFTWARE\Garden

  DeleteRegKey HKCU "Software\Classes\.grd"
  DeleteRegKey HKCU "Software\Classes\grd_auto_file"  

  ; Remove files and uninstaller
  Delete "$INSTDIR\imageformats\*.*"
  RMDir "$INSTDIR\imageformats"
  Delete "$INSTDIR\sqldrivers\*.*"
  RMDir "$INSTDIR\sqldrivers"

  Delete "$INSTDIR\*.*"
  RMDir "$INSTDIR"
  
  ; Remove directories used
  Delete "$SMPROGRAMS\Ogród.lnk"

SectionEnd