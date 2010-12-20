#ifndef __UNIT_FACTORY_HH__
#define __UNIT_FACTORY_HH__

#include <QObject>
#include <QString>
#include <QStringList>

#include "unit-property-manager.hh"

#include "qtpropertybrowser.h"
#include "qteditorfactory.h"


class QSpinBox;
class QtSpinBoxFactory;



class CUnitFactory : public QtAbstractEditorFactory<CUnitPropertyManager>
{
  Q_OBJECT
  public:
  CUnitFactory(QObject *parent = 0);
  ~CUnitFactory();
  
protected:
  void connectPropertyManager(CUnitPropertyManager *manager);
  QWidget *createEditor(CUnitPropertyManager *manager, QtProperty *property,
			QWidget *parent);
  void disconnectPropertyManager(CUnitPropertyManager *manager);
private slots:
  
  void slotSuffixChanged(QtProperty *property, const QString &suffix);
  void slotEditorDestroyed(QObject *object);
private:
  QtSpinBoxFactory *originalFactory;
  QMap<QtProperty *, QList<QSpinBox *> > createdEditors;
  QMap<QSpinBox *, QtProperty *> editorToProperty;
};

#endif // __UNIT_FACTORY_HH__
