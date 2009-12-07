// $Id: grid.h,v 1.2 2004/01/31 01:50:31 arandell Exp $
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

#ifndef GRID_H
#define GRID_H

#include "text_frame.h"
#include <qvaluevector.h>

class ListView;
class QPrinter;

// Special row/column values for set
enum { USE_CURR=-1, USE_NEXT=-2 };

class Grid: public Frame {
public:
    Grid(int columns, Frame* parent=0);
    ~Grid();

    // These sides are used both for sticky and for borders in the cell
    enum Side { Center=0, Top=1, Bottom=2, Left=4, Right=8,
		TopLeft=5, TopRight=9, BottomLeft=6, BottomRight=10,
		LeftRight=12, TopBottom=3, AllSides=15 };

    // Size of grid
    int rows() const;
    int columns() const;

    // Defaults for table
    fixed defaultRowPad() const;
    void setDefaultRowPad(fixed pad);
    int defaultSticky() const;
    void setDefaultSticky(int sticky);

    // Number of columns for header and footer
    int headerRows() const;
    int footerRows() const;
    void setHeaderRows(int rows);
    void setFooterRows(int rows);

    // Get row information
    fixed rowMinHeight(int row);
    fixed rowMaxHeight(int row);
    fixed rowPad(int row);
    fixed rowWeight(int row);

    // Set row information
    void setRowMinHeight(int row, fixed height);
    void setRowMaxHeight(int row, fixed height);
    void setRowHeight(int row, fixed height);
    void setRowPad(int row, fixed pad);
    void setRowWeight(int row, fixed weight);

    // Get column information
    fixed columnMinWidth(int column);
    fixed columnMaxWidth(int column);
    fixed columnPad(int column);
    fixed columnWeight(int column);
    int columnSticky(int column);

    // Set column information
    void setColumnMinWidth(int column, fixed width);
    void setColumnMaxWidth(int column, fixed width);
    void setColumnWidth(int column, fixed width);
    void setColumnPad(int column, fixed pad);
    void setColumnWeight(int column, fixed weight);
    void setColumnSticky(int column, int sticky);

    // Set cell information
    void setRowSpan(int row, int column, int span);
    void setColumnSpan(int row, int column, int span);
    void setBorder(int row, int column, int border);
    void setSticky(int row, int column, int sticky);
    void set(int row, int column, Frame* frame, int sticky=-1);
    void set(int row, int column, int rowSpan, int columnSpan,
	     Frame* frame, int sticky=-1);

    // Ease of use methods to create text cells
    void set(int row, int column, const QString& text, int sticky=-1);
    void set(int row, int column, int rowSpan, int columnSpan,
	     const QString& text, int sticky=-1);

    // Get cell information
    int rowSpan(int row, int column);
    int columnSpan(int row, int column);
    int border(int row, int column);
    int sticky(int row, int column);
    Frame* get(int row, int column);

    // Find row or column given offset within grid
    int rowAt(fixed y);
    int columnAt(fixed x);

    bool print(QWidget* parent, QPrinter* printer=0, bool useMargins=true);
    void paintRows(QPainter* p, int row1, int row2);
    static bool print(QValueVector<Grid*>& grids, QWidget* parent,
		      bool useMargins=true);

    // Top level printing methods
    bool showPageNumber() const;
    bool showCreateDate() const;
    void setShowPageNumber(bool show);
    void setShowCreateDate(bool show);

    // Print information used by external frames
    static int pageNumber();
    static bool lastPage();
    static QPainter* painter();

    // Redefined from Frame
    void setMeasurement(int type);
    fixed width();
    fixed height();
    void resize(fixed width, fixed height);
    void preferredSize(fixed& width, fixed& height);
    void paint(QPainter* p, int x=0, int y=0, int w=-1, int h=-1);
    void paintChildren(QPainter* p);
    void print();

    // Build grid from ListView
    static Grid* buildGrid(ListView* list, const QString& title);

protected:
    // Moved here so can't directly add/remove cells
    void addChild(Frame* child);
    void removeChild(Frame* child);

    void preferredSizes(QValueVector<fixed>& rowSize,
			QValueVector<fixed>& columnSize);
    void cellPreferredSize(int row, int column, fixed& width, fixed& height);
    void recalculate();
    void layout();
    void checkRow(int& row);
    void checkColumn(int& column);
    void printOne(QPainter& p, QPrinter* printer, bool useMargins);

    // Information on a row or column in the grid
    struct Info {
	Info() { minSize=pad=weight=size=0.0; maxSize=99999.0; sticky=-1; }
	Info(fixed _pad) { pad=_pad; minSize=weight=size=0.0; sticky=0;
				maxSize=99999.0; }

	fixed minSize;
	fixed maxSize;
	fixed pad;
	fixed weight;
	fixed size;
	int sticky;
    };

    // Information on a cell in the grid
    struct Cell {
	Cell() { row=col=border=sticky=0; rowSpan=columnSpan=1; frame=NULL; }

	int row, col;
	int rowSpan, columnSpan;
	int border;
	int sticky;
	Frame* frame;
    };

    int _columns;
    QValueVector<Cell*> _rows;
    int _lastRow, _lastColumn;
    QValueVector<Info> _rowInfo, _columnInfo;
    int _headerRows, _footerRows;
    fixed _prefWidth, _prefHeight;
    bool _needLayout;
    fixed _defaultRowPad;
    int _defaultSticky;
    bool _showPageNumber;
    bool _showCreateDate;
    static int _page;
    static bool _lastPage;
    static QPainter* _painter;
};

#endif
