// Copyright (C) 2009-2013, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2013, Alexandre Dupas <alexandre.dupas@gmail.com>
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

#ifndef __CONFLICT_DIALOG_HH
#define __CONFLICT_DIALOG_HH

#include "main-window.hh"
#include "progress-bar.hh"

#include <QDialog>
#include <QString>
#include <QMap>
#include <QStatusBar>

class QLabel;
class QTableWidget;
class QTableWidgetItem;
class QPushButton;
class QPixmap;

class CProgressBar;
class CFileCopier;


/*!
  \file conflict-dialog.hh
  \class CConflictDialog
  \brief CConflictDialog is a dialog to solve conflicts when importing already existing songs.

  A conflict dialog displays side-by-side two list of songs
  \li source: the list of conflictig songs that are being imported
  \li target: the list of already existing songs from the current library

  The dialog then presents actions to:
  \li overwrite (replace target with source)
  \li preserve  (keep target and ignore source)
  \li show differences (a diff view that relies on the <a href="http://code.google.com/p/google-diff-match-patch/">diff_match_patch</a> library)

  \image html conflict-dialog.png

*/
class CConflictDialog : public QDialog
{
  Q_OBJECT
  Q_ENUMS(ConflictSolveMode)

  public:
  /// Constructor.
  CConflictDialog(QWidget *parent = 0);

  /// Destructor.
  virtual ~CConflictDialog();

  /*!
    Define \a map as the double list of existing / to be imported files
    Key: path to source file (new song)
    Value: path to target file (existing song)
  */
  void setSourceTargetFiles(const QMap< QString, QString > &map);

  /*!
    Determines whether there are some conflicts between
    source and target files. Returns \a true if
    there are some conflicting names, \a false otherwise.
  */
  bool conflictsFound() const;

  /*!
    Returns the parent window of this dialog.
  */
  CMainWindow* parent() const;

  /*!
    Sets \a parent as the parent window of this dialog.
  */
  void setParent(CMainWindow* parent);

  /*!
    Display an informative message in the status
    bar of the parent window.
  */
  void showMessage(const QString & );

  /*!
    Returns the progress bar of the parent window.
  */
  CProgressBar* progressBar() const;

public slots:
  /*!
    Resolves existing conflicts by overwriting
    target files with source files
  */
  bool resolve();

  /*!
    Displays differences between conflicting
    source and target files in a new dialog
  */
  void showDiff();

private slots:
  void updateItemDetails(QTableWidgetItem* item);
  void openItem(QTableWidgetItem* item);
  void cancelCopy();

private:
  CMainWindow *m_parent;
  bool m_conflictsFound;
  QMap< QString, QString> m_conflicts;
  QMap< QString, QString> m_noConflicts;

  QLabel *m_mainLabel;

  QTableWidget *m_conflictView;

  QLabel *m_titleLabel;
  QLabel *m_artistLabel;
  QLabel *m_albumLabel;
  QLabel *m_pathLabel;
  QLabel *m_coverLabel;
  QPixmap *m_pixmap;

  QPushButton *m_overwriteButton;
  QPushButton *m_keepOriginalButton;
  QPushButton *m_diffButton;

  CFileCopier *m_fileCopier;
};

class CFileCopier : public QObject
{
  Q_OBJECT
  
  public:

  CFileCopier(QWidget *parent) : m_cancelCopy(false)
  {
    setParent(static_cast<CMainWindow*>(parent));
  }

  void setSourceTargets(QMap<QString, QString> &files)
  {
    m_sourceTargets = files;
  }

  bool cancelCopy() const
  {
    return m_cancelCopy;
  }

  CMainWindow* parent() const
  {
    return m_parent;
  }

  void setParent(CMainWindow* parent)
  {
    m_parent = parent;
  }

  void setCancelCopy(bool value)
  {
    m_cancelCopy = value;
  }
 
  public slots:
  void copy()
  {
    int count = 0;
    CProgressBar * progressBar = parent()->progressBar();
    progressBar->setRange(0, m_sourceTargets.size());
    progressBar->show();

    QMap<QString, QString>::const_iterator it = m_sourceTargets.constBegin();
    while (it != m_sourceTargets.constEnd())
      {
	if (cancelCopy())
	  break;

	QFile source(it.key());
	QFile target(it.value());
	if (!target.exists() && !source.copy(target.fileName()))
	  {
	    parent()->statusBar()->showMessage
	      (tr("An unexpected error occurred while copying: %1 to %2")
	       .arg(source.fileName())
	       .arg(target.fileName()));
	  }
	progressBar->setValue(++count);
	++it;
      }
    progressBar->hide();
  }

private:
  CMainWindow *m_parent;
  bool m_cancelCopy;
  QMap<QString, QString> m_sourceTargets;
};

#endif // __CONFLICT_DIALOG_HH
