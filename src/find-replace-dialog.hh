// Copyright (C) 2009-2012, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2012, Alexandre Dupas <alexandre.dupas@gmail.com>
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

#ifndef __FIND_REPLACE_DIALOG_HH
#define __FIND_REPLACE_DIALOG_HH

#include <QDialog>
#include <QTextCursor>

class QPlainTextEdit;
class QSettings;
class QComboBox;
class QCheckBox;

/*!
  \file find-replace-dialog.hh
  \class FindReplaceDialog
  \brief FindReplaceDialog is a find and replace dialog.

  \image html find-replace.png

*/
class FindReplaceDialog : public QDialog
{
    Q_OBJECT
public:
    /// Constructor.
    FindReplaceDialog(QWidget *parent = 0);

    /// Destructor.
    virtual ~FindReplaceDialog();

    /*!
    Sets \a editor as the text editor upon which
    find and replace operations are applied.
  */
    void setTextEditor(QPlainTextEdit *editor);

    /*!
    Saves the settings of the dialog.
    \sa readSettings
  */
    void writeSettings();

    /*!
    Reads the settings of the dialog.
    \sa writeSettings
  */
    void readSettings();

    /*!
    Returns the maximum size of history.
    \sa setHistorySize
  */
    int historySize() const;

    /*!
    Sets the maximum size of history to \a value.
    \sa historySize
  */
    void setHistorySize(int value);

public slots:
    /*!
    Finds the next occurrence in the editor's contents
    based upon the settings of the dialog.
    \sa replace, replaceAll
  */
    bool find();

    /*!
    Replace the current selection in the editor.
    \sa find, replaceAll
  */
    void replace();

    /*!
    Replace all occurrences in the editor.
    \sa find, replace
  */
    void replaceAll();

private slots:
    void onValueChanged(const QString &);

private:
    QTextCursor cursor() const;
    void appendToHistory(QComboBox *widget, QStringList &history);
    void setStatusTip(const QString &message);

    QPlainTextEdit *m_editor;

    QComboBox *m_findComboBox;
    QComboBox *m_replaceComboBox;

    QCheckBox *m_caseCheckBox;
    QCheckBox *m_wholeWordsCheckBox;
    QCheckBox *m_searchBackwardsCheckBox;
    QCheckBox *m_wrapCheckBox;

    QPushButton *m_findButton;
    QPushButton *m_replaceButton;
    QPushButton *m_replaceAllButton;

    QStringList m_findWords;
    QStringList m_replaceWords;
    int m_historySize;
};

#endif // __FIND_REPLACE_DIALOG_HH
