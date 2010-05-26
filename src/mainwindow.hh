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

/** \class CMainWindow "mainWindow.hh"
 * \brief CMainWindow is the base class of the application
 */

class CMainWindow : public QMainWindow
{
  Q_OBJECT
  
public:
  CMainWindow();
  ~CMainWindow();

protected:
  void closeEvent(QCloseEvent *event);
  void contextMenuEvent(QContextMenuEvent *event);

public slots:
  void buildFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
  void buildError(QProcess::ProcessError error);
  void updateCover(const QModelIndex & index);
  void readProcessOut();
  void downloadDialog();
  void synchroniseWithLocalSongs();  
  // void playSong();

private slots:
  void open();
  void save();
  void build();
  void makeLilypondSheets();
  void clean();

  void selectAll();
  void unselectAll();
  void invertSelection();
  void setDisplaySongInfo(bool value);
  void setDisplayLogInfo(bool value);
  void applyDisplayColumn();
  void connectDb();
  void filterRegExpChanged();
  
  void dockWidgets();
  void preferences();

  void about(); 

private:
  void readSettings();
  void writeSettings();

  void createActions();
  void createMenus();

  QString packageOptions();
  bool createDbConnection();

  const QString workingPath();

  QStringList getSelectedSongs();

  /// Modify mybook.tex according to the selected options
  void applyBookType();

public:
  void setWorkingPath(QString dirname);

public:
  // Widgets
  QProgressBar* m_progressBar;

private:
  // Song library and view
  CLibrary *m_library;
  QTableView *m_view;
  QItemSelectionModel *m_selectionModel;
  QSortFilterProxyModel *m_proxyModel;

  // Global
  QString m_workingPath;

  bool m_bookTypeChordbook;
  bool m_bookTypeLyricbook;

  bool m_optionChordDiagram;
  bool m_optionLilypond;
  bool m_optionTablature;

  bool m_displayColumnArtist;
  bool m_displayColumnTitle;
  bool m_displayColumnPath;
  bool m_displayColumnAlbum;
  bool m_displayColumnLilypond;
  bool m_displayColumnCover;

  QProcess* m_buildProcess;
  QMdiArea* m_area;
  QScrollArea* m_scrollArea;
  QDockWidget* m_songInfo;
  QDockWidget* m_logInfo;
  QPixmap* m_cover;
  QLabel   m_coverLabel;
  uint m_dbType;

  //Filtering
  QLineEdit* m_filterPatternLineEdit;
  QLabel* m_filterPatternLabel;
  QComboBox* m_filterSyntaxComboBox;
  QLabel* m_filterSyntaxLabel;
  QGroupBox* m_proxyGroupBox;

  //Header
  QLineEdit* m_title;
  QLineEdit* m_subtitle;
  QLineEdit* m_author;
  QLineEdit* m_version;
  QLineEdit* m_mail;
  QLineEdit* m_picture;
  QLineEdit* m_copyright;

  //Logs
  QTextEdit* m_log;

  // Menus
  QMenu *m_fileMenu;
  QMenu *m_editMenu;
  QMenu *m_dbMenu;
  QMenu *m_viewMenu;
  QMenu *m_helpMenu;

  // Actions
  QAction *m_exitAct;
  QAction *m_openAct;
  QAction *m_saveAct;
  QAction *m_buildAct;
  QAction *m_cleanAct;
  QAction *m_preferencesAct;
  QAction *m_aboutAct;
  QAction *m_selectAllAct;
  QAction *m_unselectAllAct;
  QAction *m_invertSelectionAct;
  QAction *m_displaySongInfoAct;
  QAction *m_displayLogInfoAct;
  QAction *m_adjustColumnsAct;
  QAction *m_connectDbAct;
  QAction *m_downloadDbAct;
  QAction *m_rebuildDbAct;
  QAction *m_resizeCoversAct;
  QAction *m_checkerAct;
};

#endif  // __MAIN_WINDOW_HH__
