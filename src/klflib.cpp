/***************************************************************************
 *   file klflib.cpp
 *   This file is part of the KLatexFormula Project.
 *   Copyright (C) 2010 by Philippe Faist
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

#include <QDebug>
#include <QString>
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "klfmain.h" // KLF_DB_VERSION
#include "klflib_p.h"
#include "klflib.h"


KLFLibEntry::KLFLibEntry(const QString& latex, const QDateTime& dt, const QImage& preview,
			 const QString& category, const QString& tags, const KLFStyle& style)
  : KLFPropertizedObject("KLFLibEntry")
{
  initRegisteredProperties();
  setLatex(latex);
  setDateTime(dt);
  setPreview(preview);
  setCategory(category);
  setTags(tags);
  setStyle(style);
}
KLFLibEntry::KLFLibEntry(const KLFLibEntry& copy)
  : KLFPropertizedObject("KLFLibEntry")
{
  initRegisteredProperties();
  setAllProperties(copy.allProperties());
}
KLFLibEntry::~KLFLibEntry()
{
}



// private
void KLFLibEntry::initRegisteredProperties()
{
  KLF_FUNC_SINGLE_RUN ;
  
  registerBuiltInProperty(Latex, "Latex");
  registerBuiltInProperty(DateTime, "DateTime");
  registerBuiltInProperty(Preview, "Preview");
  registerBuiltInProperty(Category, "Category");
  registerBuiltInProperty(Tags, "Tags");
  registerBuiltInProperty(Style, "Style");
}


// ---------------------------------------------------


KLFLibResourceEngine::KLFLibResourceEngine(const QUrl& url, uint featureflags,
					   QObject *parent)
  : QObject(parent), KLFPropertizedObject("KLFLibResourceEngine"), pUrl(url),
    pFeatureFlags(featureflags), pReadOnly(false)
{
  initRegisteredProperties();

  //  qDebug()<<"KLFLibResourceEngine::KLFLibResourceEngine("<<url<<","<<pFeatureFlags<<","
  //	  <<parent<<")";

  QStringList rdonly = pUrl.allQueryItemValues("klfReadOnly");
  if (rdonly.size() && rdonly.last() == "true") {
    if (pFeatureFlags & FeatureReadOnly)
      pReadOnly = true;
  }
  pUrl.removeAllQueryItems("klfReadOnly");

}
KLFLibResourceEngine::~KLFLibResourceEngine()
{
}

void KLFLibResourceEngine::initRegisteredProperties()
{
  KLF_FUNC_SINGLE_RUN

  registerBuiltInProperty(PropTitle, "Title");
  registerBuiltInProperty(PropLocked, "Locked");
  registerBuiltInProperty(PropViewType, "ViewType");
}

bool KLFLibResourceEngine::canModifyData(ModifyType /*modifytype*/) const
{
  return !isReadOnly() && !locked();
}
bool KLFLibResourceEngine::canModifyProp(int propId) const
{
  if (propId == PropLocked)
    return !isReadOnly() && (pFeatureFlags & FeatureLocked);

  return !isReadOnly() && !locked();
}
bool KLFLibResourceEngine::canRegisterProperty(const QString& /*propName*/) const
{
  return false;
}

bool KLFLibResourceEngine::setTitle(const QString& title)
{
  return setResourceProperty(PropTitle, title);
}
bool KLFLibResourceEngine::setLocked(bool setlocked)
{
  // if locked feature is supported, setResourceProperty().
  // immediately return FALSE otherwise.
  if (pFeatureFlags & FeatureLocked) {
    return setResourceProperty(PropLocked, setlocked);
  }
  return false;
}

bool KLFLibResourceEngine::setViewType(const QString& viewType)
{
  return setResourceProperty(PropViewType, viewType);
}

bool KLFLibResourceEngine::setReadOnly(bool readonly)
{
  if ( !(pFeatureFlags & FeatureReadOnly) )
    return false;

  pReadOnly = readonly;
  return true;
}


KLFLibResourceEngine::entryId KLFLibResourceEngine::insertEntry(const KLFLibEntry& entry)
{
  QList<entryId> ids = insertEntries(KLFLibEntryList() << entry);
  if (ids.size() == 0)
    return -1;

  return ids[0];
}

bool KLFLibResourceEngine::saveAs(const QUrl&)
{
  // not permitted by default. Subclasses must reimplement
  // to support this feature.
  return false;
}

