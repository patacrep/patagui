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
#include "songbook.hh"

#include <QDir>
#include <QFile>
#include <QMessageBox>

#include <QByteArray>
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QVariantList>

#include <QtGroupBoxPropertyBrowser>
#include <QtAbstractPropertyManager>

#include "qtpropertymanager.h"
#include "variant-manager.hh"
#include "variant-factory.hh"

#include "library.hh"

#include <QDebug>

Songbook::Songbook(QObject *parent)
    : IdentityProxyModel(parent)
    , m_filename()
    , m_tmpl()
    , m_selectedSongs()
    , m_songs()
    , m_modified()
    , m_propertyManager(new VariantManager())
    , m_groupManager()
    , m_parameters()
    , m_mandatoryParameters()
    , m_advancedParameters()
{
    setSourceModel(library());
}

Songbook::~Songbook()
{
    delete m_propertyManager;
}

Library *Songbook::library() const
{
    return Library::instance();
}

QString Songbook::filename() const
{
    return m_filename;
}

void Songbook::setFilename(const QString &filename)
{
    m_filename = filename;
    // ensure the .sb extension is present
    if (!filename.endsWith(".sb"))
        m_filename += ".sb";
}

bool Songbook::isModified()
{
    return m_modified;
}

void Songbook::setModified(bool modified)
{
    m_modified = modified;
    emit(wasModified(modified));
}

QString Songbook::tmpl() const
{
    return m_tmpl;
}

void Songbook::setTmpl(const QString &tmpl)
{
    int index = library()->templates().indexOf(tmpl);
    if (m_tmpl != tmpl && -1 != index) {
        m_tmpl = tmpl;
        changeTemplate(tmpl);
        setModified(true);
    }
}

QStringList Songbook::songs()
{
    return m_songs;
}

void Songbook::setSongs(QStringList songs)
{
    if (m_songs != songs) {
        setModified(true);
        m_songs = songs;
        emit(songsChanged());
    }
}

QStringList Songbook::datadirs()
{
    return QStringList(library()->directory().canonicalPath());
}

void Songbook::setDatadirs(QStringList datadirs)
{
    if (m_datadirs != datadirs) {
        m_datadirs = datadirs;
    }
}

void Songbook::reset()
{
    setFilename(QString());

    QMap<QString, QtVariantProperty *>::const_iterator it;
    for (it = m_parameters.constBegin(); it != m_parameters.constEnd(); ++it)
        it.value()->setValue(QVariant(""));

    setModified(false);
}

