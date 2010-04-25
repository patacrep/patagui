/** songbook.hh
 */

#include <QObject>
#include <QString>
#include <QStringList>

class CSongbook : public QObject
{
public:
  CSongbook();
  ~CSongbook();

  void setSongs( QStringList & songs );
  QStringList getSongs();

  void save( QString & filename);
  void load( QString & filename);

private:
  QStringList m_songs;
};
