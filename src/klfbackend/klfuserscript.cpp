/***************************************************************************
 *   file klfuserscript.cpp
 *   This file is part of the KLatexFormula Project.
 *   Copyright (C) 2012 by Philippe Faist
 *   philippe.faist at bluewin.ch
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
/* $Id$ */

#include <QFileInfo>

#include <klfdefs.h>
#include <klfdebug.h>
#include <klfdatautil.h>
#include <klfpobj.h>

#include "klfbackend.h"
#include "klfbackend_p.h"
#include "klfuserscript.h"


/** \page pageUserScript User Scripts
 *
 * \todo ......... WRITE DOC................
 *
 * Format list (e.g. for 'spitsOut()' and 'skipFormats()':
 * \todo FORMAT LIST
 *
 * \bug NEEDS TESTING: user script output formats, skipped formats, parameters, etc.
 */





/*
static int read_spec_section(const QString& str, int fromindex, const QRegExp& seprx, QString * extractedPart)
{
  int i = fromindex;
  bool in_quote = false;

  QString s;

  while (i < str.length() && (in_quote || seprx.indexIn(str, i) != i)) {
    if (str[i] == '\\') {
      s.append(str[i]);
      if (i+1 < str.length())
	s.append(str[i+1]);
      i += 2; // skip next char, too. The actual escaping will be done with klfEscapedToData()
      continue;
    }
    if (str[i] == '"') {
      in_quote = !in_quote;
      ++i;
      continue;
    }
    s.append(str[i]);
    ++i;
  }

  *extractedPart = QString::fromLocal8Bit(klfEscapedToData(s.toLocal8Bit()));

  return i; // the position of the last char separator
}

*/





struct KLFUserScriptInfo::Private : public KLFPropertizedObject
{
  Private() : KLFPropertizedObject("KLFUserScriptInfo")
  {
    refcount = 0;
    settings = NULL;
    scriptInfoError = KLFERR_NOERROR;

    registerBuiltInProperty(Category, QLatin1String("Category"));
    registerBuiltInProperty(Name, QLatin1String("Name"));
    registerBuiltInProperty(Author, QLatin1String("Author"));
    registerBuiltInProperty(Version, QLatin1String("Version"));
    registerBuiltInProperty(License, QLatin1String("License"));
    registerBuiltInProperty(KLFMinVersion, QLatin1String("KLFMinVersion"));
    registerBuiltInProperty(KLFMaxVersion, QLatin1String("KLFMaxVersion"));
    registerBuiltInProperty(SpitsOut, QLatin1String("SpitsOut"));
    registerBuiltInProperty(SkipFormats, QLatin1String("SkipFormats"));
  }

  int refcount;
  inline int ref() { return ++refcount; }
  inline int deref() { return --refcount; }

  KLFBackend::klfSettings *settings;

  QString fname;
  QString sname;
  int scriptInfoError;
  QString scriptInfoErrorString;

  QStringList notices;
  QStringList warnings;
  QStringList errors;

  //   QString category;
  //   QString name;
  //   QString author;
  //   QString version;
  //   QString license;
  //   QString klfMinVersion;
  //   QString klfMaxVersion;

  //   QStringList spitsOut;
  //   QStringList skipFormats;

  //   QMap<QString,QVariant> customInfos;

  //   QList<KLFUserScriptInfo::Param> paramList;

