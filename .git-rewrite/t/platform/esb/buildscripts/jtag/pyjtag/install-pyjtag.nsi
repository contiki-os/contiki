Name "install-pyjtag"
OutFile "install-pyjtag.exe"

!define SF_SELECTED   1
!define SF_SUBSEC     2
!define SF_SUBSECEND  4
!define SF_BOLD       8
!define SF_RO         16
!define SF_EXPAND     32

!define SECTION_OFF   0xFFFFFFFE

LicenseText License
LicenseData license.txt

SetOverwrite on
SetDateSave on

; The default installation directory
InstallDir $PROGRAMFILES\mspgcc
; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM SOFTWARE\mspgcc "rootdir"

; The text to prompt the user to enter a directory
DirText "This will install the pyjtag executables. You can choose the same \
         directory as for the other mspgcc tools."

; The text to prompt the user to enter a directory
ComponentText "Select which optional things you want installed."

Section "msp430-jtag (required)"
    SectionIn RO
    SetOutPath $INSTDIR
    
    File /r bin
    File /oname=license-pyjtag.txt      license.txt
    File /oname=readme-pyjtag.txt       readme.txt
    File /oname=bin\jtag.py             jtag.py

    ; Write the installation path into the registry
    WriteRegStr HKLM SOFTWARE\mspgcc "rootdir" "$INSTDIR"
    ; Write the uninstall keys for Windows
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\mspgcc-pyjtag" "DisplayName" "mspgcc pyjtag (remove only)"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\mspgcc-pyjtag" "UninstallString" '"$INSTDIR\uninstall-pyjtag.exe"'
    WriteUninstaller "uninstall-pyjtag.exe"
SectionEnd

Section "giveio (needed on Win NT/2k/XP, but NOT on 9x/ME)"
    SetOutPath $INSTDIR\bin
    File ..\jtag\hardware_access\giveio\giveio.sys
    File ..\jtag\hardware_access\giveio\loaddrv.exe
    SetOutPath $INSTDIR
    nsExec::ExecToLog '$INSTDIR\bin\loaddrv.exe install giveio $INSTDIR\bin\giveio.sys'
    Pop $0                              ;return value/error/timeout
    IntCmp $0 2 ext_here                ;assume its alredy installed
    IntCmp $0 0 0 ext_err ext_err       ;if not 0 -> error
    nsExec::ExecToLog '$INSTDIR\bin\loaddrv.exe start giveio'
    Pop $0                              ;return value/error/timeout
    IntCmp $0 0 0 ext_err ext_err       ;if not 0 -> error
    nsExec::ExecToLog '$INSTDIR\bin\loaddrv.exe starttype giveio auto'
    Pop $0                              ;return value/error/timeout
    IntCmp $0 0 0 ext_err ext_err       ;if not 0 -> error
    WriteRegStr HKLM SOFTWARE\mspgcc "giveio" "started"
    Goto ext_ok
ext_err:
    DetailPrint "Error while installing and starting giveio"
    MessageBox MB_OK|MB_ICONSTOP "Error while installing and starting giveio"
    Goto ext_ok
ext_here:
    DetailPrint "Installing giveio gave an error, assuming its already installed"
ext_ok:
SectionEnd

; special uninstall section.
Section "Uninstall"
    ; remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\mspgcc-pyjtag"
    DeleteRegKey HKLM SOFTWARE\NSIS_Example2
    ; remove files
    Delete $INSTDIR\bin\msp430-jtag.exe
    Delete $INSTDIR\bin\_parjtag.pyd
    Delete $INSTDIR\bin\jtag.py
    Delete $INSTDIR\bin\HIL.dll
    Delete $INSTDIR\bin\MSP430mspgcc.dll
    ;XXX python22.dll is left installed as it is used by pybsl and other tools
    Delete $INSTDIR\license-pyjtag.txt
    Delete $INSTDIR\readme-pyjtag.txt
    ; giveio
    ; if it was started by us, stop it
    ReadRegStr $0 HKLM SOFTWARE\mspgcc "giveio"
    StrCmp $0 '' no_giveio
    nsExec::ExecToLog  '$INSTDIR\bin\loaddrv.exe stop giveio'
    Pop $0                              ;return value/error/timeout
    IntCmp $0 0 0 giveio_err giveio_err ;if not 0 -> error
    nsExec::ExecToLog '$INSTDIR\bin\loaddrv.exe remove giveio'
    Pop $0                              ;return value/error/timeout
    IntCmp $0 0 0 giveio_err giveio_err ;if not 0 -> error
    Goto no_giveio
giveio_err:
    DetailPrint "Error while uninstalling giveio service"
    MessageBox MB_OK|MB_ICONSTOP "Error while uninstalling giveio service"
no_giveio:
    Delete loaddrv.exe
    Delete giveio.sys
    ; MUST REMOVE UNINSTALLER, too
    Delete $INSTDIR\uninstall-pyjtag.exe
SectionEnd
