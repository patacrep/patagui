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
#include <QFuture>

class CSongbook;
class Library;
class LibraryView;
class CTabWidget;
class CEditor;
class Label;
class CTabWidget;
class FilterLineEdit;
class Notification;
class ProgressBar;
class MakeSongbookProcess;
class CSongHighlighter;

class QPlainTextEdit;
class QItemSelectionModel;
class QSortFilterProxyModel;
class QFileSystemModel;
class QLabel;

/*!
  \file main-window.hh
  \class MainWindow
  \brief MainWindow is the base class of the application.

  \image html main-window.png

  Class for the main window of the application.
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void songEditor(const QString &filename);

    void open(const QString &filename);

public:
    /// Constructor.
    MainWindow(QWidget *parent = 0);

    /// Destructor.
    ~MainWindow();

    /*!
    Returns the progress bar that is embedded in the status bar.
  */
    ProgressBar *progressBar() const;

    /*!
    Returns the dock widget that displays LaTeX compilation logs.
    \image html logs.png
  */
    QDockWidget *log() const;

    /*!
    Returns the library view.
  */
    LibraryView *view() const;

    /*!
    Returns the library.
  */
    Library *library() const;

    /*!
    Returns the current songbook.
  */
    CSongbook *songbook() const;

    /*!
    Returns the directory of the songbook.
  */
    const QString workingPath();

    /*!
    Getter on the songs library directory.
    @return the path to the songs/ directory
  */
    const QString libraryPath();

    /*!
    Calls pdflatex to build the songbook.
    \sa makeClean, makeCleanall
  */
    void make();

    /*!
    Removes LaTeX temporary files (*.aux *.log etc.)
    that are generated in the songbook directory during the make().
    \sa make, makeCleanall
  */
    void makeClean();

    /*!
    Removes LaTeX temporary files (*.aux *.log etc.) and PDF files
    that are generated in the songbook directory during the make().
    \sa make, makeClean
  */
    void makeCleanall();

protected:
    /*!
    Saves settings before closing the application.
  */
    void closeEvent(QCloseEvent *event);

private slots:
    // songbook
    void newSongbook();
    void open();
    void save();
    void saveAs();
    void build();
    void closeTab(int index);
    void changeTab(int index);

    /// Displays a dialog to remove temporary LaTeX files.
    /// \image html clean.png
    void cleanDialog();
    void updateTempFilesView(int state);

    // library
    void newSong();
    void importSongs(const QStringList &songs);
    void importSongsDialog();
    void middleClicked(const QModelIndex &index = QModelIndex());
    void songEditor(const QModelIndex &index = QModelIndex());
    void deleteSong();

    void setupDatadirDialog();
    void deleteSong(const QString &filename);
    void updateNotification(const QString &path);
    void noDataNotification(const QDir &directory);
    void noSongbookDirectoryNotification();

    // model
    void selectedSongsChanged(const QModelIndex &topLeft,
                              const QModelIndex &bottomRight);

    // application
    void preferences();
    void setToolBarDisplayed(bool);
    void setStatusBarDisplayed(bool);
    void documentation();
    void reportBug();
    void about();

    void updateTitle(const QString &filename);
    void switchToolBar(QToolBar *toolBar);

    void cancelProcess();

private:
    void readSettings(bool firstLaunch = false);
    void writeSettings();

    void createActions();
    void createMenus();
    void createToolBar();

    bool isToolBarDisplayed();
    bool isStatusBarDisplayed();

    QItemSelectionModel *selectionModel();

    // Models and views
    LibraryView *m_view;
    CSongbook *m_songbook;
    QSortFilterProxyModel *m_proxyModel;
    QFileSystemModel *m_tempFilesmodel;

    // Widgets
    CTabWidget *m_mainWidget;
    ProgressBar *m_progressBar;
    Notification *m_noDataInfo;
    Notification *m_noDatadirSet;
    Notification *m_updateAvailable;
    QLabel *m_infoSelection;
    FilterLineEdit *m_filterLineEdit;
    QDockWidget *m_log;

    // Settings
    QString m_workingPath;
    bool m_isToolBarDisplayed;
    bool m_isStatusBarDisplayed;

    // Menus
    QMenu *m_editorMenu;
    QToolBar *m_libraryToolBar;
    QToolBar *m_currentToolBar;
    QToolBar *m_mainToolBar;

    // Application actions
    QAction *m_preferencesAct;
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
    MakeSongbookProcess *m_builder;

    // Library action
    QAction *m_newSongAct;
    QAction *m_importSongsAct;
    QAction *m_setupDatadirAct;
    QAction *m_selectAllAct;
    QAction *m_unselectAllAct;
    QAction *m_invertSelectionAct;
    QAction *m_libraryUpdateAct;

    // Editor
    CEditor *m_voidEditor;
    CSongHighlighter *m_songHighlighter;

    // Building Process
    QFuture<void> future;

public:
    const static QString _cachePath;
};

#endif // __MAIN_WINDOW_HH__
