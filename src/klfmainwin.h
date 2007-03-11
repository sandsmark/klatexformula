/***************************************************************************
 *   file klfmainwin.h
 *   This file is part of the KLatexFormula Project.
 *   Copyright (C) 2007 by Philippe Faist
 *   philippe.faist@bluewin.ch
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

#ifndef KLFMAINWIN_H
#define KLFMAINWIN_H

#include <kmainwindow.h>
#include <kpopupmenu.h>

#include <klfbackend.h>

#include <klfdata.h>
//#include <klfhistorybrowser.h>
#include <klfprogerrui.h>


class KLFMainWinUI;
class KLFHistoryBrowser;


class KLFProgErr : public KLFProgErrUI {
  Q_OBJECT
public:
  KLFProgErr(QWidget *parent, QString errtext);
  virtual ~KLFProgErr();

  static void showError(QWidget *parent, QString text);

};


/**
 * KLatexFormula Main Window
 * @author Philippe Faist &lt;philippe.faist@bluewin.ch&gt;
*/
class KLFMainWin : public KMainWindow
{
  Q_OBJECT
public:
  KLFMainWin();
  virtual ~KLFMainWin();

  bool eventFilter(QObject *obj, QEvent *event);

  KLFData::KLFStyle currentStyle() const;

public slots:

  void slotEvaluate();
  void slotClear();
  void slotHistory(bool showhist);
  void slotHistoryButtonRefreshState(bool on);
  void slotExpandOrShrink();
  void slotQuit();

  void slotDrag();
  void slotCopy();
  void slotSave();

  void slotLoadStyle(int stylenum);
  void slotSaveStyle();
  void slotSettings();

  void refreshStylePopupMenus();
  void loadStyles();
  void loadHistory();
  void saveStyles();
  void saveHistory();
  void restoreFromHistory(KLFData::KLFHistoryItem h, bool restorestyle);

protected:
  KLFMainWinUI *mMainWidget;
  KLFHistoryBrowser *mHistoryBrowser;

  KPopupMenu *mStyleMenu;

  KLFBackend::klfSettings _settings; // settings we pass to KLFBackend

  KLFBackend::klfOutput _output; // output from KLFBackend

  KLFData::KLFHistoryList _history;
  KLFData::KLFStyleList _styles;

  QSize _shrinkedsize;
  QSize _expandedsize;

  void saveProperties(KConfig *cfg);
  void readProperties(KConfig *cfg);
};

#endif
