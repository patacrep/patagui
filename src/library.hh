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

#ifndef __LIBRARY_HH__
#define __LIBRARY_HH__

#include "song.hh"
#include "singleton.hh"

#include <QAbstractTableModel>
#include <QString>
#include <QDir>
#include <QLocale>
#include <QMetaType>

class QAbstractListModel;
class QStringListModel;

class QPixmap;
class CProgressBar;
class CMainWindow;

/*!
  \file library.hh
  \class CLibrary
  \brief CLibrary is the base model that corresponds to the list of songs

  A CLibrary is a list of Song objects (structure representing .sg
  files) that are fetched from a local directory.

  This model is used to build an intermediate model
  (CSongSortFilterProxyModel) that allows filtering options, and is
  then presented in the library tab (CTabWidget) of the main window
  (CMainWindow) through its associated view (CLibraryView).

*/

class CLibrary : public QAbstractTableModel, public Singleton<CLibrary>
{
    Q_OBJECT
    Q_PROPERTY(QDir directory READ directory WRITE setDirectory)

    friend class Singleton<CLibrary>;

private:
    /// Constructor.
    CLibrary();
    /// Destructor.
    ~CLibrary();

public:
    /*!
    \enum Roles
    Each Song in the CLibrary has a set of data elements associated with it, each with its own role.
    The roles are used by the view to indicate to the model which type of data it needs.
  */
    enum Roles {
        TitleRole = Qt::UserRole + 1, /*!< the title of the song item.*/
        ArtistRole = Qt::UserRole + 2, /*!< the artist of the song item.*/
        AlbumRole = Qt::UserRole + 3, /*!< the album of the song item.*/
        CoverRole = Qt::UserRole + 4, /*!< the cover of the song item.*/
        LilypondRole = Qt::UserRole + 5, /*!< whether or not the song item contains lilypond music sheets.*/
        WebsiteRole = Qt::UserRole + 6, /*!< whether or not the song item contains a link to the website of the artist.*/
        UrlRole = Qt::UserRole + 7, /*!< the url of the artist website.*/
        LanguageRole = Qt::UserRole + 8, /*!< the language of the song item.*/
        PathRole = Qt::UserRole + 9, /*!< the absolute path to the .sg file corresponding to the song item.*/
        CoverSmallRole = Qt::UserRole + 10, /*!< the thumbnail cover (22x22) of the song item.*/
        CoverFullRole = Qt::UserRole + 11, /*!< the full cover (128x128) of the song item.*/
        RelativePathRole = Qt::UserRole + 12, /*!< the relative path to the .sg file corresponding to the song item (from the base directory of the songbook).*/
        MaxRole = RelativePathRole
    };

    void writeSettings();


    /*!
    Returns the directory of the library.
    \sa setDirectory
  */
    QDir directory() const;

    /*!
    Sets \a directory as the directory for the library.
    \sa directory
  */
    void setDirectory(const QString &directory);

    /*!
    Sets \a directory as the directory for the library.
    \sa directory
  */
    void setDirectory(const QDir &directory);

    /*!
    Returns the list of available templates (*.tmpl files).
  */
    QStringList templates() const;

    /*!
    Returns the completion model associated with the library.
    The completion model is based on the list of words from
    title, artist and album columns.
    \sa artistCompletionModel, albumCompletionModel, urlCompletionModel
  */
    QAbstractListModel * completionModel() const;

    /*!
    Returns a model based on the list (without duplicates) of artists.
    \sa completionModel, albumCompletionModel, urlCompletionModel
  */
    QAbstractListModel * artistCompletionModel() const;

    /*!
    Returns a model based on the list (without duplicates) of albums.
    \sa completionModel, artistCompletionModel, urlCompletionModel
  */
    QAbstractListModel * albumCompletionModel() const;

