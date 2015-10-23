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

#include "find-replace-dialog.hh"

#include <QPushButton>
#include <QFormLayout>
#include <QBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QSettings>
#include <QDialogButtonBox>
#include <QTextDocument>
#include <QPlainTextEdit>

#include <QDebug>

FindReplaceDialog::FindReplaceDialog(QWidget *parent)
    : QDialog(parent)
    , m_editor(0)
    , m_findComboBox(new QComboBox(this))
    , m_replaceComboBox(new QComboBox(this))
    , m_caseCheckBox(new QCheckBox(this))
    , m_wholeWordsCheckBox(new QCheckBox(this))
    , m_searchBackwardsCheckBox(new QCheckBox(this))
    , m_wrapCheckBox(new QCheckBox(this))
    , m_findButton(new QPushButton(tr("&Find")))
    , m_replaceButton(new QPushButton(tr("&Replace")))
    , m_replaceAllButton(new QPushButton(tr("Replace &all")))
{
    setModal(false);
    m_findComboBox->setEditable(true);
    m_replaceComboBox->setEditable(true);
    connect(m_findComboBox, SIGNAL(editTextChanged(const QString &)),
            this, SLOT(onValueChanged(const QString &)));

    // button box
    m_findButton->setDefault(true);
    m_findButton->setEnabled(false);
    m_replaceButton->setEnabled(false);
    m_replaceAllButton->setEnabled(false);
    connect(m_findButton, SIGNAL(clicked()), SLOT(find()));
    connect(m_replaceButton, SIGNAL(clicked()), SLOT(replace()));
    connect(m_replaceAllButton, SIGNAL(clicked()), SLOT(replaceAll()));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    buttonBox->addButton(m_replaceAllButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_replaceButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(m_findButton, QDialogButtonBox::ActionRole);
    connect(buttonBox, SIGNAL(rejected()), SLOT(close()));

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Find:"), m_findComboBox);
    formLayout->addRow(tr("Replace with:"), m_replaceComboBox);
    formLayout->addRow(tr("Match case"), m_caseCheckBox);
    formLayout->addRow(tr("Match entire word only"), m_wholeWordsCheckBox);
    formLayout->addRow(tr("Search backwards"), m_searchBackwardsCheckBox);
    formLayout->addRow(tr("Wrap around"), m_wrapCheckBox);

    QBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle(tr("Replace"));

    readSettings();
}

FindReplaceDialog::~FindReplaceDialog()
{}

void FindReplaceDialog::readSettings()
{
    QSettings settings;
    settings.beginGroup("find-replace");

    m_findComboBox->addItem(settings.value("find", QString()).toString());
    m_replaceComboBox->addItem(settings.value("replace", QString()).toString());
    m_historySize = settings.value("history-size", 5).toInt();

    m_caseCheckBox->setChecked(settings.value("case", false).toBool());
    m_wholeWordsCheckBox->setChecked(settings.value("entire-word", false).toBool());
    m_searchBackwardsCheckBox->setChecked(settings.value("backwards", false).toBool());
    m_wrapCheckBox->setChecked(settings.value("wrap", true).toBool());

    m_findWords = settings.value("find-words", QStringList()).toString().split(";");
    m_replaceWords = settings.value("replace-words", QStringList()).toString().split(";");

    settings.endGroup();
}

void FindReplaceDialog::writeSettings()
{
    QSettings settings;
    settings.beginGroup("find-replace");

    settings.setValue("find", m_findComboBox->currentText());
    settings.setValue("replace", m_replaceComboBox->currentText());
    settings.setValue("history-size", m_historySize);

    settings.setValue("case", m_caseCheckBox->isChecked());
    settings.setValue("entire-word", m_wholeWordsCheckBox->isChecked());
    settings.setValue("backwards", m_searchBackwardsCheckBox->isChecked());
    settings.setValue("wrap", m_wrapCheckBox->isChecked());

    m_findWords.removeDuplicates();
    settings.setValue("find-words", m_findWords.join(";"));

    m_replaceWords.removeDuplicates();
    settings.setValue("replace-words", m_replaceWords.join(";"));

    settings.endGroup();
}

void FindReplaceDialog::setTextEditor(QPlainTextEdit *editor)
{
    m_editor = editor;
    connect(m_editor, SIGNAL(copyAvailable(bool)), m_replaceButton, SLOT(setEnabled(bool)));
    connect(m_editor, SIGNAL(copyAvailable(bool)), m_replaceAllButton, SLOT(setEnabled(bool)));
}

bool FindReplaceDialog::find()
{
    if (!m_editor)
        return false;

    appendToHistory(m_findComboBox, m_findWords);

    QTextDocument::FindFlags options = 0;
    if (m_searchBackwardsCheckBox->isChecked())
        options |= QTextDocument::FindBackward;
    if (m_caseCheckBox->isChecked())
        options |= QTextDocument::FindCaseSensitively;
    if (m_wholeWordsCheckBox->isChecked())
        options |= QTextDocument::FindWholeWords;

    QString expr = m_findComboBox->currentText();
    if (!m_editor->find(expr, options))
    {
        if (m_wrapCheckBox->isChecked())
        {
            m_editor->moveCursor(m_searchBackwardsCheckBox->isChecked() ?
                                     QTextCursor::End : QTextCursor::Start);
            if (!m_editor->find(expr, options))
            {
                setStatusTip(tr("\"%1\" not found").arg(expr));
                return false;
            }
        }
        else
        {
            setStatusTip(tr("\"%1\" not found").arg(expr));
            return false;
        }
    }

    setStatusTip("");
    return true;
}

void FindReplaceDialog::appendToHistory(QComboBox *widget, QStringList & history)
{
    if (!widget)
        return;

    QString expr = widget->currentText();
    // append the replaced expression to history
    if (!expr.isEmpty() && !history.contains(expr))
    {
        history.append(expr);
        widget->addItem(expr);
        while (history.size() >= historySize())
        {
            history.removeFirst();
            widget->removeItem(0);
        }
        widget->setCurrentIndex(widget->count()-1);
    }
}

void FindReplaceDialog::replace()
{
    if (!m_editor)
        return;

    appendToHistory(m_replaceComboBox, m_replaceWords);

    if (!cursor().hasSelection())
        find();
    else
        cursor().insertText(m_replaceComboBox->currentText());
}

void FindReplaceDialog::replaceAll()
{
    if (!m_editor)
        return;

    appendToHistory(m_replaceComboBox, m_replaceWords);

    bool wrapMode = m_wrapCheckBox->isChecked();
    if (wrapMode)
    {
        // move the cursor at document start and
        // temporary disable wrap mode to avoid infinite loops
        m_editor->moveCursor(m_searchBackwardsCheckBox->isChecked() ?
                                 QTextCursor::End : QTextCursor::Start);
        m_wrapCheckBox->setChecked(false);
    }

    if (!cursor().hasSelection())
        find();

    cursor().beginEditBlock();
    int count = 0;
    while (cursor().hasSelection())
    {
        cursor().insertText(m_replaceComboBox->currentText());
        find();
        ++count;
    }
    setStatusTip(tr("Replaced %1 occurrence(s)").arg(count));
    cursor().endEditBlock();

    // restore original wrap mode
    m_wrapCheckBox->setChecked(wrapMode);
}

int FindReplaceDialog::historySize() const
{
    return m_historySize;
}

void FindReplaceDialog::setHistorySize(int value)
{
    m_historySize = value;
}

QTextCursor FindReplaceDialog::cursor() const
{
    return m_editor ? m_editor->textCursor() : QTextCursor();
}

void FindReplaceDialog::setStatusTip(const QString & message)
{
    if (!m_editor)
        return;

    m_editor->setStatusTip(message);
}

void FindReplaceDialog::onValueChanged(const QString & text)
{
    QComboBox *currentComboBox = qobject_cast< QComboBox* >(sender());
    if (currentComboBox == m_findComboBox)
        m_findButton->setEnabled(!text.isEmpty());
}
