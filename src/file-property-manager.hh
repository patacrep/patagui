#ifndef __FILE_PROPERTY_MANAGER_HH__
#define __FILE_PROPERTY_MANAGER_HH__

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtIntPropertyManager>
#include <QtVariantProperty>
#include <QtGroupBoxPropertyBrowser>

class CFilePropertyManager : public QtStringPropertyManager
{
  Q_OBJECT
  public:
  CFilePropertyManager(QObject *parent = 0);
  ~CFilePropertyManager();

  QString filename(const QtProperty *property) const;
  static int id(){return 126;};
  virtual QString value(const QtProperty *property) const;

public slots:
  void setFilename(QtProperty *property, const QString &filter);

signals:
  void filenameChanged(QtProperty *property, const QString &filter);

protected:
  virtual void initializeProperty(QtProperty *property);
  virtual void uninitializeProperty(QtProperty *property);

private:
  struct Data {
    QString filename;
  };
  QMap<const QtProperty *, Data> propertyToData;
};

#endif // __FILE_PROPERTY_MANAGER_HH__
