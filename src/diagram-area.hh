// Copyright (C) 2009-2012, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2012, Alexandre Dupas <alexandre.dupas@gmail.com>
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
#ifndef __DIAGRAM_AREA_HH__
#define __DIAGRAM_AREA_HH__


#include <QWidget>
#include <QAbstractTableModel>
#include <QModelIndex>
#include <QString>
#include <QList>
#include <QPoint>
#include <QVector>
#include <QMap>

#include "chord.hh"


/*!
  \file diagram-area.hh
  \class CTableDiagram
  \brief CTableDiagram is a table model that contains CChord objects.

  A CTableDiagram presents data on a grid where the number of rows or
  columns is specified with setRowCount() or setColumnCount(). Adding
  a new data element (CChord) to the model does not require
  indicating its position on the grid as this is automatically
  computed.

  The following example defines a model that may have several columns
  but a single row:

  \code
  CTableDiagram *model = new CTableDiagram;
  model->setRowCount(1);
  model->addItem("\gtab{C}{X32010}");
  model->addItem("\gtab{Am}{X02210}");
  model->addItem("\gtab{G}{X02210}");
  model->addItem("\gtab{D}{XX0232}");
  \endcode

  Intuitively, the example above behaves like a QHBoxLayout whereas in
  this second example, the model presents data on 3 columns but any
  number of rows (thus, the D chord is displayed on the second row):

  \code
  CTableDiagram *model = new CTableDiagram;
  model->setRowCount(3);
  model->addItem("\gtab{C}{X32010}");
  model->addItem("\gtab{Am}{X02210}");
  model->addItem("\gtab{G}{X02210}");
  model->addItem("\gtab{D}{XX0232}");
  \endcode
*/
class CTableDiagram : public QAbstractTableModel
{
  Q_OBJECT

  public:
  /*!
    \enum DiagramRoles
    Each CChord object the model has a set of data elements associated with it, each with its own role.
    The roles are used by the view to indicate to the model which type of data it needs.
  */
  enum DiagramRoles {
    NameRole = Qt::UserRole + 1, /*!< the name of the chord.*/
    StringsRole = Qt::UserRole + 2, /*!< the strings sequence of the chord.*/
    TypeRole = Qt::UserRole + 3, /*!< the instrument of the chord.*/
    ImportantRole = Qt::UserRole + 4, /*!< whether the chord is important.*/
    MaxRole = ImportantRole
  };

  /// Constructor.
  CTableDiagram(QWidget *parent=0);

  /// Destructor.
  ~CTableDiagram();

  /*!
    Returns the number of columns.
    \sa setColumnCount, rowCount, setRowCount
  */
  virtual int columnCount(const QModelIndex & index = QModelIndex()) const;

  /*!
    Sets the number of columns to \a value.
    Calling this method defines a "fixed" column count mode which means
    that any item that is added afterwards will start a new row if necessary.
    \sa columnCount, rowCount, setRowCount
  */
  virtual void setColumnCount(int value);

  /*!
    Returns the number of rows.
    \sa setRowCount, columnCount, setColumnCount
  */
  virtual int rowCount(const QModelIndex & index = QModelIndex()) const;

    /*!
    Sets the number of rows to \a value.
    Calling this method defines a "fixed" row count mode which means
    that any item that is added afterwards will start a new column if necessary.
    \sa rowCount, columnCount, setColumnCount
  */
  virtual void setRowCount(int value);

  /* drag and drop */
  Qt::DropActions supportedDropActions() const;
  Qt::DropActions supportedDragActions() const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QStringList mimeTypes() const;
  QMimeData * mimeData(const QModelIndexList &indexes) const;
  bool dropMimeData(const QMimeData *data, Qt::DropAction action,
		    int row, int column, const QModelIndex &parent);

  /*!
    Reimplements QAbstractTableModel::data().
    Returns the item (CChord object) at position \a index according to the role \a role.
    \sa setData
   */
  QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

  /*!
    Reimplements QAbstractTableModel::setData().
    Sets the value \a value of the item (CChord object) at position \a index for the role \a role.
    \sa data
   */
  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);


  /*!
    Returns the CChord object at position \a index.
    \sa data, setData
   */
  CChord * getDiagram(const QModelIndex & index) const;

public slots:
  void insertItem(const QModelIndex & index, const QString & value);
  void removeItem(const QModelIndex & index);
  void addItem(const QString & value);

private:
  QModelIndex indexFromPosition(int position);
  int positionFromIndex(const QModelIndex & index) const;

private:
  bool m_fixedColumnCount;
  bool m_fixedRowCount;
  int m_columnCount;
  int m_rowCount;
  QVector<CChord*> m_data;
};


class QPushButton;
class QTableView;
class QSortFilterProxyModel;

