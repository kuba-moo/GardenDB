; example1.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply 
; prompts the user asking them where to install, and drops a copy of example1.nsi
; there. 

;--------------------------------

; The name of the installer
Name "Example1"

; The file to write
OutFile "garden-installer.exe"

; The default installation directory
InstallDir $ProgramFiles\Garden

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page directory
Page instfiles

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File garden.exe
  File libgcc_s_dw2-1.dll
  File libgettextlib-0-18-1.dll
  File libstdc++-6.dll
  File mingwm10.dll
  File QtCore4.dll
  File QtGui4.dll
  File QtSql4.dll
  File rose.ico

  WriteRegStr HKCU "Software\Classes\.grd" "" "grd_auto_file"
  WriteRegStr HKCU "Software\Classes\.grd\OpenWithProgIds" "grd_auto_file" ""

  WriteRegStr HKCU "Software\Classes\grd_auto_file" "" "Garden file"
  WriteRegExpandStr HKCU "Software\Classes\grd_auto_file\DefultIcon" "" "%ProgramFiles%\Garden\rose.ico"
  WriteRegExpandStr HKCU "Software\Classes\grd_auto_file\shell\open\command" "" "%ProgramFiles%\Garden\garden.exe %1"
  
  createShortCut "$SMPROGRAMS\Ogród.lnk" "%ProgramFiles%\Garden\garden.exe"

SectionEnd ; end the section
