#include "patacrep.hh"

Patacrep::Patacrep(QObject *parent) : QObject(parent)
{
    // Setup Python interpreter
    PythonQt::init(PythonQt::RedirectStdOut);
    pythonModule = PythonQt::self()->getMainModule();
    // Redirect Std Out and Std Err to qDebug output through slot for easier
    // debug.
    connect(PythonQt::self(), SIGNAL(pythonStdOut(QString)),
            SLOT(stdOut(QString)));
    connect(PythonQt::self(), SIGNAL(pythonStdErr(QString)),
            SLOT(stdErr(QString)));
    // Import Python file containing all necessary functions and imports
    pythonModule.evalFile(":/python_scripts/songbook.py");
}

void Patacrep::setWorkingDirectory(const QString &dir)
{
    // Set Current directory in python
}

const bool Patacrep::testPython()
{
    //FIXME Test python
    return true;
}

Songbook *Patacrep::getSongbook() const
{
    return songbook;
}

void Patacrep::setSongbook(Songbook *value)
{
    songbook = value;
}