void Songbook::changeTemplate(const QString &filename)
{
    QString templateFilename("patacrep.tex");
    if (!filename.isEmpty())
        templateFilename = filename;

    QString json;

    // reserved template parameters
    QStringList reservedParameters;
    reservedParameters << "name"
                       << "template"
                       << "content";

    // read template file
    QFile file(
        QString("%1/templates/%2").arg(workingPath()).arg(templateFilename));
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        in.setCodec("UTF-8");
        QRegExp jsonFilter("^%%:");
        QString line;
        json = "(";
        do {
            line = in.readLine();
            if (line.startsWith("%%:"))
                json += line.remove(jsonFilter) + "\n";

        } while (!line.isNull());
        json += ")";
        file.close();
    }

    /* FIXME: Switch to non QScript stuff // Use variables read in patacrep!!!
    // Load json encoded songbook data
    QScriptEngine engine;

    // check syntax
    QScriptSyntaxCheckResult res = QScriptEngine::checkSyntax(json);
    if (res.state() != QScriptSyntaxCheckResult::Valid)
    {
        qDebug() << "Songbook::changeTemplate : Error line "<<
    res.errorLineNumber()
                 << " column " << res.errorColumnNumber()
                 << ":" << res.errorMessage();
        return;
    }

    // evaluate the json data
    QScriptValue parameters = engine.evaluate(json);

    // load parameters data
    if (parameters.isValid() && parameters.isArray())
    {
        QScriptValue svName;
        QScriptValue svDescription;
        QScriptValue svType;
        QScriptValue svDefault;
        QScriptValue svValues;
        QScriptValue svMandatory;

        int propertyType;

        QMap< QString, QVariant > oldValues;
        {
            QMap< QString, QtVariantProperty* >::const_iterator it =
    m_parameters.constBegin();
            while (it != m_parameters.constEnd())
            {
                oldValues.insert(it.key(), it.value()->value());
                ++it;
            }
            m_parameters.clear();
            m_propertyManager->clear();
        }

        QtVariantProperty *item;
        QScriptValueIterator it(parameters);
        bool advancedParameters = false;

        m_mandatoryParameters.clear();

        delete m_groupManager;
        m_groupManager = new QtGroupPropertyManager(this);
        m_advancedParameters = m_groupManager->addProperty(tr("Advanced
    Parameters"));

        while (it.hasNext())
        {
            it.next();

            if (it.flags() & QScriptValue::SkipInEnumeration)
                continue;

            svName = it.value().property("name");
            if (!reservedParameters.contains(svName.toString()))
            {
                QVariant oldValue;
                QStringList stringValues;

                svDescription = it.value().property("description");
                svDefault = it.value().property("default");
                svType = it.value().property("type");
                svValues = it.value().property("values");
                svMandatory = it.value().property("mandatory");

                // determine property type
                if (svType.toString() == QString("string"))
                    propertyType = QVariant::String;
                else if (svType.toString() == QString("color"))
                    propertyType = QVariant::Color;
                else if (svType.toString() == QString("enum"))
                    propertyType = QtVariantPropertyManager::enumTypeId();
                else if (svType.toString() == QString("flag"))
                    propertyType = QtVariantPropertyManager::flagTypeId();
                else if (svType.toString() == QString("font"))
                    propertyType = VariantManager::unitTypeId();
                else if (svType.toString() == QString("file"))
                    propertyType = VariantManager::filePathTypeId();
                else
                    propertyType = QVariant::String;

                // add new property
                item = m_propertyManager
                        ->addProperty(propertyType, svDescription.toString());

                // retrieve existing or default value
                if (oldValues.contains(svName.toString()))
                {
                    oldValue = oldValues.value(svName.toString());
                }
                else if (svDefault.isValid())
                {
                    oldValue = svDefault.toVariant();
                }

                if (propertyType == QtVariantPropertyManager::enumTypeId())
                {
                    qScriptValueToSequence(svValues, stringValues);
                    m_propertyManager->setAttribute(item, "enumNames",
                                                    QVariant(stringValues));
                    // handle existing or default value in case of enum
                    if (oldValue.isValid() && oldValue.type() ==
    QVariant::String)
                    {
                        oldValue =
    QVariant(stringValues.indexOf(oldValue.toString()));
                    }
                }
                else if (propertyType == QtVariantPropertyManager::flagTypeId())
                {
                    qScriptValueToSequence(svValues, stringValues);
                    m_propertyManager->setAttribute(item, "flagNames",
                                                    QVariant(stringValues));
                    // handle existing or default value in case of flag
                    if (oldValue.isValid() && oldValue.type() == QVariant::List)
                    {
                        QStringList activatedFlags;
                        qScriptValueToSequence(svDefault, activatedFlags);
                        int flags = 0;
                        int index = 1;
                        for (int i = 0; i < stringValues.size(); ++i)
                        {
                            if (activatedFlags.contains(stringValues.at(i)))
                            {
                                flags |= index;
                            }
                            index *= 2;
                        }
                        oldValue = QVariant(flags);
                    }
                }
                // set the existing or default value
                if (oldValue.isValid())
                    item->setValue(oldValue);

                if (propertyType == VariantManager::filePathTypeId())
                {
                    if (svName.toString() == "picture")
                        item->setAttribute("filter", "Image files (*.jpg)");
                    else if (svName.toString() == "license")
                        item->setAttribute("filter", "TeX files (*.tex)");
                }
                if (propertyType == VariantManager::unitTypeId())
                {
                    if (svName.toString() == "mainfontsize")
                        item->setAttribute("unit", "pt");
                }

                // insert the property into the list of parameters
                m_parameters.insert(svName.toString(), item);

                // handle the mandatory boolean parameter
                if (svMandatory.isValid() && svMandatory.toBool())
                {
                    m_mandatoryParameters << item;
                }
                else
                {
                    advancedParameters = true;
                    m_advancedParameters->addSubProperty(item);
                }
            }
        }
        if (advancedParameters)
        {
            m_mandatoryParameters << m_advancedParameters;
        }
    }
    */
}

