;---------------------------------------------
;
; File : gmic_gimp2.10_win64.iss
;
; Description : Inno Setup Script to create
;               a Windows installer for
;               G'MIC for GIMP.
;
; Authors : François Collard
;           David Tschumperlé
;
;---------------------------------------------

#define AppName "G'MIC-Qt for GIMP"

[Setup]
AppName={#AppName}
AppVersion=XXX
AppPublisherURL=https://gmic.eu/
DefaultDirName={userappdata}\GIMP\2.10\plug-ins\gmic_gimp_qt\
DefaultGroupName={#AppName}
UninstallDisplayIcon={app}\gmic_gimp_qt.exe
LicenseFile={#file AddBackslash(SourcePath) + "CeCILL.rtf"}
OutputDir={#SourcePath}
UninstallFilesDir={app}\uninst
AppendDefaultDirName=false
UsePreviousAppDir=true
DirExistsWarning=no
WizardImageFile=gmic_instimg.bmp
WizardSmallImageFile=gmic_instimg_small.bmp
PrivilegesRequired=lowest
OutputBaseFilename=gmic_XXX_gimp2.10_win64

[Files]
Source: build64-gimp\gmic_gimp_qt.exe; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\platforms\qdirect2d.dll; DestDir: {app}\platforms
Source: build64-gimp\platforms\qminimal.dll; DestDir: {app}\platforms
Source: build64-gimp\platforms\qoffscreen.dll; DestDir: {app}\platforms
Source: build64-gimp\platforms\qwebgl.dll; DestDir: {app}\platforms
Source: build64-gimp\platforms\qwindows.dll; DestDir: {app}\platforms
Source: build64-gimp\styles\qwindowsvistastyle.dll; DestDir: {app}\styles
Source: build64-gimp\updatexxx.gmic; DestDir: {userappdata}\gmic; Flags: ignoreversion
Source: build64-gimp\gmic_cluts.gmz; DestDir: {userappdata}\gmic; Flags: ignoreversion
Source: build64-gimp\gmic_denoise_cnn.gmz; DestDir: {userappdata}\gmic; Flags: ignoreversion

Source: build64-gimp\Qt5Core.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\Qt5Gui.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\Qt5Network.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\Qt5Widgets.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\exchndl.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libbabl-0.1-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libbrotlicommon.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libbrotlidec.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libbz2-1.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libcairo-2.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libcrypto-1_1-x64.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libcurl-4.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libdouble-conversion.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libexiv2.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libexpat-1.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libffi-7.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libfftw3-3.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libfontconfig-1.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libfreetype-6.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgcc_s_seh-1.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgdk_pixbuf-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgegl-0.4-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgexiv2-2.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgimp-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgimpbase-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgimpcolor-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgimpconfig-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgimpmath-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgio-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libglib-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgmodule-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgobject-2.0-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgomp-1.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libgraphite2.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libharfbuzz-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libiconv-2.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libicudt69.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libicuin69.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libicuuc69.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libidn2-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libintl-8.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\liblcms2-2.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libmd4c.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libnghttp2-14.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libpcre-1.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libpcre2-16-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libpixman-1-0.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libpng16-16.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libjpeg-8.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libpsl-5.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libssh2-1.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libssl-1_1-x64.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libstdc++-6.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libunistring-2.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libwinpthread-1.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\libzstd.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\mgwhelp.dll; DestDir: {app}; Flags: ignoreversion
Source: build64-gimp\zlib1.dll; DestDir: {app}; Flags: ignoreversion




;[Icons]
;Name: {userstartmenu}\Gimp\Gmic_Gimp\Uninstall Gmic_Gimp; Filename: {uninstallexe}

[Languages]
Name: French; MessagesFile: compiler:Languages\French.isl
Name: English; MessagesFile: compiler:Default.isl
Name: Czech; MessagesFile: compiler:Languages\Czech.isl
Name: Danish; MessagesFile: compiler:Languages\Danish.isl
Name: Dutch; MessagesFile: compiler:Languages\Dutch.isl
Name: Finnish; MessagesFile: compiler:Languages\Finnish.isl
Name: German; MessagesFile: compiler:Languages\German.isl
Name: Hebrew; MessagesFile: compiler:Languages\Hebrew.isl
Name: Italian; MessagesFile: compiler:Languages\Italian.isl
Name: Norwegian; MessagesFile: compiler:Languages\Norwegian.isl
Name: Polish; MessagesFile: compiler:Languages\Polish.isl
Name: Portuguese; MessagesFile: compiler:Languages\Portuguese.isl
Name: Russian; MessagesFile: compiler:Languages\Russian.isl
Name: Slovenian; MessagesFile: compiler:Languages\Slovenian.isl
Name: Spanish; MessagesFile: compiler:Languages\Spanish.isl
