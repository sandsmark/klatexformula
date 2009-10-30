######################################################################
# Automatically generated by qmake (2.01a) Thu Dec 25 09:18:38 2008
######################################################################

VERSION = 3.1.0alpha1

# Warning: make install works only under Unix/Linux.
INSTALLPREFIX=$$[KLF_INSTALLPREFIX]
CP=cp
ICONTHEME=$$[KLF_ICONTHEME]

# ---

TEMPLATE = app
TARGET = klatexformula
DESTDIR = ..
DEPENDPATH += . klfbackend
INCLUDEPATH += . klfbackend
CONFIG += qt release
QT = core gui xml
!win32 {
	QT += dbus
}

DEFINES += KLFBACKEND_QT4 KLF_VERSION_STRING=\\\"$$VERSION\\\" KLF_SRC_BUILD
LIBS += -Lklfbackend -Lklfbackend/release -lklfbackend

# Input
HEADERS += klfcolorchooser.h \
           klfconfig.h \
           klfdata.h \
           klflatexsymbols.h \
           klflibrary.h \
           klfmainwin.h \
           klfpathchooser.h \
           klfsettings.h \
           klfstylemanager.h \
	   klfpluginiface.h \
	   qtcolortriangle.h
!win32 {
	HEADERS +=	klfdbus.h
}
FORMS += klflatexsymbolsui.ui \
         klflibrarybrowserui.ui \
         klfmainwinui.ui \
         klfprogerrui.ui \
         klfsettingsui.ui \
         klfstylemanagerui.ui \
	 klfaboutdialogui.ui \
	 klfcolorchoosewidgetui.ui \
	 klfcolordialogui.ui
SOURCES += klfcolorchooser.cpp \
           klfconfig.cpp \
           klfdata.cpp \
           klflatexsymbols.cpp \
           klflibrary.cpp \
           klfmainwin.cpp \
           klfpathchooser.cpp \
           klfsettings.cpp \
           klfstylemanager.cpp \
           main.cpp \
	   qtcolortriangle.cpp
!win32 {
	SOURCES +=	klfdbus.cpp
}
RESOURCES += klfres.qrc plugins/klfbaseplugindata.qrc
win32: RESOURCES += plugins/klfbaseplugins_win.qrc
unix: RESOURCES += plugins/klfbaseplugins_unix.qrc
macx: RESOURCES += plugins/klfbaseplugins_unix.qrc

!win32 {
	DEFINES += KLF_USE_DBUS
}

# For Mac OS X
ICON = klficon.icns

# For Windows
RC_FILE = klatexformula.rc

TRANSLATIONS += i18n/klf_fr.ts

# INSTALLS are UNIX-only.anyway
unix {
  klfcmdl.extra = ln -sf \"$$INSTALLPREFIX/bin/klatexformula\" \"$$INSTALLPREFIX/bin/klatexformula_cmdl\"
  klfcmdl.path = $$INSTALLPREFIX/bin
  target.path = $$INSTALLPREFIX/bin
  desktopfile.files = klatexformula.desktop
  desktopfile.path = $$INSTALLPREFIX/share/applications
  icon16.extra = install -m 644 -p  hi16-app-klatexformula.png $$INSTALLPREFIX/share/icons/$$ICONTHEME/16x16/apps/klatexformula.png
  icon16.path = $$INSTALLPREFIX/share/icons/$$ICONTHEME/16x16/apps
  icon32.extra = install -m 644 -p  hi32-app-klatexformula.png $$INSTALLPREFIX/share/icons/$$ICONTHEME/32x32/apps/klatexformula.png
  icon32.path = $$INSTALLPREFIX/share/icons/$$ICONTHEME/32x32/apps
  icon64.extra = install -m 644 -p  hi64-app-klatexformula.png $$INSTALLPREFIX/share/icons/$$ICONTHEME/64x64/apps/klatexformula.png
  icon64.path = $$INSTALLPREFIX/share/icons/$$ICONTHEME/64x64/apps
  icon128.extra = install -m 644 -p hi128-app-klatexformula.png $$INSTALLPREFIX/share/icons/$$ICONTHEME/128x128/apps/klatexformula.png
  icon128.path = $$INSTALLPREFIX/share/icons/$$ICONTHEME/128x128/apps
  INSTALLS += target klfcmdl desktopfile icon16 icon32 icon64 icon128
}
