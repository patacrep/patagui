#ifndef __UNIT_PROPERTY_MANAGER_HH__
#define __UNIT_PROPERTY_MANAGER_HH__

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtIntPropertyManager>
#include <QtVariantProperty>
#include <QtGroupBoxPropertyBrowser>


class CUnitPropertyManager : public QtIntPropertyManager
{
  Q_OBJECT
  public:
  CUnitPropertyManager(QObject *parent = 0);
  ~CUnitPropertyManager();

  QString suffix(const QtProperty *property) const;
  QString valueText(const QtProperty *property) const;  
  static int id(){return 127;};

public slots:
  void setSuffix(QtProperty *property, const QString &suffix);

signals:
  void suffixChanged(QtProperty *property, const QString &suffix);

protected:
  virtual void initializeProperty(QtProperty *property);
  virtual void uninitializeProperty(QtProperty *property);

private:
  struct Data {
    QString suffix;
  };
  QMap<const QtProperty *, Data> propertyToData;
};

#endif // __UNIT_PROPERTY_MANAGER_HH__