QVariant KLFLibResourceEngine::resourceProperty(const QString& name) const
{
  return KLFPropertizedObject::property(name);
}

bool KLFLibResourceEngine::loadResourceProperty(const QString& propName, const QVariant& value)
{
  int propId = propertyIdForName(propName);
  if (propId < 0) {
    if (!canRegisterProperty(propName))
      return false;
    // register the property
    propId = registerProperty(propName);
    if (propId < 0)
      return false;
  }
  // finally set the property
  return setResourceProperty(propId, value);
}

bool KLFLibResourceEngine::setResourceProperty(int propId, const QVariant& value)
{
  if ( ! KLFPropertizedObject::propertyIdRegistered(propId) )
    return false;

  if ( !canModifyProp(propId) ) {
    return false;
  }

  bool result = saveResourceProperty(propId, value);
  if (!result) // operation not permitted or failed
    return false;

  // operation succeeded: set KLFPropertizedObject-based property.
  KLFPropertizedObject::setProperty(propId, value);
  emit resourcePropertyChanged(propId);
  return true;
}


// -----

QDataStream& operator<<(QDataStream& stream, const KLFLibResourceEngine::KLFLibEntryWithId& entrywid)
{
  return stream << entrywid.id << entrywid.entry;
}
QDataStream& operator>>(QDataStream& stream, KLFLibResourceEngine::KLFLibEntryWithId& entrywid)
{
  return stream >> entrywid.id >> entrywid.entry;
}


// ---------------------------------------------------


QList<KLFLibEngineFactory*> KLFLibEngineFactory::pRegisteredFactories =
	 QList<KLFLibEngineFactory*>();

KLFLibEngineFactory::KLFLibEngineFactory(QObject *parent)
  : QObject(parent)
{
  registerFactory(this);
}
KLFLibEngineFactory::~KLFLibEngineFactory()
{
  unRegisterFactory(this);
}

bool KLFLibEngineFactory::canCreateResource(const QString& /*scheme*/) const
{
  return false;
}

QWidget * KLFLibEngineFactory::createPromptCreateParametersWidget(QWidget */*parent*/,
									  const QString& /*scheme*/,
									  const Parameters& /*par*/)
{
  return NULL;
}
KLFLibEngineFactory::Parameters
/* */ KLFLibEngineFactory::retrieveCreateParametersFromWidget(const QString& /*scheme*/,
								      QWidget */*parent*/)
{
  return Parameters();
}
KLFLibResourceEngine *KLFLibEngineFactory::createResource(const QString& /*scheme*/,
								  const Parameters& /*param*/,
								  QObject */*parent*/)
{
  return NULL;
}

bool KLFLibEngineFactory::canResourceSaveAs(const QString& /*scheme*/) const
{
  return false;
}
QWidget *KLFLibEngineFactory::createPromptSaveAsWidget(QWidget */*parent*/,
						       const QString& /*scheme*/,
						       const QUrl& /*defaultUrl*/)
{
  return NULL;
}
QUrl KLFLibEngineFactory::retrieveSaveAsUrlFromWidget(const QString& /*scheme*/,
						      QWidget */*widget*/)
{
  return QUrl();
}





KLFLibEngineFactory *KLFLibEngineFactory::findFactoryFor(const QString& urlScheme)
{
  int k;
  // walk registered factories, and return the first that supports this scheme.
  for (k = 0; k < pRegisteredFactories.size(); ++k) {
    if (pRegisteredFactories[k]->supportedSchemes().contains(urlScheme))
      return pRegisteredFactories[k];
  }
  // no factory found
  return NULL;
}

QStringList KLFLibEngineFactory::allSupportedSchemes()
{
  QStringList schemes;
  int k;
  for (k = 0; k < pRegisteredFactories.size(); ++k) {
    schemes << pRegisteredFactories[k]->supportedSchemes();
  }
  return schemes;
}

void KLFLibEngineFactory::registerFactory(KLFLibEngineFactory *factory)
{
  if (pRegisteredFactories.indexOf(factory) != -1)
    return;
  pRegisteredFactories.append(factory);
}

void KLFLibEngineFactory::unRegisterFactory(KLFLibEngineFactory *factory)
{
  if (pRegisteredFactories.indexOf(factory) == -1)
    return;
  pRegisteredFactories.removeAll(factory);
}




// ---------------------------------------------------