void Songbook::initializeEditor(QtGroupBoxPropertyBrowser *editor)
{
    editor->setFactoryForManager(m_propertyManager, new VariantFactory());

    QtProperty *item;
    foreach (item, m_mandatoryParameters) {
        editor->addProperty(item);
    }
}

void Songbook::save(const QString &filename)
{
    // get the song list in the correct format from the selected songs
    songsFromSelection();
    // write the songbook
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // Start Json Objecy and fill it
        QJsonObject json;
        // TODO Check Template insertion
        if (!tmpl().isEmpty()) {
            json.insert("template", tmpl());
        } else {
            json.insert("template", "patacrep.tex");
        }
        json.insert("lang", "french");
        // Book Options
        QJsonArray bookoptions;
        bookoptions.append("diagram");
        // bookoptions.append("lilypond");
        bookoptions.append("pictures");
        json.insert("bookoptions", bookoptions);
        // Authwords
        QJsonObject authwords;
        authwords.insert("sep", "");
        json.insert("authwords", authwords);
        // Datadirs. For now, only library path, later maybe other paths.
        json.insert("datadir", library()->directory().absolutePath());
        // Songs
        QJsonArray songlist;
        foreach (QString song, songs()) {
            songlist.append(song);
        }
        json.insert("content", songlist);
        file.write(QJsonDocument(json).toJson());

        file.close();
        setModified(false);
        setFilename(filename);

    } else {
        qWarning() << "Could not open File: " + filename;
    }
}

void Songbook::load(const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        // Read File
        QByteArray rawData = file.readAll();

        // Parse document
        QJsonParseError error;
        QJsonDocument doc(QJsonDocument::fromJson(rawData, &error));

        // Get JSON object
        QJsonObject json = doc.object();

        // load data into this object
        if (!json.isEmpty()) // i.e. JsonObject is valid, parsing went fine
        {
            QJsonValue jsonvalue;
            // template property
            jsonvalue = json.value("template");
            if (!(jsonvalue.isNull())) {
                setTmpl(jsonvalue.toString());
            }

            /* FIXME Template interpreter */
            // template specific properties
            QtVariantProperty *property;
            int type;
            QVariant value;
            QMap<QString, QtVariantProperty *>::const_iterator it;
            for (it = m_parameters.constBegin(); it != m_parameters.constEnd();
                 ++it) {
                jsonvalue = json.value(it.key());
                if (!(jsonvalue.isNull())) {
                    property = it.value();
                    type = m_propertyManager->propertyType(property);
                    value = jsonvalue.toVariant();
                    QVariant stringValues;

                    if (type == QtVariantPropertyManager::enumTypeId()) {
                        stringValues = m_propertyManager->attributeValue(
                            property, "enumNames");
                        value = QVariant(stringValues.toStringList().indexOf(
                            value.toString()));
                    } else if (type == QtVariantPropertyManager::flagTypeId()) {
                        stringValues = m_propertyManager->attributeValue(
                            property, "flagNames");
                        QStringList flagValues = stringValues.toStringList();
                        QStringList activatedFlags = value.toStringList();
                        int flags = 0;
                        int index = 1;
                        for (int i = 0; i < flagValues.size(); ++i) {
                            if (activatedFlags.contains(flagValues.at(i))) {
                                flags |= index;
                            }
                            index *= 2;
                        }
                        value = QVariant(flags);
                    }

                    m_propertyManager->setValue(property, value);
                }
            }

            // songs property (if not an array, the value can be "all")
            jsonvalue = json.value("content");
            if (!jsonvalue.isNull()) {
                QStringList items;
                if (!jsonvalue.isArray()) {
                    qDebug() << "Songbook::load : not implemented yet";
                } else {
                    // Convert QVariantList to QStringList by casting one by one
                    foreach (QVariant var,
                             jsonvalue.toArray().toVariantList()) {
                        items << var.toString();
                    }
                }
                setSongs(items);
            }
        } else {
            // TODO Treat error properly
            qDebug() << error.errorString();
        }

        songsToSelection();
        file.close();
        setModified(false);
        setFilename(filename);
    } else {
        // TODO Transfer error message to mainwindow
        qWarning() << "Songbook::load : unable to open file in read mode";
    }
}

