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

#include "conflict-dialog.hh"
#include "song.hh"

#include "diff_match_patch/diff_match_patch.h"

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QPixmap>
#include <QPixmapCache>
#include <QDesktopServices>
#include <QCryptographicHash>
#include <QWizard>
#include <QPlainTextEdit>

#include <QDebug>

CConflictDialog::CConflictDialog(QWidget *parent)
    : QDialog(parent)
    , m_conflictsFound(false)
    , m_conflictView(new QTableWidget(10, 2, this))
    , m_titleLabel(new QLabel)
    , m_artistLabel(new QLabel)
    , m_albumLabel(new QLabel)
    , m_coverLabel(new QLabel)
    , m_pixmap(new QPixmap(42, 42))
    , m_fileCopier(new CFileCopier(parent))
{
    setWindowTitle(tr("Resolve conflicts"));
    setParent(static_cast<CMainWindow*>(parent));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(rejected()), SLOT(close()));

    m_overwriteButton = new QPushButton(tr("Overwrite"), this);
    connect(m_overwriteButton, SIGNAL(clicked()), SLOT(resolve()));
    buttonBox->addButton(m_overwriteButton, QDialogButtonBox::ActionRole);

    m_keepOriginalButton = new QPushButton(tr("Preserve"), this);
    connect(m_keepOriginalButton, SIGNAL(clicked()), SLOT(resolve()));
    buttonBox->addButton(m_keepOriginalButton, QDialogButtonBox::ActionRole);

    m_diffButton = new QPushButton(tr("Show differences"), this);
    connect(m_diffButton, SIGNAL(clicked()), SLOT(showDiff()));
    buttonBox->addButton(m_diffButton, QDialogButtonBox::ActionRole);

    connect(progressBar(), SIGNAL(canceled()), SLOT(cancelCopy()));

    m_conflictView->setColumnWidth(0, 290);
    m_conflictView->setColumnWidth(1, 290);
    m_conflictView->horizontalHeader()->setStretchLastSection(true);
    m_conflictView->setAlternatingRowColors(true);
    m_conflictView->setHorizontalHeaderLabels(QStringList() << tr("Source") << tr("Target"));
    m_conflictView->verticalHeader()->setVisible(false);
    m_conflictView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(m_conflictView, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(updateItemDetails(QTableWidgetItem*)));
    connect(m_conflictView, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(openItem(QTableWidgetItem*)));

    QLabel *messageLabel = new QLabel;
    messageLabel->setText(tr("Importing the following source items would overwrite those target items: "));
    messageLabel->setWordWrap(true);
    QIcon warningIcon = QIcon::fromTheme("dialog-warning");
    QLabel *iconLabel = new QLabel;
    iconLabel->setPixmap(warningIcon.pixmap(32, 32));
    QHBoxLayout *warningLayout = new QHBoxLayout;
    warningLayout->addWidget(iconLabel);
    warningLayout->addWidget(messageLabel, 1);
    warningLayout->addStretch();

    QHBoxLayout *detailsLayout = new QHBoxLayout;

    QFormLayout *infoLayout = new QFormLayout;
    infoLayout->addRow(tr("Title:"), m_titleLabel);
    infoLayout->addRow(tr("Artist:"), m_artistLabel);
    infoLayout->addRow(tr("Album:"), m_albumLabel);

    QVBoxLayout *coverLayout = new QVBoxLayout;
    coverLayout->addWidget(m_coverLabel);
    coverLayout->addStretch();

    detailsLayout->addLayout(infoLayout);
    detailsLayout->addStretch();
    detailsLayout->addLayout(coverLayout);

    QBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(warningLayout);
    mainLayout->addWidget(m_conflictView);
    mainLayout->addLayout(detailsLayout);
    mainLayout->addWidget(buttonBox);

    setFixedWidth(600);
    setLayout(mainLayout);
}

CConflictDialog::~CConflictDialog()
{
    delete m_conflictView;
    delete m_titleLabel;
    delete m_artistLabel;
    delete m_albumLabel;
    delete m_coverLabel;
    delete m_pixmap;
    delete m_fileCopier;
}

void CConflictDialog::setParent(CMainWindow* parent)
{
    m_parent = parent;
}

CMainWindow* CConflictDialog::parent() const
{
    return m_parent;
}

CProgressBar* CConflictDialog::progressBar() const
{
    return parent()->progressBar();
}

void CConflictDialog::showMessage(const QString & message)
{
    parent()->statusBar()->showMessage(message);
}

void CConflictDialog::updateItemDetails(QTableWidgetItem* item)
{
    QString path = item->data(Qt::ToolTipRole).toString();
    QFileInfo fi(path);

    Song song = Song::fromFile(path);
    m_titleLabel->setText(song.title);
    m_artistLabel->setText(song.artist);
    m_albumLabel->setText(song.album);

    QString cover = QString("%1/%2.jpg").arg(fi.absolutePath()).arg(song.coverName);
    if (QFile(cover).exists())
    {
        m_pixmap->load(cover);
        *m_pixmap = m_pixmap->scaled(42, 42);
    }
    else if (!QPixmapCache::find("cover-missing-full", m_pixmap))
    {
        *m_pixmap = QIcon::fromTheme("image-missing", QIcon(":/icons/tango/32x32/status/image-missing.png")).pixmap(42, 42);
        QPixmapCache::insert("cover-missing-full", *m_pixmap);
    }

    m_coverLabel->setPixmap(*m_pixmap);
}