static QByteArray image_data(const QImage& img, const char *format)
{
  QByteArray data;
  QBuffer buf(&data);
  buf.open(QIODevice::WriteOnly);
  img.save(&buf, format);
  buf.close();
  return data;
}

template<class T>
static QByteArray metatype_to_data(const T& object)
{
  QByteArray data;
  {
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << object;
    // force close of buffer in destroying stream
  }
  return data;
}

template<class T>
static T metatype_from_data(const QByteArray& data)
{
  T object;
  QDataStream stream(data);
  stream >> object;
  return object;
}





// static
KLFLibDBEngine * KLFLibDBEngine::openUrl(const QUrl& url, QObject *parent)
{

  QString datatablename = url.queryItemValue("dataTableName");
  if (datatablename.isEmpty()) {
    return NULL;
  }

  QSqlDatabase db;
  if (url.scheme() == "klf+sqlite") {
    db = QSqlDatabase::addDatabase("QSQLITE", url.toString());
    db.setDatabaseName(url.path());
  } else {
    qWarning("KLFLibDBEngine::KLFLibDBEngine: bad url scheme in URL\n\t%s",
	     qPrintable(url.toString()));
    return NULL;
  }

  if ( !db.open() || db.lastError().isValid() ) {
    QMessageBox::critical(0, tr("Error"),
			  tr("Unable to open library file %1 (engine: %2).\nError: %3")
			  .arg(url.path(), db.driverName(), db.lastError().text()), QMessageBox::Ok);
    return NULL;
  }

  return new KLFLibDBEngine(db, datatablename, true /*autoDisconnect*/, url, parent);
}

// static
KLFLibDBEngine * KLFLibDBEngine::createSqlite(const QString& fileName, const QString& dtablename,
					      QObject *parent)
{
  QString datatablename = dtablename;
  bool r;

  if (QFile::exists(fileName)) {
    // fail; we want to _CREATE_ a database. use openUrl() to open an existing DB.
    return NULL;
  }
  QUrl url = QUrl::fromLocalFile(fileName);
  url.setScheme("klf+sqlite");

  if (datatablename.isEmpty())
    datatablename = "klfentries";
  else
    url.addQueryItem("dataTableName", datatablename);

  QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", url.toString());
  db.setDatabaseName(url.path());

  r = db.open(); // go open (here create) the DB
  if ( !r || db.lastError().isValid() ) {
    QMessageBox::critical(0, tr("Error"),
			  tr("Unable to create library file %1 (SQLITE database):\n"
			     "%2")
			  .arg(url.path(), db.lastError().text()), QMessageBox::Ok);
    return NULL;
  }

  r = initFreshDatabase(db, datatablename);
  if ( !r ) {
    QMessageBox::critical(0, tr("Error"),
			  tr("Unable to initialize the SQLITE database file %1!").arg(url.path()));
    return NULL;
  }

  return new KLFLibDBEngine(db, datatablename, true /*autoDisconnect*/, url, parent);
}

// private
KLFLibDBEngine::KLFLibDBEngine(const QSqlDatabase& db, const QString& tablename, bool autodisconnect,
			       const QUrl& url, QObject *parent)
  : KLFLibResourceEngine(url, FeatureReadOnly|FeatureLocked, parent), pAutoDisconnectDB(autodisconnect)
{
  pDataTableName = "t_"+tablename;

  setDatabase(db);
  // read resource properties from database
  QSqlQuery q = QSqlQuery(pDB);
  q.prepare("SELECT name,value FROM klf_properties");
  q.exec();
  while (q.next()) {
    QString propname = q.value(0).toString();
    QVariant propvalue = q.value(1);
    qDebug()<<"Setting property `"<<propname<<"' to `"<<propvalue<<"'";
    if (!propertyNameRegistered(propname)) {
      if (!canRegisterProperty(propname))
	continue;
      KLFPropertizedObject::registerProperty(propname);
    }
    KLFPropertizedObject::setProperty(propname, propvalue);
  }
}

KLFLibDBEngine::~KLFLibDBEngine()
{
  if (pAutoDisconnectDB) {
    pDB.close();
    QSqlDatabase::removeDatabase(pDB.connectionName());
  }
}

bool KLFLibDBEngine::canModifyData(ModifyType mt) const
{
  if ( !KLFLibResourceEngine::canModifyData(mt) )
    return false;

  /** \todo TODO: check if file is writable (SQLITE3), if permissions on the database
   * are granted (MySQL, PgSQL, etc.) */
  return true;
}