  void query_script_info()
  {
    scriptInfoError = 0;
    scriptInfoErrorString = QString();

    QByteArray scriptinfo;
    //    bool want_full_template = true;
    { // Query Script Info phase
      KLFBackendFilterProgram p(QObject::tr("User Script (ScriptInfo)"), settings);
      p.resErrCodes[KLFFP_NOSTART] = KLFERR_USERSCRIPT_NORUN;
      p.resErrCodes[KLFFP_NOEXIT] = KLFERR_USERSCRIPT_NONORMALEXIT;
      p.resErrCodes[KLFFP_NOSUCCESSEXIT] = KLFERR_PROGERR_USERSCRIPT;
      p.resErrCodes[KLFFP_NODATA] = KLFERR_USERSCRIPT_NOSCRIPTINFO;
      p.resErrCodes[KLFFP_DATAREADFAIL] = KLFERR_USERSCRIPT_NOSCRIPTINFO;
      
      p.setArgv(QStringList() << fname << "--scriptinfo" << KLF_VERSION_STRING);
      
      bool ok = p.run(QString(), &scriptinfo);
      if (!ok) {
	scriptInfoError = p.resultStatus();
	scriptInfoErrorString = p.resultErrorString();
	klfDbg("Failed to query scriptinfo: "<<scriptInfoErrorString) ;
	return;
      }
    }
    scriptinfo.replace("\r\n", "\n");
    if (scriptinfo.isEmpty()) {
      scriptinfo = "ScriptInfo\n";
    }
    klfDbg("got scriptinfo="<<scriptinfo) ;
    // parse scriptinfo
    if (!scriptinfo.startsWith("ScriptInfo\n")) {
      scriptInfoError = KLFERR_USERSCRIPT_INVALIDSCRIPTINFO;
      scriptInfoErrorString = QObject::tr("User script did not provide valid --scriptinfo output.");
      qWarning()<<KLF_FUNC_NAME<<": User script did not provide valid --scriptinfo (missing header line).";
      return;
    }
    QList<QByteArray> lines = scriptinfo.split('\n');
    lines.removeAt(0); // skip of course the 'ScriptInfo\n' line !
    int k;
    for (k = 0; k < lines.size(); ++k) {
      QString line = QString::fromLocal8Bit(lines[k]);
      if (line.trimmed().isEmpty())
	continue;
      //  Key: Value   or
      //  Key[specifiers]: Value
      //    Key is [-A-Za-z0-9_.]+
      //    specifiers (optional) is [-A-Za-z0-9_.,]*, intendend to be a list of comma-separated options
      //    value is anything until end of line. Use 'base64' specifier if you need to have newlines in the value
      //        and encode the value as base64.
      QRegExp rx("([-A-Za-z0-9_.]+)(?:\\[([-A-Za-z0-9_.,]*)\\])?:\\s*(.*)");
      QRegExp boolrxtrue("(t(rue)?|y(es)?|on|1)");
      if (!rx.exactMatch(line)) {
	qWarning()<<KLF_FUNC_NAME<<": User script did not provide valid --scriptinfo.\nCannot parse line: "<<line;
	scriptInfoError = KLFERR_USERSCRIPT_INVALIDSCRIPTINFO;
	scriptInfoErrorString = QObject::tr("User script provided invalid --scriptinfo output.", "KLFBackend");
	return;
      }
      QString key = rx.cap(1);
      QString specstr = rx.cap(2);
      QStringList specs = specstr.split(',');
      QString val = rx.cap(3).trimmed();
      if (val.isEmpty())
	val = QString(); // empty value is null string
      if (specs.contains(QLatin1String("base64"))) {
	val = QString::fromLocal8Bit(QByteArray::fromBase64(val.toLatin1()));
      }
      klfDbg("key="<<key<<", value="<<val) ;
      if (key == QLatin1String("Notice")) {
	// emit a notice.
	notices << val;
	klfDbg("User script notice: "<< val) ;
      } else if (key == QLatin1String("Warning")) {
	// emit a warning.
	warnings << val;
	klfWarning("User script warning: "<< val) ;
      } else if (key == QLatin1String("Error")) {
	// emit an error.
	errors << val;
	klfWarning("User script error: "<<val) ;
      }
      // now parse and set the property
      if (key == QLatin1String("Category")) {
	setProperty(Category, val);
	klfDbg("Read category: "<<property(Category)) ;
      } else if (key == QLatin1String("Name")) {
	setProperty(Name, val);
	klfDbg("Read name: "<<property(Name)) ;
      } else if (key == QLatin1String("Author") || key == QLatin1String("Authors")) {
	setProperty(Author, property(Author).toStringList() + (QStringList()<<val));
	klfDbg("Read (cumulated) author: "<<property(Author)) ;
      } else if (key == QLatin1String("Version")) {
	setProperty(Version, val);
	klfDbg("Read version: "<<property(Version)) ;
      } else if (key == QLatin1String("License")) {
	setProperty(License, val);
      } else if (key == QLatin1String("KLFMinVersion")) {
	setProperty(KLFMinVersion, val);
	klfDbg("Read klfMinVersion: "<<property(KLFMinVersion)) ;
      } else if (key == QLatin1String("KLFMaxVersion")) {
	setProperty(KLFMaxVersion, val);
	klfDbg("Read klfMaxVersion: "<<property(KLFMaxVersion)) ;
      } else if (key == QLatin1String("SpitsOut")) {
	setProperty(SpitsOut, property(SpitsOut).toStringList() + val.split(QRegExp("(,|\\s+)")));
      } else if (key == QLatin1String("SkipFormats")) {
	setProperty(SkipFormats, property(SkipFormats).toStringList() + val.split(QRegExp("(,|\\s+)")));
      } /*
	  else if (key == QLatin1String("Param")) {
	  // parse a paramter request specification, eg.
	  // Param: USE_PDF;bool;"Use PDF?";"Generate PDF; or rather prefer old-fashioned PS?"
	  int k;
	  KLFUserScriptInfo::Param param;
	  QRegExp seprx(";");
	  // read name
	  k = read_spec_section(val, 0, seprx, &param.name);
	  // now read type
	  QString typstring;
	  k = read_spec_section(val, k+1, seprx, &typstring);
	  // read title
	  k = read_spec_section(val, k+1, seprx, &param.title);
	  // and description
	  k = read_spec_section(val, k+1, seprx, &param.description);

	  // decode type
	  QString typname;
	  int j;
	  QRegExp enumsep(":");
	  j = read_spec_section(typstring, 0, enumsep, &typname);
	  typname = typname.trimmed().toLower();
	  if (typname == "enum") {
	  param.type = KLFUserScriptInfo::Param::Enum;
	  while (j < typstring.length()) {
	  QString enumstr;
	  j = read_spec_section(typstring, j+1, enumsep, &enumstr);
	  if (!enumstr.isEmpty())
	  param.type_enums << enumstr;
	  }
	  } else if (typname == "bool") {
	  param.type = KLFUserScriptInfo::Param::Bool;
	  } else if (typname == "int") {
	  param.type = KLFUserScriptInfo::Param::Int;
	  } else {
	  if (typname != "string") {
	  klfWarning("interpreting unknown type name "<<typname<<" as 'string' type") ;
	  }
	  param.type = KLFUserScriptInfo::Param::String;
	  }

	  // and add this to our param list
	  paramList << param;
	  }
	*/ 
      else {
	klfDbg("Custom userscript info key: "<<key<<", value="<<val);
	QVariant v = QVariant(val);
	// if the key is already present, morph the stored value into the first item of a variantlist and append
	// the new data.
	if (hasPropertyValue(key) && property(key).type() == QVariant::List) {
	  setProperty(key, QVariantList() << property(key).toList() << v);
	} else if (hasPropertyValue(key)) {
	  setProperty(key, QVariantList() << property(key) << v);
	} else {
	  setProperty(key, v);
	}
      }
    }
  }


