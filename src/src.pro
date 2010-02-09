######################################################################
# QMake Project File for klatexformula main source library (internal)
######################################################################
# $Id$
######################################################################

include(../VERSION.pri)

# ---


SHAREDORSTATIC = $$[KLF_SRCLIB_SHAREDORSTATIC]

TEMPLATE = lib
TARGET = klfsrc
DESTDIR = .
DEPENDPATH += . klfbackend
INCLUDEPATH += . klfbackend
CONFIG += qt $$SHAREDORSTATIC debug
# release
QT = core gui xml sql
!win32 {
	QT += dbus
	DEFINES += KLF_USE_DBUS
}

DEFINES += KLFBACKEND_QT4 KLF_VERSION_STRING=\\\"$$VERSION\\\" KLF_SRC_BUILD KLF_DEBUG_TIME_PRINT
LIBS += -Lklfbackend -Lklfbackend/release -lklfbackend

# Input
HEADERS += klfcolorchooser.h \
           klfconfig.h \
           klfdata.h \
	   klfpobj.h \
           klflatexsymbols.h \
           klflibrary.h \
	   klflib.h \
	   klflibview.h \
	   klflibentryeditor.h \
	   klflibbrowser.h \
	   klfmime.h \
           klfmainwin.h \
           klfpathchooser.h \
           klfsettings.h \
           klfstylemanager.h \
	   klfpluginiface.h \
	   qtcolortriangle.h \
	   klfmain.h \
	   klfdisplaylabel.h
!win32 {
	HEADERS +=	klfdbus.h
}
FORMS += klflatexsymbolsui.ui \
         klflibrarybrowserui.ui \
	 klflibbrowser.ui \
	 klflibentryeditor.ui \
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
	   klfpobj.cpp \
           klflatexsymbols.cpp \
           klflibrary.cpp \
	   klflib.cpp \
	   klflibview.cpp \
	   klflibentryeditor.cpp \
	   klflibbrowser.cpp \
	   klfmime.cpp \
           klfmainwin.cpp \
           klfpathchooser.cpp \
           klfsettings.cpp \
           klfstylemanager.cpp \
	   qtcolortriangle.cpp \
	   klfmain.cpp \
	   klfdisplaylabel.cpp
win32 {
	SOURCES +=      klfwinclipboard.cpp
}
!win32 {
	SOURCES +=	klfdbus.cpp
}

#TRANSLATIONS += i18n/klf_fr.ts
