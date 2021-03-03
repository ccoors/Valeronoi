!include "MUI2.nsh"

Name "Valeronoi"
OutFile "InstallValeronoi.exe"
Unicode True

InstallDir "$LOCALAPPDATA\Valeronoi"

InstallDirRegKey HKCU "Software\ccoors\Valeronoi" ""

RequestExecutionLevel user

Var StartMenuFolder

!define MUI_ABORTWARNING

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "COPYING"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\ccoors\Valeronoi"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "Valeronoi" SecValeronoi
  SectionIn RO
  SetOutPath "$INSTDIR"

  File /r *.exe
  File /r *.dll

  SetShellVarContext current
  CreateShortCut "$DESKTOP\Valeronoi.lnk" "$INSTDIR\valeronoi.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Valeronoi.lnk" "$INSTDIR\valeronoi.exe"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_END

  WriteRegStr HKCU "Software\ccoors\Valeronoi" "" $INSTDIR
  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

LangString DESC_SecValeronoi ${LANG_ENGLISH} "Valeronoi main application."

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecValeronoi} $(DESC_SecValeronoi)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir /r "$INSTDIR"
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$DESKTOP\Valeronoi.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Valeronoi.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  RMDir /r "$SMPROGRAMS\$StartMenuFolder"
  DeleteRegKey HKCU "Software\ccoors\Valeronoi"
SectionEnd
