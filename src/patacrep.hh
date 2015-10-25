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

    /*!
    Return the songbook to compile.
    \sa setSongbook
  */
    const Songbook* songbook() const;

    /*! Sets Working directory for process
  */
    void setWorkingDirectory(const QString &dir);

    /*! Test for Python
  */
    const bool testPython();

signals:

public slots:    

private:
    QStringList datadirs;
    PythonQtObjectPtr pythonModule;
};

#endif // PATACREP_H
