/***************************************************************************
 *   file klfmime.h
 *   This file is part of the KLatexFormula Project.
 *   Copyright (C) 2009 by Philippe Faist
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

#ifndef KLFMIME_H
#define KLFMIME_H

#include <QString>
#include <QList>
#include <QStringList>
#include <QByteArray>
#include <QMap>
#include <QImage>
#include <QImageWriter>
#include <QMimeData>
#include <QTemporaryFile>

#include <klfdefs.h>
#include <klfbackend.h>



class KLF_EXPORT KLFMimeExporter : public QObject
{
  Q_OBJECT
public:
  KLFMimeExporter(QObject *parent) : QObject(parent) { }
  virtual ~KLFMimeExporter() { }

  virtual QStringList keys() const = 0;
  virtual QByteArray data(const QString& key, const KLFBackend::klfOutput& klfoutput) = 0;

  /* returns the MS Windows Format Name for the given mime-type \c key.
   *
   * The default implementation just returns \c key. You should reimplement
   * this function to return a useful standard name.
   */
  virtual QString windowsFormatName(const QString& key) const { return key; }

  /** \brief Shortcut function (do not reimplement in subclasses)
   *
   * \returns true if key is in \ref keys() list. */
  virtual bool supportsKey(const QString& key) const;

  /** Looks up an exporter supporting type \c key. The first exporter in the list
   * is returned. If no exporter is found, NULL is returned. */
  static KLFMimeExporter * mimeExporterLookup(const QString& key);
  
  static QList<KLFMimeExporter *> mimeExporterList();
  /** Adds the instance \c exporter to the internal list of exporters. Ownership of the
   * object is NOT transferred.
   *
   * If \c overrides is TRUE, then prepends the exporter to the list (will be looked
   * up first), otherwise appends it to list. */
  static void addMimeExporter(KLFMimeExporter *exporter, bool overrides = true);

private:
  static void initMimeExporterList();
  static QList<KLFMimeExporter*> p_mimeExporterList;
};


class KLF_EXPORT KLFMimeExportProfile
{
public:
  KLFMimeExportProfile(const QString& pname, const QString& desc, const QStringList& mtypes,
		       const QStringList& wintypes);
  KLFMimeExportProfile(const KLFMimeExportProfile& copy);

  QString profileName() const { return p_profileName; }
  QString description() const { return p_description; }
  QStringList mimeTypes() const { return p_mimeTypes; }

  /** Windows Clipboard Formats to show for each mime type (respectively).
   *
   * An empty string in the list means that the caller should query the mime
   * exporter for a windows format name with KLFMimeExporter::windowsFormatName().
   */
  QStringList respectiveWinTypes() const { return p_respectiveWinTypes; }

  /** Returns the k-th element in respectiveWinTypes. If that element is empty,
   * queries the correct mime-type exporter for a windows format name with
   * KLFMimeExporter::windowsFormatName().
   */
  QString respectiveWinType(int k) const;


  /** Returns a list of mime types, for which we garantee that (at least at the time
   * of calling this function), KLFMimeExporter::mimeExporterLookup(mimetype) will
   * not return NULL.
   */
  QStringList availableExporterMimeTypes() const;


  static QList<KLFMimeExportProfile> exportProfileList();
  static void addExportProfile(const KLFMimeExportProfile& exportProfile);

  static KLFMimeExportProfile findExportProfile(const QString& pname);

private:

  QString p_profileName;
  QString p_description;
  QStringList p_mimeTypes;
  QStringList p_respectiveWinTypes;

  static void ensureLoadedExportProfileList();
  static void loadFromXMLFile(const QString& fname);
  static QList<KLFMimeExportProfile> p_exportProfileList;
};



/** A QMimeData subclass for Copy and Drag operations in KLFMainWin, that supports delayed
 * data processing, ie. that actually creates the requested data only on drop or paste, and
 * not when the operation is initiated.
 *
 * This function can be used as a regular QMimeData object to copy or drag any
 * KLFBackend::klfOutput data, with a given export profile.
 */
class KLF_EXPORT KLFMimeData : public QMimeData
{
  Q_OBJECT
public:
  KLFMimeData(const QString& exportProfileName, const KLFBackend::klfOutput& output);
  virtual ~KLFMimeData();

  QStringList formats() const;

protected:
  QVariant retrieveData(const QString &mimetype, QVariant::Type type) const;

private:
  KLFMimeExportProfile pExportProfile;
  KLFBackend::klfOutput pOutput;
};





#endif
