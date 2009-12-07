// $Id: table.h,v 1.28 2004/02/03 00:56:03 arandell Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Accounting
//
// This file may be distributed and/or modified under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL included in the
// packaging of this file.
//
// Licensees holding a valid Quasar Commercial License may use this file
// in accordance with the Quasar Commercial License Agreement provided
// with the Software in the LICENSE.COMMERCIAL file.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// See http://www.linuxcanada.com or email sales@linuxcanada.com for
// information about Quasar Accounting support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#ifndef TABLE_H
#define TABLE_H

#include "table_column.h"
#include "table_row.h"
#include "cell_editor.h"
#include <qscrollview.h>
#include <qvaluevector.h>

class QHeader;

class Table: public QScrollView {
    Q_OBJECT
public:
    Table(QWidget* parent=0, const char* name=0);
    ~Table();

    // Look and feel
    bool showGrid() const;
    bool localTab() const;
    int displayRows() const;

    // Column methods
    void addColumn(TableColumn* column);
    int columns() const;
    TableColumn* column(int col);
    void setColumnName(int col, const QString& name);

    // Row methods
    void appendRow(TableRow* row);
    void deleteRow(int row);
    void insertRow(int before, TableRow* row);
    void clear();
    int rows() const;
    TableRow* row(int row);

    // Data columns (used for appendRow() and insertRow())
    int dataColumns() const;
    void setDataColumns(int columns);

    // Movement
    void setCurrentCell(int row, int col);
    int currentRow() const;
    int currentColumn() const;
    void ensureCellVisible(int row, int col);

    // Proxy for TableRow
    Variant cellValue(int row, int col, bool current=true);
    void setCellValue(int row, int col, Variant value);

    // Sizing
    QSize sizeHint() const;
    void setRowHeight(int row, int height);
    void setColumnWidth(int col, int width);
    QSize tableSize() const;
    void adjustColumn(int col);
    void adjustRow(int row);

    // Cell geometry
    QRect cellGeometry(int row, int col) const;
    int columnWidth(int col) const;
    int rowHeight(int row) const;
    int columnPos(int col) const;
    int rowPos(int row) const;
    int columnAt(int pos) const;
    int rowAt(int pos) const;

    // Margins
    void setLeftMargin(int margin);
    void setTopMargin(int margin);

    int columnMinWidth(int col) const;
    int columnMaxWidth(int col) const;

    void setUpdatesEnabled(bool flag);
    void repaintCell(int row, int col);
    void repaintRow(int row);
    void stopUserInsertDelete();

    enum MoveType { MoveUp, MoveDown, MoveNext, MovePrev };
    bool eventFilter(QObject* o, QEvent* e);

signals:
    void cellStartEdit(int row, int col);
    void cellMoved(int row, int col);
    void cellClicked(int row, int col);
    void cellValidate(int row, int col, Variant old, bool& ok);
    void cellChanged(int row, int col, Variant old);
    void rowDeleted(int row);
    void rowInserted(int row);
    void focusNext(bool& leave, int& newRow, int& newCol, int type);

public slots:
    void setShowGrid(bool show);
    void setLocalTab(bool tabLocally);
    void setDisplayRows(int rows);

protected slots:
    void rowHeightChanged(int row);
    void columnWidthChanged(int col, int oldWidth, int newWidth);
    void appendRow();
    void insertRow();
    void deleteRow();

protected:
    void drawContents(QPainter *p, int cx, int cy, int cw, int ch);
    void contentsMousePressEvent(QMouseEvent* e);
    void contentsMouseReleaseEvent(QMouseEvent* e);
    void resizeEvent(QResizeEvent* e);
    void showEvent(QShowEvent* e);
    void focusInEvent(QFocusEvent* e);
    void focusOutEvent(QFocusEvent* e);

    void editorResize();
    void updateGeometries();
    void paintCell(QPainter* p, int row, int col, const QRect& cr);
    void paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch);
    void updateCell(int row, int col);
    void startEdit();
    bool endEdit(bool grabFocus=true);
    void columnExpandShrink(int ignoreCol);
    void nextCell(bool& leave, int& newRow, int& newCol, int type);

    QValueVector<TableColumn*> _columns;
    QValueVector<TableRow*> _rows;
    QHeader* _leftHeader;
    QHeader* _topHeader;
    int _row;
    int _col;
    bool _showGrid;
    bool _localTab;
    int _defaultRowHeight;
    int _displayRows;
    CellEditor* _editor;
    bool _ignoreFocus;
    int _popupRow;
    int _dataColumns;
    bool _userInsertDelete;
};

#endif // TABLE_H
