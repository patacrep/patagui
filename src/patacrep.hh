#ifndef PATACREP_H
#define PATACREP_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "PythonQt.h"
#include "songbook.hh"

class Patacrep : public QObject
{
    Q_OBJECT
public:
    explicit Patacrep(QObject *parent = 0);

    /*! Sets Working directory for process
  */
    void setWorkingDirectory(const QString &dir);

    /*! Test for Python
  */
    bool testPython();

    /*! Test for Patacrep
  */
    bool testPatacrep();

    /*!
     * \brief getSongbook
     * \return Returns songbook
     */
    Songbook *getSongbook() const;

    QStringList getDatadirs() const;

signals:
    void aboutToStart();
    void finished();
    void message(const QString &message, int timeout);

public slots:
    /*!
     * \brief setSongbook
     * \param value
     */

    void setSongbook(Songbook *value);

    void setDatadirs(const QStringList &datadirs);

    void addDatadir(const QString &datadir);

    void stopBuilding();

    void buildSongbook();

private:
    PythonQtObjectPtr pythonModule;
    Songbook *songbook;
    QStringList datadirs;
};

#endif // PATACREP_H