bool KLFLibDBEngine::canModifyProp(int propId) const
{
  return KLFLibResourceEngine::canModifyProp(propId);
}
bool KLFLibDBEngine::canRegisterProperty(const QString& /*propName*/) const
{
  // we can register properties if we can write them
  return canModifyProp(-1);
}

bool KLFLibDBEngine::validDatabase() const
{
  return pDB.isOpen();
}

void KLFLibDBEngine::setDatabase(const QSqlDatabase& db)
{
  pDB = db;
}

// private
bool KLFLibDBEngine::saveResourceProperty(int propId, const QVariant& value)
{
  QString propName = propertyNameForId(propId);
  if ( propName.isEmpty() )
    return false;

  {
    QSqlQuery q = QSqlQuery(pDB);
    q.prepare("DELETE FROM klf_properties WHERE name = ?");
    q.addBindValue(propName);
    bool r = q.exec();
    if ( !r || q.lastError().isValid() ) {
      qWarning()<<"KLFLibDBEngine::setRes.Property("<<propId<<","<<value<<"): can't DELETE!\n\t"
		<<q.lastError().text()<<"\n\tSQL="<<q.lastQuery();
      return false;
    }
  }
  {
    QSqlQuery q = QSqlQuery(pDB);
    q.prepare("INSERT INTO klf_properties (name,value) VALUES (?,?)");
    q.bindValue(0, propName);
    q.bindValue(1, value);
    if ( ! q.exec() || q.lastError().isValid() ) {
      qWarning()<<"KLFLibDBEngine::setRes.Property("<<propId<<","<<value<<"): can't INSERT!\n\t"
		<<q.lastError().text()<<"\n\tSQL="<<q.lastQuery();
      return false;
    }
  }
  KLFPropertizedObject::setProperty(propId, value);
  emit resourcePropertyChanged(propId);
  return true;
}




// private
QMap<int,int> KLFLibDBEngine::detectEntryColumns(const QSqlQuery& q)
{
  const QSqlRecord rec = q.record();
  QMap<int,int> cols;
  KLFLibEntry e; // dummy entry to test property names
  QList<int> props = e.registeredPropertyIdList();
  int k;
  for (k = 0; k < props.size(); ++k) {
    cols[props[k]] = rec.indexOf(e.propertyNameForId(props[k]));
  }
  return cols;
}
// private
KLFLibEntry KLFLibDBEngine::readEntry(const QSqlQuery& q, QMap<int,int> col)
{
  // and actually read the result and return it
  KLFLibEntry entry;
  entry.setLatex(q.value(col[KLFLibEntry::Latex]).toString());
  entry.setDateTime(QDateTime::fromString(q.value(col[KLFLibEntry::DateTime]).toString(), Qt::ISODate));
  QImage img; img.loadFromData(q.value(col[KLFLibEntry::Preview]).toByteArray());
  entry.setPreview(img);
  entry.setCategory(q.value(col[KLFLibEntry::Category]).toString());
  entry.setTags(q.value(col[KLFLibEntry::Tags]).toString());
  entry.setStyle(metatype_from_data<KLFStyle>(q.value(col[KLFLibEntry::Style]).toByteArray()));
  return entry;
}



KLFLibEntry KLFLibDBEngine::entry(entryId id)
{
  if ( ! validDatabase() )
    return KLFLibEntry();

  QSqlQuery q = QSqlQuery(pDB);
  q.prepare("SELECT * FROM `"+pDataTableName+"` WHERE id = ?");
  q.addBindValue(id);
  bool r = q.exec();

  if ( !r || q.lastError().isValid() || q.size() == 0) {
    qWarning("KLFLibDBEngine::entry: id=%d cannot be found!\n"
	     "SQL Error (?): %s", id, qPrintable(q.lastError().text()));
    return KLFLibEntry();
  }
  //   if (q.size() != 1) {
  //     qWarning("KLFLibDBEngine::entry: %d results returned for id %d!", q.size(), id);
  //     return KLFLibEntry();
  //   }
  if ( ! q.next() ) {
    qWarning("KLFLibDBEngine::entry(): no entry available!\n"
	     "SQL=\"%s\" (?=%d)", qPrintable(q.lastQuery()), id);
    return KLFLibEntry();
  }

  // read the result and return it
  KLFLibEntry e = readEntry(q, detectEntryColumns(q));
  q.finish();
  return e;
}