/*!
  \file diagram-area.hh
  \class CDiagramArea
  \brief CDiagramArea is a widget displaying a list of chords

  A CDiagramArea embeds a QTableView based upon a CTableDiagram model.
  A CDiagramArea can be defined as read-only or editable through
  setReadOnly() and is usually placed as a widget inside a scrolling
  area:

  \code
  CDiagramArea *area = new CDiagramArea(this);
  area->setRowCount(1);
  area->setReadOnly(false);

  QScrollArea *scroll = new QScrollArea;
  scroll->setWidget(area);
  scroll->setBackgroundRole(QPalette::Dark);
  scroll->setWidgetResizable(true);
  \endcode

  In the example above, the area is editable which means that chords
  can be edited or removed (from the contextMenu()). The area also
  features a "add" button that appends new chords to the model.
  An implementation may be found in song-header-editor.cc.

  If the CDiagramArea is read-only, displayed chords can neither be
  edited nor removed. Thus, the area does not propose the "add" button
  to add a new chord to the list. The following example can be found
  in the dialog from diagram-editor.cc where it is used to display all
  the chords in songbook/tex/chords.tex as a list of common chords to
  pick from:

  \code
  CDiagramArea *area = new CDiagramArea(this);
  area->setReadOnly(true);
  area->setColumnCount(8);

  QScrollArea *scroll = new QScrollArea;
  scroll->setWidget(area);
  scroll->setBackgroundRole(QPalette::Base);
  scroll->setWidgetResizable(true);
  \endcode

  The list of chords in a CDiagramArea can be filtered by methods such
  as setTypeFilter(), setNameFilter() and setStringsFilter() that will
  only display rows that contain chords whose CChord::instrument(),
  CChord::name() and CChord::strings() match the filter. In the
  dialog from diagram-editor.cc, those slots are thus connected to the
  QLineEdit widgets:

  \code
  QLineEdit *nameLineEdit = new QLineEdit;
  QLineEdit *stringsLineEdit = new QLineEdit;
  connect(nameLineEdit, SIGNAL(textChanged(const QString &)),
          area, SLOT(setNameFilter(const QString &)));
  connect(stringsLineEdit, SIGNAL(textChanged(const QString &)),
          area, SLOT(setStringsFilter(const QString &)));
  \endcode
*/
class CDiagramArea : public QWidget
{
  Q_OBJECT

  public:
  /// Constructor.
  CDiagramArea(QWidget *parent=0);

  /*!
    Returns \a true if the diagram-area is in read-only mode; \a false otherwise.
    \sa setReadOnly
  */
  bool isReadOnly() const;

  /*!
    Set the diagram-area as editable (\a false) or read-only (\a true) according to \a value.
    \sa isReadOnly
  */
  void setReadOnly(bool value);

  /*!
    Forces the list of chords to be displayed on \a value columns.
    \sa setRowCount
  */
  void setColumnCount(int value);

  /*!
    Forces the list of chords to be displayed on \a value rows.
    \sa setColumnCount
  */
  void setRowCount(int value);

  /*!
    Returns all the chords. Note that it returns chords from the model, not the view;
    thus, filtered chords are also included.
  */
  QList< CChord* > diagrams();

public slots:
  /*!
    Adds a new chord to the list. This slot is connected to the "add" button
    and pops-up a CDiagramEditor.
  */
  void newDiagram();

  /*!
    Appends the chord \a chord to the list.
    The user is responsible for the correctness of the chord.
  */
  void addDiagram(const QString & chord);

  /*!
    Triggers a CDiagramEditor associated to the chord at position \a index.
    This slot is only available in editable mode.
    \sa setReadOnly
  */
  void editDiagram(QModelIndex index = QModelIndex());

  /*!
    Removes the chord at position \a index from the list.
    This slot is only available in editable mode.
    \sa setReadOnly
  */
  void removeDiagram(QModelIndex index = QModelIndex());

  /*!
    Filters rows that contain chords whose instrument matches \a type.
    \sa setNameFilter, setStringsFilter
  */
  void setTypeFilter(const CChord::Instrument & type);

  /*!
    Filters rows that contain chords whose name matches \a name.
    \sa setTypeFilter, setStringsFilter
  */
  void setNameFilter(const QString & name);

  /*!
    Filters rows that contain chords whose strings matches \a strings.
    \sa setTypeFilter, setNameFilter
  */
  void setStringsFilter(const QString & strings);

  /*!
    Removes all filters.
    \sa setTypeFilter, setNameFilter, setStringsFilter
  */
  void clearFilters();

private slots:
  void update();
  void resizeRows();
  void onDiagramChanged();
  void contextMenu(const QPoint & pos);
  void onViewClicked(const QModelIndex &);

signals:
  /*!
    This signal is emitted when the content of the list of chords changes
    such as when editing, removing or adding a new chord.
    This signal is never emitted in read-only mode.
    \sa editDiagram, removeDiagram, newDiagram
  */
  void contentsChanged();

  /*!
    This signal is emitted when the contents of the model changes
    such as when removing, adding or filtering diagrams.
    \sa removeDiagram, newDiagram
  */
  void layoutChanged();

  /*!
    This signal is emitted when the read-only property changes.
    \sa isReadOnly, setReadOnly
  */
  void readOnlyModeChanged();

  /*!
    This signal is emitted when a chord from the list is clicked.
  */
  void diagramClicked(CChord * diagram);

private:
  bool m_isReadOnly;
  CTableDiagram *m_diagramModel;
  QSortFilterProxyModel *m_proxyModel;
  QTableView *m_diagramView;
  QPushButton *m_addDiagramButton;
  int m_nbDiagrams;
};


#endif //__DIAGRAM_AREA_HH__
