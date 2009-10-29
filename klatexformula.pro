######################################################################
# Automatically generated by qmake (2.01a) Sat Jan 3 12:33:46 2009
######################################################################

# --------------------------------------------------------------------
# Variables you may pass to qmake to configure build environment:
# --------------------------------------------------------------------
# QMAKE
#   qmake program (in $PATH or with explicit path). Defaults to 'qmake'
# BACKEND_ONLY
#   if non-empty, will build only the backend library (and thus not the GUI program)
# BACKEND_INSTALLPREFIX
#   prefix where to install the backend library with 'make install'. Defaults to
#   contents of INSTALLPREFIX
# BACKEND_LIBDIR
#   'lib' or 'lib64'. Defaults to 'lib', or 'lib64' if (uname -m) contains '64'
# BACKEND_SHAREDORSTATIC
#   'staticlib' or 'dll' for static or shared library. Defaults to 'staticlib'
# BACKEND_USE_QT4
#   'true' or 'false'. This variable exists because I'm lazy and don't want to
#   parse the contents of $$QT_VERSION. Note: We MUST be using Qt4 if we're building
#   the GUI too. Defaults to Qt4 ('true').
# INSTALLPREFIX
#   prefix to install GUI application, desktop files, icons, etc. Defaults to /usr
# ICONTHEME
#   the icon theme to add our icons to. Defaults to 'hicolor'
# ------------------------------------------------------------------
# To change the value of a variable, pass it in qmake command line:
#  > qmake-qt4  QMAKE=qmake-qt4  INSTALLPREFIX=/usr/local
# Note that the KLF_ prefix must be omitted.
# ------------------------------------------------------------------


VERSION = 3.1.0alpha0

TEMPLATE = subdirs
CONFIG += ordered release


# SET DEFAULT VALUES

isEmpty(QMAKE) {
  QMAKE = qmake
}
isEmpty(INSTALLPREFIX) {
  INSTALLPREFIX=/usr
}
#isEmpty(BACKEND_ONLY) {
#}
isEmpty(BACKEND_INSTALLPREFIX) {
  BACKEND_INSTALLPREFIX = $$INSTALLPREFIX
}
isEmpty(BACKEND_LIBDIR) {
  BACKEND_LIBDIR = lib
  unix:contains($$system(uname -m), 64) {
    BACKEND_LIBDIR = lib64
  }
  macx:contains($$system(uname -m), 64) {
    BACKEND_LIBDIR = lib64
  }
}
isEmpty(BACKEND_SHAREDORSTATIC) {
  # may be staticlib or dll
  BACKEND_SHAREDORSTATIC = staticlib
}
isEmpty(BACKEND_USE_QT4) {
  BACKEND_USE_QT4 = true
}
isEmpty(ICONTHEME) {
  ICONTHEME = hicolor
}


#win32 {
#} else {
  # -- Test for $$QMAKE
  contains(BACKEND_USE_QT4, false) {
    # using Qt3 -- 'system' succeeds if return value == 1
    system($$QMAKE -version) {
      error(qmake program `$$QMAKE\' not found !)
    }
  } else {
    # using Qt4 -- 'system' succeeds if zero return value
    !system($$QMAKE -version) {
      error(qmake program `$$QMAKE\' not found !)
    }
  }
#}


# -- Store KLF configuration into QMake environment variables

system($$QMAKE -set KLF_QMAKE '$$QMAKE')
system($$QMAKE -set KLF_BACKEND_ONLY '$$BACKEND_ONLY')
system($$QMAKE -set KLF_BACKEND_INSTALLPREFIX '$$BACKEND_INSTALLPREFIX')
system($$QMAKE -set KLF_BACKEND_LIBDIR '$$BACKEND_LIBDIR')
system($$QMAKE -set KLF_BACKEND_SHAREDORSTATIC '$$BACKEND_SHAREDORSTATIC')
system($$QMAKE -set KLF_BACKEND_USE_QT4 '$$BACKEND_USE_QT4')
system($$QMAKE -set KLF_ICONTHEME '$$ICONTHEME')
system($$QMAKE -set KLF_INSTALLPREFIX '$$INSTALLPREFIX')

SUBDIRS = src/klfbackend
isEmpty(BACKEND_ONLY) {
  SUBDIRS += src src/plugins
}

message(Will build the following subdirs: $$SUBDIRS)
message(With the following options: (only those beginning with KLF_ are relevant:))
!win32:system($$QMAKE -query | egrep '^KLF_')
win32:system($$QMAKE -query)
message(You may now run `make\' or re-run qmake with other options to adjust above settings.)
message(NOTE: option names for qmake command line do NOT take the leading KLF_ prefix.)
message(Please look at the top of klatexformula.pro file for more information on options.)