QList<KLFLibResourceEngine::KLFLibEntryWithId> KLFLibDBEngine::allEntries()
{
  if ( ! validDatabase() )
    return QList<KLFLibResourceEngine::KLFLibEntryWithId>();

  QSqlQuery q = QSqlQuery(pDB);
  q.prepare("SELECT * FROM `"+pDataTableName+"`");
  q.setForwardOnly(true);
  q.exec();

  QList<KLFLibEntryWithId> entryList;

  int colId = q.record().indexOf("id");
  QMap<int,int> cols = detectEntryColumns(q);
  while (q.next()) {
    KLFLibEntryWithId e;
    e.id = q.value(colId).toInt();
    e.entry = readEntry(q, cols);
    entryList << e;
  }

  return entryList;
}


// private
QVariant KLFLibDBEngine::convertVariantToDBData(const QVariant& value) const
{
  QVariant data = value;
  // setup data in proper format if needed
  if (data.type() == QVariant::Image)
    data = QVariant::fromValue<QByteArray>(image_data(data.value<QImage>(), "PNG"));
  if (data.type() == QVariant::DateTime)
    data = data.value<QDateTime>().toString(Qt::ISODate);
  if (data.type() == QVariant::nameToType("KLFStyle"))
    data = QVariant::fromValue<QByteArray>(metatype_to_data(data.value<KLFStyle>()));
  return data;
}

QList<KLFLibResourceEngine::entryId> KLFLibDBEngine::insertEntries(const KLFLibEntryList& entrylist)
{
  int k, j;
  if ( ! validDatabase() )
    return QList<entryId>();
  if ( entrylist.size() == 0 )
    return QList<entryId>();

  if ( !canModifyData(InsertData) )
    return QList<entryId>();

  KLFLibEntry e; // dummy object to test for properties
  QList<int> propids = e.registeredPropertyIdList();
  QStringList props;
  QStringList questionmarks;
  for (k = 0; k < propids.size(); ++k) {
    props << e.propertyNameForId(propids[k]);
    questionmarks << "?";
  }

  QList<entryId> insertedIds;

  QSqlQuery q = QSqlQuery(pDB);
  q.prepare("INSERT INTO `" + pDataTableName + "` (" + props.join(",") + ") "
	    " VALUES (" + questionmarks.join(",") + ")");
  //  qDebug()<<"INSERT query: "<<q.lastQuery();
  // now loop all entries, and exec the query with appropriate bound values
  for (j = 0; j < entrylist.size(); ++j) {
    //    qDebug()<<"New entry to insert.";
    for (k = 0; k < propids.size(); ++k) {
      QVariant data = convertVariantToDBData(entrylist[j].property(propids[k]));
      // and add a corresponding bind value for sql query
      //      qDebug()<<"Binding value "<<k<<": "<<data;
      q.bindValue(k, data);
    }
    // and exec the query with these bound values
    bool r = q.exec();
    if ( ! r || q.lastError().isValid() ) {
      qWarning()<<"INSERT failed! SQL Error: "<<q.lastError().text()<<"\n\tSQL="<<q.lastQuery();
      insertedIds << -1;
    } else {
      QVariant v_id = q.lastInsertId();
      if ( ! v_id.isValid() )
	insertedIds << -2;
      else
	insertedIds << v_id.toInt();
    }
  }

  emit dataChanged();
  return insertedIds;
}


bool KLFLibDBEngine::changeEntries(const QList<entryId>& idlist, const QList<int>& properties,
				   const QList<QVariant>& values)
{
  if ( ! validDatabase() )
    return false;
  if ( ! canModifyData(ChangeData) )
    return false;

  if ( properties.size() != values.size() ) {
    qWarning("KLFLibDBEngine::changeEntry(): properties' and values' sizes mismatch!");
    return false;
  }

  if ( idlist.size() == 0 )
    return true; // no items to change

  KLFLibEntry e; // dummy 
  QStringList updatepairs;
  int k;
  for (k = 0; k < properties.size(); ++k) {
    updatepairs << (e.propertyNameForId(properties[k]) + " = ?");
  }
  QStringList questionmarks_ids;
  for (k = 0; k < idlist.size(); ++k) questionmarks_ids << "?";
  // prepare query
  QSqlQuery q = QSqlQuery(pDB);
  q.prepare("UPDATE `"+pDataTableName+"` SET " + updatepairs.join(", ") + "  WHERE id IN (" +
	    questionmarks_ids.join(", ") + ")");
  for (k = 0; k < properties.size(); ++k) {
    q.addBindValue(convertVariantToDBData(values[k]));
  }
  for (k = 0; k < idlist.size(); ++k)
    q.addBindValue(idlist[k]);
  bool r = q.exec();

  //  qDebug() << "UPDATE: SQL="<<q.lastQuery()<<"; boundvalues="<<q.boundValues().values();
  if ( !r || q.lastError().isValid() ) {
    qWarning() << "SQL UPDATE Error: "<<q.lastError();
    return false;
  }

  //  qDebug() << "Wrote Entry change to ids "<<idlist;
  emit dataChanged();
  return true;
}

