// Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2011, Alexandre Dupas <alexandre.dupas@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
//******************************************************************************

#ifndef __MAIN_WINDOW_HH__
#define __MAIN_WINDOW_HH__

#include <QMainWindow>

#include <QModelIndex>
#include <QDir>
#include <QProcess>

class CSongbook;
class CLibrary;
class CLibraryView;
class CTabWidget;
class CSongEditor;
class CLabel;
class CTabWidget;
class CFilterLineEdit;
class CNotification;
class CProgressBar;
class CMakeSongbookProcess;

class QPlainTextEdit;
class QItemSelectionModel;
class QSortFilterProxyModel;
class QFileSystemModel;
class QLabel;

/**
 * \file main-window.hh
 * \class CMainWindow
 * \brief CMainWindow is the base class of the application.
 *
 * \image html main-window.png
 *
 * Class for the main window of the application.
 *
 */
class CMainWindow : public QMainWindow
{
  Q_OBJECT

public slots:
  /// System call that allows to build a songbook.
  /// @param command : the command line
  void setBuildCommand(const QString &command);

  /// System call that allows to remove temporary files
  /// that are generated during the compilation of a songbook.
  /// @param command : the command line
  void setCleanCommand(const QString &command);

  /// System call that allows to remove temporary files and pdf files
  /// that are generated during the compilation of a songbook.
  /// @param command : the command line
  void setCleanallCommand(const QString &command);

public:
  /// Constructor.
  CMainWindow(QWidget *parent=0);

  /// Destructor.
  ~CMainWindow();

  /// Getter on the progress bar that is embedded in the status bar.
  /// @return the progress bar
  CProgressBar * progressBar() const;

  /// Getter on the LaTeX compilation logs widget that
  /// displays terminal output of the compilation process.
  /// \image html logs.png
  /// @return the logs widget
  QDockWidget * log() const;

  /// Getter on the songs' library view.
  /// @return the songs' library view
  CLibraryView * view() const;

  /// Getter on the songs' library.
  /// @return the songs' library
  CLibrary * library() const;

  /// Getter on songs' library.
  /// @return the songs' library
  CSongbook * songbook() const;

  /// Getter on the songbook directory.
  /// @return the path to the songbook directory
  const QString workingPath();

  /// Getter on the songbook build command.
  /// @return the command line
  const QString & buildCommand() const;

  /// Getter on the songbook clean command.
  /// @return the command line
  const QString & cleanCommand() const;

  /// Getter on the songbook cleanall command.
  /// @return the command line
  const QString & cleanallCommand() const;

  /// Calls pdflatex to build the songbook
  void make();

  /// Remove LaTeX temporary files
  void makeClean();

  /// Remove LaTeX temporary files and pdf files
  void makeCleanall();

protected:
  void closeEvent(QCloseEvent *event);

private slots:
  //songbook
  void newSongbook();
  void open();
  void save(bool forced = false);
  void saveAs();
  void build();
  void closeTab(int index);
  void changeTab(int index);

  /// Displays a dialog to download a remote songbook.
  /// \image html download.png
  void downloadDialog();

  /// Displays a dialog to remove temporary LaTeX files.
  /// \image html clean.png
  void cleanDialog();
  void updateTempFilesView(int state);

  //library
  void newSong();
  void songEditor(const QModelIndex &index = QModelIndex());
  void deleteSong();

  void songEditor(const QString &filename);
  void deleteSong(const QString &filename);
  void updateNotification(const QString &path);
  void noDataNotification(const QDir &directory);

  //model
  void selectedSongsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

  //application
  void preferences();
  void setToolBarDisplayed(bool);
  void setStatusbarDisplayed(bool);
  void documentation();
  void reportBug();
  void about();

  void updateTitle(const QString &filename);
  void switchToolBar(QToolBar *toolBar);

  void buildError(QProcess::ProcessError error);

  void cancelProcess();

private:
  void readSettings(bool firstLaunch=false);
  void writeSettings();

  void createActions();
  void createMenus();
  void createToolBar();

  bool isToolBarDisplayed();
  bool isStatusbarDisplayed();

  QItemSelectionModel * selectionModel();

  // Models and views
  CLibrary *m_library;
  CLibraryView *m_view;
  CSongbook *m_songbook;
  QSortFilterProxyModel *m_proxyModel;
  QFileSystemModel* m_tempFilesmodel;

  // Widgets
  CTabWidget *m_mainWidget;
  CProgressBar *m_progressBar;
  CNotification *m_noDataInfo;
  CNotification *m_updateAvailable;
  QLabel *m_infoSelection;
  CFilterLineEdit *m_filterLineEdit;
  QDockWidget *m_log;

  // Settings
  QString m_workingPath;
  bool m_isToolBarDisplayed;
  bool m_isStatusbarDisplayed;
  QString m_buildCommand;
  QString m_cleanCommand;
  QString m_cleanallCommand;

  // Menus
  QMenu *m_editorMenu;
  CSongEditor *m_voidEditor;
  QToolBar *m_libraryToolBar;
  QToolBar *m_currentToolBar;
  QToolBar *m_mainToolBar;

  // Application actions
  QAction *m_preferencesAct;
  QAction *m_toolBarViewAct;
  QAction *m_statusbarViewAct;
  QAction *m_adjustColumnsAct;
  QAction *m_documentationAct;
  QAction *m_bugsAct;
  QAction *m_aboutAct;
  QAction *m_exitAct;

  // Songbook actions
  QAction *m_newAct;
  QAction *m_openAct;
  QAction *m_saveAct;
  QAction *m_saveAsAct;
  QAction *m_buildAct;
  QAction *m_cleanAct;
  QAction *m_sbInfoAct;
  CMakeSongbookProcess *m_builder;

  // Library action
  QAction *m_newSongAct;
  QAction *m_selectAllAct;
  QAction *m_unselectAllAct;
  QAction *m_invertSelectionAct;
  QAction *m_libraryUpdateAct;
  QAction *m_libraryDownloadAct;
};

#endif  // __MAIN_WINDOW_HH__