  static QMap<QString,KLFRefPtr<Private> > userScriptInfoCache;
  
private:
  /* no copy constructor */
  Private(const Private& /*other*/) : KLFPropertizedObject("KLFUserScriptInfo") { }
};


// static
QMap<QString,KLFRefPtr<KLFUserScriptInfo::Private> > KLFUserScriptInfo::Private::userScriptInfoCache;

// static
void KLFUserScriptInfo::clearCacheAll()
{
  // will decrease the refcounts if needed automatically (KLFRefPtr)
  Private::userScriptInfoCache.clear();
}


// static
bool KLFUserScriptInfo::hasScriptInfoInCache(const QString& scriptFileName)
{
  QString normalizedfn = QFileInfo(scriptFileName).canonicalFilePath();
  return Private::userScriptInfoCache.contains(normalizedfn);
}

KLFUserScriptInfo::KLFUserScriptInfo(const QString& scriptFileName, KLFBackend::klfSettings * settings)
{
  QString normalizedfn = QFileInfo(scriptFileName).canonicalFilePath();
  if (Private::userScriptInfoCache.contains(normalizedfn)) {
    d = Private::userScriptInfoCache[normalizedfn];
  } else {
    d = new KLFUserScriptInfo::Private;

    d()->settings = settings;
    d()->fname = scriptFileName;
    d()->sname = QFileInfo(scriptFileName).fileName();

    KLF_ASSERT_NOT_NULL(settings, "Given NULL settings pointer! The KLFUserScript will not be initialized!", ; ) ;
    
    if (d()->settings != NULL) {
      d()->query_script_info();
      Private::userScriptInfoCache[normalizedfn] = d;
    }
  }
}