bool KLFLibDBEngine::deleteEntries(const QList<entryId>& idlist)
{
  if ( ! validDatabase() )
    return false;
  if (idlist.size() == 0)
    return true; // nothing to do
  if ( ! canModifyData(DeleteData) )
    return false;

  int k;
  QStringList qmarks_ids;
  for (k = 0; k < idlist.size(); ++k)
    qmarks_ids << "?";

  QSqlQuery q = QSqlQuery(pDB);
  q.prepare("DELETE FROM `"+pDataTableName+"` WHERE id IN ("+qmarks_ids.join(", ")+")");
  for (k = 0; k < idlist.size(); ++k)
    q.addBindValue(idlist[k]);
  bool r = q.exec();
  q.finish();

  emit dataChanged();

  if ( !r || q.lastError().isValid())
    return false;   // an error was set

  return true;  // no error
}

bool KLFLibDBEngine::saveAs(const QUrl& newPath)
{
  if (newPath.scheme() != "klf+sqlite") {
    qWarning()<<"KLFLibDBEngine::saveAs("<<newPath<<"): Bad scheme!";
    return false;
  }
  if (!newPath.host().isEmpty()) {
    qWarning()<<"KLFLibDBEngine::saveAs("<<newPath<<"): Expected empty host!";
    return false;
  }
  return QFile::copy(url().path(), newPath.path());
}

// static
bool KLFLibDBEngine::initFreshDatabase(QSqlDatabase db, const QString& dtname)
{
  QString datatablename = "t_"+dtname;
  if ( ! db.isOpen() )
    return false;

  QStringList sql;
  // the CREATE TABLE statements should be adapted to the database type (sqlite, mysql, pgsql) since
  // syntax is different...
  sql << "CREATE TABLE `"+datatablename+"` (id INTEGER PRIMARY KEY, Latex TEXT, DateTime TEXT, "
    "       Preview BLOB, Category TEXT, Tags TEXT, Style BLOB)";
  sql << "CREATE TABLE klf_properties (id INTEGER PRIMARY KEY, name TEXT, value BLOB)";
  sql << "INSERT INTO klf_properties (name, value) VALUES ('Title', 'New Resource')";
  sql << "INSERT INTO klf_properties (name, value) VALUES ('Locked', 'false')";
  sql << "CREATE TABLE klf_dbmetainfo (id INTEGER PRIMARY KEY, name TEXT, value BLOB)";
  sql << "INSERT INTO klf_dbmetainfo (name, value) VALUES ('klf_version', '" KLF_VERSION_STRING "')";
  sql << "INSERT INTO klf_dbmetainfo (name, value) VALUES ('klf_dbversion', '"+
    QString::number(KLF_DB_VERSION)+"')";

  int k;
  for (k = 0; k < sql.size(); ++k) {
    QSqlQuery query(db);
    query.prepare(sql[k]);
    bool r = query.exec();
    if ( !r || query.lastError().isValid() ) {
      qWarning()<<"initFreshDatabase(): SQL Error: "<<query.lastError().text()<<"\n"
		<<"SQL="<<sql[k];
      return false;
    }
  }

  return true;
}


QStringList KLFLibDBEngine::getDataTableNames(const QUrl& url)
{
  KLFLibDBEngine *resource = openUrl(url, NULL);
  QStringList tables = resource->pDB.tables(QSql::Tables);
  delete resource;
  // filter out t_<tablename> tables
  int k;
  QStringList dataTableNames;
  for (k = 0; k < tables.size(); ++k)
    if (tables[k].startsWith("t_"))
      dataTableNames << tables[k].mid(2);
  return dataTableNames;
}


