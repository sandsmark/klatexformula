######################################################################
# Automatically generated by qmake (2.01a) Thu Dec 25 09:18:38 2008
######################################################################

TEMPLATE = app
TARGET = klatexformula
DEPENDPATH += . klfbackend
INCLUDEPATH += . klfbackend
CONFIG += qt debug
QT = core gui xml

DEFINES += KLFBACKEND_QT4
LIBS += -Lklfbackend -lklfbackend

# Input
HEADERS += klfcolorchooser.h \
           klfconfig.h \
           klfdata.h \
           klflatexsymbols.h \
           klflibrary.h \
           klfmainwin.h \
           klfpathchooser.h \
           klfsettings.h \
           klfstylemanager.h
FORMS += klflatexsymbolsui.ui \
         klflibrarybrowserui.ui \
         klfmainwinui.ui \
         klfprogerrui.ui \
         klfsettingsui.ui \
         klfstylemanagerui.ui
SOURCES += klfcolorchooser.cpp \
           klfconfig.cpp \
           klfdata.cpp \
           klflatexsymbols.cpp \
           klflibrary.cpp \
           klfmainwin.cpp \
           klfpathchooser.cpp \
           klfsettings.cpp \
           klfstylemanager.cpp \
           main.cpp
RESOURCES += klfres.qrc