QString Songbook::workingPath() const
{
    return library()->directory().canonicalPath();
}

bool Songbook::isChecked(const QModelIndex &index)
{
    return m_selectedSongs[index.row()];
}

void Songbook::setChecked(const QModelIndex &index, bool checked)
{
    if (isChecked(index) != checked) {
        m_selectedSongs[index.row()] = checked;
        emit(dataChanged(index, index));
    }
}

void Songbook::toggle(const QModelIndex &index)
{
    m_selectedSongs[index.row()] = !m_selectedSongs[index.row()];
    emit(dataChanged(index, index));
}

void Songbook::checkAll()
{
    for (int i = 0; i < m_selectedSongs.size(); ++i) {
        m_selectedSongs[i] = true;
    }
    emit(dataChanged(index(0, 0), index(m_selectedSongs.size() - 1, 0)));
}

void Songbook::uncheckAll()
{
    for (int i = 0; i < m_selectedSongs.size(); ++i) {
        m_selectedSongs[i] = false;
    }
    emit(dataChanged(index(0, 0), index(m_selectedSongs.size() - 1, 0)));
}

void Songbook::toggleAll()
{
    for (int i = 0; i < m_selectedSongs.size(); ++i) {
        m_selectedSongs[i] = !m_selectedSongs[i];
    }
    emit(dataChanged(index(0, 0), index(m_selectedSongs.size() - 1, 0)));
}

int Songbook::selectedCount() const
{
    int count = 0;
    for (int i = 0; i < m_selectedSongs.size(); ++i) {
        if (m_selectedSongs[i])
            count++;
    }
    return count;
}

void Songbook::songsFromSelection()
{
    m_songs.clear();
    QString song;
    for (int i = 0; i < m_selectedSongs.size(); ++i) {
        if (m_selectedSongs[i]) {
            song = data(index(i, 0), Library::RelativePathRole).toString();
#ifdef Q_WS_WIN
            song.replace("\\", "/");
#endif
            m_songs << song;
        }
    }
}

void Songbook::songsToSelection()
{
    if (m_songs.isEmpty())
        uncheckAll();

    for (int i = 0; i < m_selectedSongs.size(); ++i) {
        m_selectedSongs[i] = false;
        if (m_songs.contains(
                data(index(i, 0), Library::RelativePathRole).toString()))
            m_selectedSongs[i] = true;
    }
    emit(dataChanged(index(0, 0), index(m_selectedSongs.size() - 1, 0)));
}

void Songbook::selectLanguages(const QStringList &languages)
{
    for (int i = 0; i < m_selectedSongs.size(); ++i) {
        m_selectedSongs[i] = false;
        if (languages.contains(
                data(index(i, 0), Library::LanguageRole).toString()))
            m_selectedSongs[i] = true;
    }
    emit(dataChanged(index(0, 0), index(m_selectedSongs.size() - 1, 0)));
}

QVariant Songbook::data(const QModelIndex &index, int role) const
{
    if (index.column() == 0 && role == Qt::CheckStateRole) {
        return (m_selectedSongs[index.row()] ? Qt::Checked : Qt::Unchecked);
    }
    return IdentityProxyModel::data(index, role);
}

Qt::ItemFlags Songbook::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsUserCheckable | IdentityProxyModel::flags(index);
}

bool Songbook::setData(const QModelIndex &index, const QVariant &value,
                       int role)
{
    if (index.column() == 0 && role == Qt::CheckStateRole) {
        m_selectedSongs[index.row()] = value.toBool();
        emit(dataChanged(index, index));
        return true;
    }
    return IdentityProxyModel::setData(index, value, role);
}

void Songbook::sourceModelAboutToBeReset()
{
    songsFromSelection();
    beginResetModel();
}

void Songbook::sourceModelReset()
{
    m_selectedSongs.clear();
    for (int i = 0; i < sourceModel()->rowCount(); ++i) {
        m_selectedSongs << false;
    }
    songsToSelection();
    endResetModel();
}
