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
    const bool testPython();

    /*! Test for Patacrep
  */
    const bool testPatacrep();

    Songbook *getSongbook() const;

    void setSongbook(Songbook *value);

signals:

public slots:

private:
    PythonQtObjectPtr pythonModule;
    Songbook *songbook;
    QStringList datadirs;
};

#endif // PATACREP_H
