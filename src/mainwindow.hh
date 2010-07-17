// Copyright (C) 2009 Romain Goffe, Alexandre Dupas
//
// Songbook Creator is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Songbook Creator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//******************************************************************************

/**
 * \file mainWindow.hh
 *
 * Class for the main window of the application.
 *
 */

#ifndef __MAIN_WINDOW_HH__
#define __MAIN_WINDOW_HH__

#include <QtGui>

class CSongbook;
class CLibrary;
class CTabWidget;
class CDialogNewSong;

/** \class CMainWindow "mainWindow.hh"
 * \brief CMainWindow is the base class of the application
 */

class CMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  CMainWindow();
  ~CMainWindow();

  QProgressBar * progressBar();

public slots:
  void buildFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void buildError(QProcess::ProcessError error);
  void updateCover(const QModelIndex & index);
  void readProcessOut();
  void downloadDialog();
  void synchroniseWithLocalSongs();

  void setWorkingPath(QString dirname);

protected:
  void closeEvent(QCloseEvent *event);
  void contextMenuEvent(QContextMenuEvent *event);

private slots:
  void newSongbook();
  void open();
  void save();
  void saveAs();

  void build();
  void clean();

  void newSong();
  void songTemplate();
  bool checkNewSongRequiredFields();
  void selectAll();
  void unselectAll();
  void invertSelection();
  void updateSongsList();

  void applyOptionChanges();

  bool connectDb();
  void filterChanged();

  void preferences();

  void songEditor();
  void changeTabLabel();
  void deleteSong();

  void setToolbarDisplayed(bool);
  void setStatusbarDisplayed(bool);

  void documentation();
  void about();

  void updateTitle(const QString &filename);

private:
  void readSettings();
  void writeSettings();

  void createActions();
  void createMenus();

  QWidget * createSongInfoWidget();

  bool createDbConnection();

  const QString workingPath();

  QStringList getSelectedSongs();

  QString filenameConvention(const QString &, const QString & sep);

  bool isToolbarDisplayed();
  bool isStatusbarDisplayed();

  QItemSelectionModel * selectionModel();
  QDataWidgetMapper* m_mapper;
  
  // Song library and view
  CLibrary *m_library;
  QSortFilterProxyModel *m_proxyModel;

  // Songbook
  CSongbook *m_songbook;

  // Widgets
  CTabWidget* m_mainWidget;
  QTableView *m_view;
  QProgressBar* m_progressBar;

  // Global
  QString m_workingPath;

  bool m_displayColumnArtist;
  bool m_displayColumnTitle;
  bool m_displayColumnPath;
  bool m_displayColumnAlbum;
  bool m_displayColumnLilypond;
  bool m_displayColumnCover;
  bool m_displayCompilationLog;

  bool m_isToolbarDisplayed;
  bool m_isStatusbarDisplayed;

  QProcess* m_buildProcess;
  QMdiArea* m_area;
  QScrollArea* m_scrollArea;
  QDockWidget* m_songInfo;
  QDockWidget* m_logInfo;
  QPixmap *m_cover;
  QLabel m_coverLabel;
  uint m_dbType;
  QBoxLayout* m_currentSongWidgetLayout;
  CDialogNewSong *m_newSongDialog;
  QDockWidget* m_songbookInfo;

  //Logs
  QTextEdit* m_log;

  // Menus
  QMenu *m_fileMenu;
  QMenu *m_editMenu;
  QMenu *m_dbMenu;
  QMenu *m_viewMenu;
  QMenu *m_helpMenu;

  QToolBar *m_toolbar;

  // Actions
  QAction *m_newAct;
  QAction *m_openAct;
  QAction *m_saveAct;
  QAction *m_saveAsAct;
  QAction *m_documentationAct;
  QAction *m_aboutAct;
  QAction *m_exitAct;

  QAction *m_newSongAct;
  QAction *m_buildAct;
  QAction *m_cleanAct;
  QAction *m_preferencesAct;
  QAction *m_selectAllAct;
  QAction *m_unselectAllAct;
  QAction *m_invertSelectionAct;
  QAction *m_toolbarViewAct;
  QAction *m_statusbarViewAct;
  QAction *m_adjustColumnsAct;
  QAction *m_connectDbAct;
  QAction *m_downloadDbAct;
  QAction *m_rebuildDbAct;
  QAction *m_resizeCoversAct;
  QAction *m_checkerAct;

};

class CTabWidget : public QTabWidget
{
  Q_OBJECT

public:
  CTabWidget();
  virtual ~CTabWidget();

  int addTab(QWidget* widget, const QString & label);

private slots:
  void closeTab(int);

};

#endif  // __MAIN_WINDOW_HH__