// ------------------------------------


KLFLibDBEngineFactory::KLFLibDBEngineFactory(QObject *parent)
  : KLFLibEngineFactory(parent)
{
}

QStringList KLFLibDBEngineFactory::supportedSchemes() const
{
  return QStringList() << QLatin1String("klf+sqlite");
}

QString KLFLibDBEngineFactory::schemeTitle(const QString& scheme) const
{
  if (scheme == QLatin1String("klf+sqlite"))
    return tr("Local Library File");
  return QString();
}


KLFLibResourceEngine *KLFLibDBEngineFactory::openResource(const QUrl& location, QObject *parent)
{
  return KLFLibDBEngine::openUrl(location, parent);
}

QWidget * KLFLibDBEngineFactory::createPromptUrlWidget(QWidget *parent, const QString& scheme,
						       QUrl defaultlocation)
{
  if (scheme == QLatin1String("klf+sqlite")) {
    KLFLibSqliteOpenWidget *w = new KLFLibSqliteOpenWidget(parent);
    w->setUrl(defaultlocation);
    return w;
  }
  return NULL;
}

QUrl KLFLibDBEngineFactory::retrieveUrlFromWidget(const QString& scheme, QWidget *widget)
{
  if (scheme == "klf+sqlite") {
    if (widget == NULL || !widget->inherits("KLFLibSqliteOpenWidget")) {
      qWarning("KLFLibDBEngineFactory::retrieveUrlFromWidget(): Bad Widget provided!");
      return QUrl();
    }
    return qobject_cast<KLFLibSqliteOpenWidget*>(widget)->url();
  } else {
    qWarning()<<"Bad scheme: "<<scheme;
    return QUrl();
  }
}


QWidget *KLFLibDBEngineFactory::createPromptCreateParametersWidget(QWidget *parent,
								   const QString& scheme,
								   const Parameters& defaultparameters)
{
  if (scheme == QLatin1String("klf+sqlite")) {
    KLFLibSqliteCreateWidget *w = new KLFLibSqliteCreateWidget(parent);
    w->setUrl(defaultparameters["Url"].toUrl());
    return w;
  }
  return NULL;
}

KLFLibEngineFactory::Parameters
/* */ KLFLibDBEngineFactory::retrieveCreateParametersFromWidget(const QString& scheme,
								QWidget *widget)
{
  if (scheme == QLatin1String("klf+sqlite")) {
    if (widget == NULL || !widget->inherits("KLFLibSqliteCreateWidget")) {
      qWarning("KLFLibDBEngineFactory::retrieveUrlFromWidget(): Bad Widget provided!");
      return Parameters();
    }
    KLFLibSqliteCreateWidget *w = qobject_cast<KLFLibSqliteCreateWidget*>(widget);
    Parameters p;
    QString filename = w->fileName();
    if (QFile::exists(filename) && !w->confirmedOverwrite()) {
      QMessageBox::StandardButton result =
	QMessageBox::warning(widget, tr("Overwrite?"),
			     tr("The specified file already exists. Overwrite it?"),
			     QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel,
			     QMessageBox::No);
      if (result == QMessageBox::No) {
	p["klfRetry"] = true;
	return p;
      } else if (result == QMessageBox::Cancel) {
	return Parameters();
      }
      // remove the previous file, because otherwise KLFLibDBEngine will fail.
      bool r = QFile::remove(filename);
      if ( !r ) {
	QMessageBox::critical(widget, tr("Error"), tr("Failed to overwrite the file %1.")
			      .arg(filename));
	return Parameters();
      }
    }

    p["Filename"] = w->fileName();
    p["Url"] = w->url();
    return p;
  }

  return Parameters();
}

KLFLibResourceEngine *KLFLibDBEngineFactory::createResource(const QString& scheme,
							    const Parameters& parameters,
							    QObject *parent)
{
  if (scheme == QLatin1String("klf+sqlite")) {
    if ( !parameters.contains("Filename") || !parameters.contains("Url") ) {
      qWarning()
	<<"KLFLibDBEngineFactory::createResource: bad parameters. They do not contain `Filename' or\n"
	"`Url': "<<parameters;
      return NULL;
    }
    return KLFLibDBEngine::createSqlite(parameters["Filename"].toString(),
					parameters["dataTableName"].toString(), parent);
  }
  return NULL;
}
