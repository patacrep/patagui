/****************************************************************************
 **
 ** Copyright (C) 2006 Trolltech ASA. All rights reserved.
 ** Copyright (C) 2012, Romain Goffe <romain.goffe@gmail.com>
 ** Copyright (C) 2012, Alexandre Dupas <alexandre.dupas@gmail.com>
 **
 ** This file is part of the documentation of Qt. It was originally
 ** published as part of Qt Quarterly.
 **
 ** This file may be used under the terms of the GNU General Public License
 ** version 2.0 as published by the Free Software Foundation or under the
 ** terms of the Qt Commercial License Agreement. The respective license
 ** texts for these are provided with the open source and commercial
 ** editions of Qt.
 **
 ** If you are unsure which license is appropriate for your use, please
 ** review the following information:
 ** http://www.trolltech.com/products/qt/licensing.html or contact the
 ** sales department at sales@trolltech.com.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** Modified to serve personnal purposes.
 **
 ****************************************************************************/

#ifndef __VARIANT_FACTORY_HH__
#define __VARIANT_FACTORY_HH__

#include "qtvariantproperty.h"

class FileChooser;
class QSpinBox;

class VariantFactory : public QtVariantEditorFactory
{
    Q_OBJECT
public:
    VariantFactory(QObject *parent = 0) : QtVariantEditorFactory(parent) {}

    virtual ~VariantFactory();

protected:
    virtual void connectPropertyManager(QtVariantPropertyManager *manager);
    virtual QWidget *createEditor(QtVariantPropertyManager *manager,
                                  QtProperty *property, QWidget *parent);
    virtual void disconnectPropertyManager(QtVariantPropertyManager *manager);
private slots:
    void slotPropertyChanged(QtProperty *property, const QVariant &value);
    void slotPropertyAttributeChanged(QtProperty *property,
                                      const QString &attribute,
                                      const QVariant &value);
    void slotSetValue(const QString &value);
    void slotSetIntValue(int);
    void slotEditorDestroyed(QObject *object);

private:
    QMap<QtProperty *, QList<FileChooser *> > theCreatedEditors;
    QMap<FileChooser *, QtProperty *> theEditorToProperty;
    QMap<QtProperty *, QList<QSpinBox *> > theCreatedSpinBoxes;
    QMap<QSpinBox *, QtProperty *> theSpinBoxToProperty;
};

#endif // __VARIANT_FACTORY_HH__
