/**
 * \file songSortFilterProxyModel.hh
 *
 * Sort and filter proxy model for the songs library.
 *
 */
#ifndef __SONG_SORT_FILTER_PROXY_MODEL_HH__
#define __SONG_SORT_FILTER_PROXY_MODEL_HH__

#include <QSortFilterProxyModel>

class CSongSortFilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  CSongSortFilterProxyModel(QObject *parent = 0);
  ~CSongSortFilterProxyModel();

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
};

#endif // __SONG_SORT_FILTER_PROXY_MODEL_HH__