KLFUserScriptInfo::KLFUserScriptInfo(const KLFUserScriptInfo& copy)
  : KLFAbstractPropertizedObject()
{
  // will increase the refcount (thanks to KLFRefPtr)
  d = copy.d;
}

KLFUserScriptInfo::~KLFUserScriptInfo()
{
  d.setNull(); // will delete the data if refcount reaches zero (see KLFRefPtr)
}

QString KLFUserScriptInfo::fileName() const
{
  return d()->fname;
}
QString KLFUserScriptInfo::scriptName() const
{
  return d()->sname;
}

int KLFUserScriptInfo::scriptInfoError() const
{
  return d()->scriptInfoError;
}
QString KLFUserScriptInfo::scriptInfoErrorString() const
{
  return d()->scriptInfoErrorString;
}


QString KLFUserScriptInfo::category() const { return info(Category).toString(); }
QString KLFUserScriptInfo::name() const { return info(Name).toString(); }
QString KLFUserScriptInfo::author() const { return info(Author).toStringList().join("; "); }
QStringList KLFUserScriptInfo::authorList() const { return info(Author).toStringList(); }
QString KLFUserScriptInfo::version() const { return info(Version).toString(); }
QString KLFUserScriptInfo::license() const { return info(License).toString(); }
QString KLFUserScriptInfo::klfMinVersion() const { return info(KLFMinVersion).toString(); }
QString KLFUserScriptInfo::klfMaxVersion() const { return info(KLFMaxVersion).toString(); }
QStringList KLFUserScriptInfo::spitsOut() const { return info(SpitsOut).toStringList(); }
QStringList KLFUserScriptInfo::skipFormats() const { return info(SkipFormats).toStringList(); }


// KLF_DEFINE_PROPERTY_GET(KLFUserScriptInfo, QList<KLFUserScriptInfo::Param>, paramList) ;

bool KLFUserScriptInfo::hasNotices() const
{
  return d->notices.size();
}
bool KLFUserScriptInfo::hasWarnings() const
{
  return d->warnings.size();
}
bool KLFUserScriptInfo::hasErrors() const
{
  return d->errors.size();
}

KLF_DEFINE_PROPERTY_GET(KLFUserScriptInfo, QStringList, notices) ;

KLF_DEFINE_PROPERTY_GET(KLFUserScriptInfo, QStringList, warnings) ;

KLF_DEFINE_PROPERTY_GET(KLFUserScriptInfo, QStringList, errors) ;



QVariant KLFUserScriptInfo::info(int propId) const
{
  return d()->property(propId);
}

QVariant KLFUserScriptInfo::info(const QString& field) const
{
  KLF_DEBUG_BLOCK(KLF_FUNC_NAME) ;
  QString x = field;

  if (x == QLatin1String("Authors"))
    x = QLatin1String("Author");

  klfDbg("x="<<x) ;
  return info(d()->propertyIdForName(x));
}

QStringList KLFUserScriptInfo::infosList() const
{
  return d()->propertyNameList();
}

QString KLFUserScriptInfo::objectKind() const { return d()->objectKind(); }

// protected. Used by eg. KLFExportTypeUserScriptInfo to normalize list property values.
void KLFUserScriptInfo::internalSetProperty(const QString& key, const QVariant &val)
{
  d()->setProperty(key, val);
}

const KLFPropertizedObject * KLFUserScriptInfo::pobj()
{
  return d();
}






// ----------------------------------------

struct KLFUserScriptFilterProcessPrivate
{
  KLF_PRIVATE_HEAD(KLFUserScriptFilterProcess)
  {
    usinfo = NULL;
  }

  KLFUserScriptInfo * usinfo;
};

KLFUserScriptFilterProcess::KLFUserScriptFilterProcess(const QString& scriptFileName,
						       KLFBackend::klfSettings * settings)
  : KLFFilterProcess("User Script " + scriptFileName, settings)
{
  KLF_DEBUG_BLOCK(KLF_FUNC_NAME);
  klfDbg("scriptFileName= "<<scriptFileName) ;

  d = KLF_INIT_PRIVATE(KLFUserScriptFilterProcess) ;

  d->usinfo = new KLFUserScriptInfo(scriptFileName, settings);

  setArgv(QStringList() << d->usinfo->fileName());
}


KLFUserScriptFilterProcess::~KLFUserScriptFilterProcess()
{
  delete d->usinfo;
  KLF_DELETE_PRIVATE ;
}