void CConflictDialog::openItem(QTableWidgetItem* item)
{
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(Qt::ToolTipRole).toString())) &&
            !QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(Qt::DisplayRole).toString())) &&
            !QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(Qt::EditRole).toString())) &&
            !QDesktopServices::openUrl(QUrl::fromLocalFile(item->data(Qt::WhatsThisRole).toString())) )
    {
        showMessage(tr("Can't open: %1").arg(item->data(Qt::DisplayRole).toString()));
    }
}

void CConflictDialog::setSourceTargetFiles(const QMap< QString, QString > &files)
{ 
    m_conflictsFound = false;
    m_conflictView->setRowCount(files.size());
    int row = 0;
    QMap<QString, QString>::const_iterator it = files.constBegin();
    while (it != files.constEnd())
    {
        if (QFile(it.value()).exists())
        {
            QFile source(it.key());
            QFile target(it.value());

            if (source.open(QIODevice::ReadOnly) && target.open(QIODevice::ReadOnly))
            {
                QCryptographicHash sourceHash(QCryptographicHash::Sha1);
                sourceHash.addData(source.readAll());
                source.close();

                QCryptographicHash targetHash(QCryptographicHash::Sha1);
                targetHash.addData(target.readAll());
                target.close();

                if (sourceHash.result() != targetHash.result())
                {
                    m_conflictsFound = true;
                    m_conflicts.insert(it.key(), it.value());

                    QFileInfo fileInfo(it.key());
                    QTableWidgetItem *srcItem = new QTableWidgetItem;
                    srcItem->setIcon(QIcon(":/icons/songbook/48x48/song.png"));
                    srcItem->setData(Qt::DisplayRole, fileInfo.fileName());
                    srcItem->setData(Qt::ToolTipRole, fileInfo.absoluteFilePath());
                    m_conflictView->setItem(row, 0, srcItem);

                    fileInfo = QFileInfo(it.value());
                    QTableWidgetItem *targetItem = new QTableWidgetItem;
                    targetItem->setIcon(QIcon(":/icons/songbook/48x48/song.png"));
                    targetItem->setData(Qt::DisplayRole, fileInfo.fileName());
                    targetItem->setData(Qt::ToolTipRole, fileInfo.absoluteFilePath());
                    m_conflictView->setItem(row, 1, targetItem);
                    ++row;
                }
                else
                {
                    m_noConflicts.insert(it.key(), it.value());
                }
            }
        }
        ++it;
    }

    m_conflictView->setRowCount(row);
    if (row > 0)
        updateItemDetails(m_conflictView->itemAt(0, 0));
}

bool CConflictDialog::conflictsFound() const
{
    return m_conflictsFound;
}

void CConflictDialog::cancelCopy()
{
    m_fileCopier->setCancelCopy(true);
}

void CConflictDialog::showDiff()
{
    QWizard *wizard = new QWizard(this);
    wizard->setWindowTitle("Show differences");

    QMap<QString, QString>::const_iterator it = m_conflicts.constBegin();
    while (it != m_conflicts.constEnd())
    {
        QFile source(it.key());
        QFile target(it.value());

        if (source.open(QIODevice::ReadOnly) && target.open(QIODevice::ReadOnly))
        {
            // retrieve source song infos
            Song song = Song::fromFile(it.key());
            QFileInfo fi(it.key());
            QString cover = QString("%1/%2.jpg").arg(fi.absolutePath()).arg(song.coverName);
            QPixmap *pixmap = 0;
            if (QFile(cover).exists())
            {
                pixmap = new QPixmap(cover);
                *pixmap = pixmap->scaled(42, 42);
            }

            // retrieve diffs info
            diff_match_patch* dmp = new diff_match_patch();
            QList<Diff> diffList = dmp->diff_main(source.readAll(), target.readAll());
            source.close();
            target.close();
            dmp->diff_cleanupSemantic(diffList);

            // setup wizard page
            QWizardPage *page = new QWizardPage;
            page->setTitle(song.title);
            page->setSubTitle(song.artist);
            if (pixmap && !pixmap->isNull())
                page->setPixmap(QWizard::LogoPixmap, *pixmap);

            QPlainTextEdit* area = new QPlainTextEdit;
            area->setReadOnly(true);
            area->appendHtml(dmp->diff_prettyHtml(diffList));
            area->moveCursor(QTextCursor::Start);

            QVBoxLayout *layout = new QVBoxLayout;
            layout->addWidget(area);
            page->setLayout(layout);
            wizard->addPage(page);
        }
        ++it;
    }

    wizard->show();
}

bool CConflictDialog::resolve()
{
    QPushButton *button = qobject_cast<QPushButton*>(QObject::sender());
    if (button == m_overwriteButton)
    {
        QMap<QString, QString>::const_iterator it = m_conflicts.constBegin();
        while (it != m_conflicts.constEnd())
        {
            QFile target(it.value());
            if (!target.remove())
            {
                parent()->statusBar()->showMessage
                        (tr("An unexpected error occurred while removing: %1")
                         .arg(target.fileName()));
            }
            ++it;
        }
        m_fileCopier->setSourceTargets(m_conflicts.unite(m_noConflicts));
    }
    else if (button == m_keepOriginalButton)
    {
        m_fileCopier->setSourceTargets(m_noConflicts);
    }

    m_fileCopier->copy();

    accept();
    return true;
}