    /*!
    Returns a model based on the list (without duplicates) of urls.
    \sa completionModel, artistCompletionModel, albumCompletionModel
  */
    QAbstractListModel * urlCompletionModel() const;

    /*!
    Reimplements QAbstractTableModel::headerData.
    \sa data
  */
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    /*!
    Reimplements QAbstractTableModel::data.
    Returns the data at position \a index for the given \a role.
    \sa headerData
  */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    /*!
    Reimplements QAbstractTableModel::rowCount.
    Returns the number of Song items in the library.
    \sa columnCount
  */
    virtual int rowCount(const QModelIndex &index = QModelIndex()) const;

    /*!
    Reimplements QAbstractTableModel::columnCount.
    Returns \a 7 as the number of columns for a song item
    (title, artist, path, album, lilypond, website, language).
    \sa rowCount
  */
    virtual int columnCount(const QModelIndex &index = QModelIndex()) const;

    /*!
    Returns the absolute path of an .sg file from \a artist and \a title names.
    Following the songbook convention, the song "Hello world!" from artist "Unknown" would
    be located in \a /path/to/songbook/songs/unknown/hello_world.sg".
   */
    QString pathToSong(const QString &artist, const QString &title) const;

    /*!
    This is a convenience method that returns the absolute path of
    an .sg file from a Song object.
  */
    QString pathToSong(Song &song) const;

    /*!
    Adds a the song \a song to the library.
    If \a reset is \a true, the whole library is parsed again.
    \sa addSongs
  */
    void addSong(const Song &song, bool reset=false);

    /*!
    A Song object is built from the file \a path
    and is added to the library.
    \sa addSongs
  */
    void addSong(const QString &path);

    /*!
    Song objects are built from the files in \a paths
    and are added to the library.
    \sa addSong
  */
    void addSongs(const QStringList &paths);

    void importSongs(const QStringList & filenames);

    /*!
    Returns \a true if the song \a path is already in the library.
    \sa addSong, removeSong
  */
    bool containsSong(const QString &path);

    /*!
    Removes the song \a path from the library.
    \sa addSong, addSongs
  */
    void removeSong(const QString &path);

    /*! Returns the index of the song \a path
    from the library.
    \sa getSong
  */
    int getSongIndex(const QString &path) const;

    /*!
    Returns the Song object whose path is \a path from the library.
    \sa getSongIndex
  */
    Song getSong(const QString &path) const;

    /*!
    Loads a Song object in the library from the file \a path.
  */
    void loadSong(const QString &path, Song *song);

    /*!
    Creates the artist subdirectory for the song \a song (if required).
  */
    void createArtistDirectory(Song &song);

    /*!
    Saves the song \a song in the library (update the song path if required).
    \sa saveCover
  */
    void saveSong(Song &song);

    /*!
    Saves the cover \a cover in the library directory (update the cover path if required).
    \sa saveSong
  */
    void saveCover(Song &song, const QImage &cover);

    /*!
    Removes the file \a path from the library.
  */
    void deleteSong(const QString &path);

    static QString checkPath(const QString & path);

    static void recursiveFindFiles(const QString & path, const QStringList& filters, QStringList& files);

    CProgressBar *progressBar() const;
    void showMessage(const QString &);

    CMainWindow* parent() const;
    void setParent(CMainWindow* parent);

public slots:
    void readSettings();
    void update();

signals:
    void wasModified();
    void directoryChanged(const QDir &directory);
    void noDirectory();

protected:

private:
    CMainWindow *m_parent;
    bool checkSongbookPath(const QString & path);
    QString findSongbookPath();

    QDir m_directory;

    QStringListModel *m_completionModel;
    QStringListModel *m_artistCompletionModel;
    QStringListModel *m_albumCompletionModel;
    QStringListModel *m_urlCompletionModel;

    QStringList m_templates;
    QList< Song > m_songs;
};

Q_DECLARE_METATYPE(QLocale::Language)

#endif // __LIBRARY_HH__
