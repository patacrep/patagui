#ifndef __FILE_FACTORY_HH__
#define __FILE_FACTORY_HH__

#include <QObject>
#include <QString>
#include <QStringList>

#include "file-property-manager.hh"

#include "qtpropertybrowser.h"
#include "qteditorfactory.h"


class QLineEdit;
class QtLineEditFactory;



class CFileFactory : public QtAbstractEditorFactory<CFilePropertyManager>
{
  Q_OBJECT
  public:
  CFileFactory(QObject *parent = 0);
  ~CFileFactory();
  
protected:
  void connectPropertyManager(CFilePropertyManager *manager);
  QWidget *createEditor(CFilePropertyManager *manager, QtProperty *property,
			QWidget *parent);
    void disconnectPropertyManager(CFilePropertyManager *manager);
private slots:
  void browse();
  void slotFilenameChanged(QtProperty *property, const QString &filename);
  void slotEditorDestroyed(QObject *object);
private:
  QtLineEditFactory *originalFactory;
  CFilePropertyManager* m_manager;
  QtProperty* m_property;
  QMap<QtProperty *, QList<QLineEdit *> > createdEditors;
  QMap<QLineEdit *, QtProperty *> editorToProperty;
};

#endif // __FILE_FACTORY_HH__
