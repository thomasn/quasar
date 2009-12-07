// $Id: grid.cpp,v 1.4 2005/02/17 09:55:03 bpepers Exp $
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

#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"
#include "list_view_item.h"
#include "user_config.h"

#include <qprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qmessagebox.h>

#include <assert.h>

#include <qapplication.h>
#include <stdlib.h>
#define tr(text) qApp->translate("Grid",text)

int Grid::_page=1;
bool Grid::_lastPage = true;
QPainter* Grid::_painter = NULL;

Grid::Grid(int columns, Frame* parent)
    : Frame(parent), _columns(columns), _lastRow(-1), _lastColumn(-1),
      _headerRows(0), _footerRows(0), _needLayout(false),
      _defaultRowPad(0.0), _defaultSticky(0), _showPageNumber(true),
      _showCreateDate(true)
{
    _columnInfo.resize(_columns);
}

Grid::~Grid()
{
    for (unsigned int i = 0; i < _rows.size(); ++i)
	delete _rows[i];
}

int
Grid::rows() const
{
    return _rows.size();
}

int
Grid::columns() const
{
    return _columns;
}

fixed
Grid::defaultRowPad() const
{
    return _defaultRowPad;
}

void
Grid::setDefaultRowPad(fixed pad)
{
    _defaultRowPad = pad;
}

int
Grid::defaultSticky() const
{
    return _defaultSticky;
}

void
Grid::setDefaultSticky(int sticky)
{
    _defaultSticky = sticky;
}

int
Grid::headerRows() const
{
    return _headerRows;
}

int
Grid::footerRows() const
{
    return _footerRows;
}

void
Grid::setHeaderRows(int rows)
{
    _headerRows = rows;
}

void
Grid::setFooterRows(int rows)
{
    _footerRows = rows;
}

fixed
Grid::rowMinHeight(int row)
{
    checkRow(row);
    return _rowInfo[row].minSize;
}

fixed
Grid::rowMaxHeight(int row)
{
    checkRow(row);
    return _rowInfo[row].maxSize;
}

fixed
Grid::rowPad(int row)
{
    checkRow(row);
    return _rowInfo[row].pad;
}

fixed
Grid::rowWeight(int row)
{
    checkRow(row);
    return _rowInfo[row].weight;
}

void
Grid::setRowMinHeight(int row, fixed height)
{
    checkRow(row);
    _rowInfo[row].minSize = height;
    invalidate();
}

void
Grid::setRowMaxHeight(int row, fixed height)
{
    checkRow(row);
    _rowInfo[row].maxSize = height;
    invalidate();
}

void
Grid::setRowHeight(int row, fixed height)
{
    checkRow(row);
    _rowInfo[row].minSize = height;
    _rowInfo[row].maxSize = height;
    invalidate();
}

void
Grid::setRowPad(int row, fixed pad)
{
    checkRow(row);
    _rowInfo[row].pad = pad;
    invalidate();
}

void
Grid::setRowWeight(int row, fixed weight)
{
    checkRow(row);
    _rowInfo[row].weight = weight;
    invalidate();
}

fixed
Grid::columnMinWidth(int column)
{
    checkColumn(column);
    return _columnInfo[column].minSize;
}

fixed
Grid::columnMaxWidth(int column)
{
    checkColumn(column);
    return _columnInfo[column].maxSize;
}

fixed
Grid::columnPad(int column)
{
    checkColumn(column);
    return _columnInfo[column].pad;
}

fixed
Grid::columnWeight(int column)
{
    checkColumn(column);
    return _columnInfo[column].weight;
}

int
Grid::columnSticky(int column)
{
    checkColumn(column);
    return _columnInfo[column].sticky;
}

void
Grid::setColumnMinWidth(int column, fixed width)
{
    checkColumn(column);
    _columnInfo[column].minSize = width;
    invalidate();
}

void
Grid::setColumnMaxWidth(int column, fixed width)
{
    checkColumn(column);
    _columnInfo[column].maxSize = width;
    invalidate();
}

void
Grid::setColumnWidth(int column, fixed width)
{
    checkColumn(column);
    _columnInfo[column].minSize = width;
    _columnInfo[column].maxSize = width;
    invalidate();
}

void
Grid::setColumnPad(int column, fixed pad)
{
    checkColumn(column);
    _columnInfo[column].pad = pad;
    invalidate();
}

void
Grid::setColumnWeight(int column, fixed weight)
{
    checkColumn(column);
    _columnInfo[column].weight = weight;
    invalidate();
}

void
Grid::setColumnSticky(int column, int sticky)
{
    checkColumn(column);
    _columnInfo[column].sticky = sticky;
}

void
Grid::setRowSpan(int row, int column, int span)
{
    checkRow(row);
    checkColumn(column);

    Cell& cell = _rows[row][column];
    cell.rowSpan = span;
}

void
Grid::setColumnSpan(int row, int column, int span)
{
    checkRow(row);
    checkColumn(column);
    assert(column + span <= _columns);

    Cell& cell = _rows[row][column];
    cell.columnSpan = span;
}

void
Grid::setBorder(int row, int column, int border)
{
    checkRow(row);
    checkColumn(column);

    Cell& cell = _rows[row][column];
    cell.border = border;
}

void
Grid::setSticky(int row, int column, int sticky)
{
    checkRow(row);
    checkColumn(column);

    Cell& cell = _rows[row][column];
    cell.sticky = sticky;
}

void
Grid::set(int row, int column, Frame* frame, int sticky)
{
    set(row, column, 1, 1, frame, sticky);
}

void
Grid::set(int row, int column, int rs, int cs, Frame* frame, int sticky)
{
    checkRow(row);
    checkColumn(column);
    assert(column + cs <= _columns);
    assert(rs > 0);
    assert(cs > 0);
    int lastRow = row + rs - 1;
    checkRow(lastRow);

    if (frame) frame->setParent(this);

    if (sticky == -1)
	sticky = _columnInfo[column].sticky;
    if (sticky == -1)
	sticky = _defaultSticky;

    Cell& cell = _rows[row][column];
    cell.row = row;
    cell.col = column;
    cell.rowSpan = rs;
    cell.columnSpan = cs;
    cell.sticky = sticky;
    delete cell.frame;
    cell.frame = frame;

    invalidate();
    _lastRow = row;
    _lastColumn = column;
}

void
Grid::set(int row, int column, const QString& text, int sticky)
{
    TextFrame* frame = new TextFrame(text, this);
    set(row, column, frame, sticky);
}

void
Grid::set(int row, int column, int rs, int cs, const QString& text, int sticky)
{
    TextFrame* frame = new TextFrame(text, this);
    set(row, column, rs, cs, frame, sticky);
}

int
Grid::rowSpan(int row, int column)
{
    checkRow(row);
    checkColumn(column);
    return _rows[row][column].rowSpan;
}

int
Grid::columnSpan(int row, int column)
{
    checkRow(row);
    checkColumn(column);
    return _rows[row][column].columnSpan;
}

int
Grid::border(int row, int column)
{
    checkRow(row);
    checkColumn(column);
    return _rows[row][column].border;
}

int
Grid::sticky(int row, int column)
{
    checkRow(row);
    checkColumn(column);
    return _rows[row][column].sticky;
}

Frame*
Grid::get(int row, int column)
{
    checkRow(row);
    checkColumn(column);
    return _rows[row][column].frame;
}

int
Grid::rowAt(fixed y)
{
    recalculate();
    layout();

    fixed yPos = 0;
    for (int row = 0; row < rows(); ++row) {
	yPos += _rowInfo[row].size;
	if (y < yPos) return row;
    }
    return -1;
}

int
Grid::columnAt(fixed x)
{
    recalculate();
    layout();

    fixed xPos = 0;
    for (int col = 0; col < _columns; ++col) {
	xPos += _columnInfo[col].size;
	if (x < xPos) return col;
    }
    return -1;
}

fixed
Grid::width()
{
    recalculate();
    return _width;
}

fixed
Grid::height()
{
    recalculate();
    return _height;
}

void
Grid::resize(fixed width, fixed height)
{
    recalculate();

    fixed oldWidth = _width;
    fixed oldHeight = _height;

    _width = width;
    _height = height;

    fixed widthDiff = width - oldWidth;
    if (widthDiff != 0.0) {
	QValueVector<int> index;
	fixed totalWeight = 0.0;
	for (int col = 0; col < _columns; ++col) {
	    if (_columnInfo[col].weight == 0) continue;
	    totalWeight += _columnInfo[col].weight;
	    index.push_back(col);
	}

	if (totalWeight == 0.0) {
	    _width = oldWidth;
	} else {
	    fixed alloc = 0;
	    for (int col = 0; col < _columns; ++col) {
		fixed percent = _columnInfo[col].weight / totalWeight;
		fixed change = percent * widthDiff;
		_columnInfo[col].size += change;
		alloc += change;
	    }
	    if (alloc != widthDiff) {
		int col = index[rand() % index.size()];
		_columnInfo[col].size += widthDiff - alloc;
	    }
	}
    }

    fixed heightDiff = height - oldHeight;
    if (heightDiff != 0) {
	QValueVector<int> index;
	fixed totalWeight = 0;
	for (int row = 0; row < rows(); ++row) {
	    if (_rowInfo[row].weight == 0) continue;
	    totalWeight += _rowInfo[row].weight;
	    index.push_back(row);
	}

	if (totalWeight == 0.0) {
	    _height = oldHeight;
	} else {
	    fixed alloc = 0.0;
	    for (int row = 0; row < rows(); ++row) {
		fixed percent = _rowInfo[row].weight / totalWeight;
		fixed change = percent * heightDiff;
		_rowInfo[row].size += change;
		alloc += change;
	    }
	    if (alloc != heightDiff) {
		int row = index[rand() % index.size()];
		_rowInfo[row].size += heightDiff - alloc;
	    }
	}
    }

    _needLayout = true;
}

void
Grid::preferredSize(fixed& width, fixed& height)
{
    recalculate();
    width = _prefWidth;
    height = _prefHeight;
}

void
Grid::paint(QPainter* p, int, int y, int w, int h)
{
    recalculate();
    layout();

    if (w == -1) w = _width.toInt();
    if (h == -1) h = _height.toInt();

    // Pick rows to paint
    int row1 = -1;
    int row2 = -1;
    fixed yPos = 0.0;
    for (int row = 0; row < rows(); ++row) {
	fixed maxSize = 0.0;
	for (int col = 0; col < _columns; ++col) {
	    Cell& cell = _rows[row][col];
	    fixed size = 0.0;
	    for (int i = 0; i < cell.rowSpan; ++i)
		size += _rowInfo[row + i].size;
	    if (size > maxSize) maxSize = size;
	}
	if (y < yPos + maxSize && row1 == -1) row1 = row;
	if (y + h < yPos + _rowInfo[row].size) row2 = row;
	if (row1 != -1 && row2 != -1) break;
	yPos += _rowInfo[row].size;
    }

    if (row1 == -1) return;
    if (row2 == -1) row2 = rows() - 1;
    paintRows(p, row1, row2);
}

void
Grid::paintRows(QPainter* p, int row1, int row2)
{
    fixed y = 0.0;
    int row;
    for (row = 0; row < row1; ++row)
	y += _rowInfo[row].size;

    for (row = row1; row <= row2; ++row) {
	fixed x = 0.0;
	for (int col = 0; col < _columns; ++col) {
	    Cell& cell = _rows[row][col];

	    fixed cellWidth = 0.0;
	    int i;
	    for (i = 0; i < cell.columnSpan; ++i)
		cellWidth += _columnInfo[col + i].size;

	    fixed cellHeight = 0.0;
	    for (i = 0; i < cell.rowSpan; ++i)
		cellHeight += _rowInfo[row + i].size;

	    // Draw borders
	    if (cell.border != 0) {
		int x1 = x.toInt();
		int y1 = y.toInt();
		int x2 = (x + cellWidth).toInt();
		int y2 = (y + cellHeight).toInt();

		if (cell.border & Left)
		    p->drawLine(x1, y1, x1, y2);
		if (cell.border & Right)
		    p->drawLine(x2, y1, x2, y2);
		if (cell.border & Top)
		    p->drawLine(x1, y1, x2, y1);
		if (cell.border & Bottom)
		    p->drawLine(x1, y2, x2, y2);
	    }

	    if (cell.frame != NULL) {
		p->setClipRect(p->xForm(QRect(x.toInt(), y.toInt(),
					      cellWidth.toInt(),
					      cellHeight.toInt())));
		p->translate(cell.frame->x().toInt(),
			     cell.frame->y().toInt());
		cell.frame->paint(p);
		p->setClipping(false);
		p->translate(-cell.frame->x().toInt(),
			     -cell.frame->y().toInt());
	    }
	    x += _columnInfo[col].size;
	}
	y += _rowInfo[row].size;
    }
}

void
Grid::paintChildren(QPainter*)
{
    // don't use this to paint children
}

bool
Grid::print(QValueVector<Grid*>& grids, QWidget* parent, bool useMargins)
{
    // Setup painter
    QPainter p;

    UserConfig config;
    config.load();

    // Ask for printer if needed
    QPrinter localPrinter(QPrinter::HighResolution);
    QPrinter* printer = &localPrinter;

    // TODO: get proper default?
    printer->setColorMode(QPrinter::Color);
    printer->setFullPage(true);

    if (config.preferA4)
	printer->setPageSize(QPrinter::A4);
    else
	printer->setPageSize(QPrinter::Letter);

    if (!printer->setup(parent))
	return false;

    if (printer->pageSize() == QPrinter::A4) {
	config.preferA4 = true;
	config.save(true);
    }

    // Printout pages
    p.begin(printer);
    for (unsigned int i = 0; i < grids.size(); ++i) {
	grids[i]->printOne(p, printer, useMargins);
	if (i != grids.size() - 1) printer->newPage();
    }
    p.end();

    return true;
}

bool
Grid::print(QWidget* parent, QPrinter* printer, bool useMargins)
{
    // Setup painter
    QPainter p;
    _painter = &p;

    // Ask for printer if needed
    QPrinter localPrinter(QPrinter::HighResolution);
    if (printer == NULL) {
	printer = &localPrinter;

	UserConfig config;
	config.load();

	// TODO: get proper default?
	printer->setColorMode(QPrinter::Color);
	printer->setFullPage(true);

	if (config.preferA4)
	    printer->setPageSize(QPrinter::A4);
	else
	    printer->setPageSize(QPrinter::Letter);

start:
	if (!printer->setup(parent)) {
	    _painter = NULL;
	    return false;
	}

	if (printer->pageSize() == QPrinter::A4) {
	    config.preferA4 = true;
	    config.save(true);
	}

	// Calculates sizes and positions on paper (extra 2 for borders)
	QPaintDeviceMetrics metrics(printer);
	fixed pageWidth = metrics.width() - 2 * printer->margins().width() - 2;
	fixed pageHeight = metrics.height() - 2 * printer->margins().height();

	// Setup painter so that font sizes are correct
	p.begin(printer);
	invalidate();

	// Check that preferred size isn't too large
	fixed prefWidth, prefHeight;
	preferredSize(prefWidth, prefHeight);
	if (prefWidth > pageWidth) {
	    fixed inches = (prefWidth + 2 * printer->margins().width()) /
		metrics.logicalDpiX();
	    fixed mm = inches * 25.4;
	    QString inchesS = inches.toString();
	    QString mmS = mm.toString();

	    QString message = tr("The report cannot fit on the paper size "
		"selected.  It will need\npaper at least %1 inches (%2 mm) "
		"wide.  Do you\nwish to choose a different paper size or "
		"to cancel printing?").arg(inchesS).arg(mmS);
	    int choice = -1;
	    if (prefHeight <= pageHeight)
		choice = QMessageBox::warning(NULL, tr("Paper Size Error"),
					      message, tr("Retry"),
					      tr("Ignore"), tr("Cancel"));
	    else
		choice = QMessageBox::warning(NULL, tr("Paper Size Error"),
					      message, tr("Retry"),
					      tr("Ignore"), tr("Cancel"));

	    if (choice == 0) goto start;
	    if (choice == 2) {
		printer->abort();
		_painter = NULL;
		return false;
	    }
	    if (choice == 3)
		printer->setOrientation(QPrinter::Landscape);
	}
    } else {
	// Setup painter so that font sizes are correct
	p.begin(printer);
	invalidate();
    }

    printOne(p, printer, useMargins);
    p.end();

    return true;
}

void
Grid::printOne(QPainter& p, QPrinter* printer, bool useMargins)
{
    _painter = &p;
    invalidate();
    _lastPage = true;
    _page = 1;

    // Get margins
    fixed topMargin = printer->margins().height();
    fixed bottomMargin = printer->margins().height();
    fixed leftMargin = printer->margins().width();
    fixed rightMargin = printer->margins().width();

    // Adjust margins as per user settings
    fixed topAdjust = 0.0;
    fixed bottomAdjust = 0.0;
    fixed leftAdjust = 0.0;
    fixed rightAdjust = 0.0;
    topMargin += topAdjust;
    bottomMargin += bottomAdjust;
    leftMargin += leftAdjust;
    rightMargin += rightAdjust;

    // Check against minimums
    QPaintDeviceMetrics metrics(printer);
    fixed minWidth = int(20.0 * metrics.logicalDpiX() / 72.0);
    fixed minHeight = int(40.0 * metrics.logicalDpiY() / 72.0);
    if (topMargin < minHeight) topMargin = minHeight;
    if (bottomMargin < minHeight) bottomMargin = minHeight;
    if (leftMargin < minWidth) leftMargin = minWidth;
    if (rightMargin < minWidth) rightMargin = minWidth;

    // Add room for page number and create date
    if (_showCreateDate || _showPageNumber)
        bottomMargin += minWidth;

    // Reset margins to zero if not being used
    if (!useMargins) {
	topMargin = 0.0;
	bottomMargin = 0.0;
	leftMargin = 0.0;
	rightMargin = 0.0;
    }

    // Calculates sizes and positions on paper (extra 2 on width for borders)
    fixed pageWidth = metrics.width() - leftMargin - rightMargin - 2;
    fixed pageHeight = metrics.height() - topMargin - bottomMargin;
    resize(preferredWidth(), preferredHeight());

    // Get header height
    fixed headerHeight = 0;
    int i;
    for (i = 0; i < _headerRows; ++i)
	headerHeight += _rowInfo[i].size;

    // Get footer height
    fixed footerHeight = 0;
    for (i = 0; i < _footerRows; ++i)
	footerHeight += _rowInfo[rows() - i - 1].size;

#if 0
    // How many pages to fit on?
    fixed bodyHeight = pageHeight - headerHeight - footerHeight;
    fixed totalBodyHeight = preferredHeight() - headerHeight - footerHeight;
    fixed perPage = pageHeight - headerHeight - footerHeight;
    fixed pages = (totalBodyHeight + perPage - 1) / perPage;
    fixed best = pages.toInt() * bodyHeight + headerHeight + footerHeight;

    // Try and fit to pages
    resize(preferredWidth(), preferredHeight());
    resize(pageWidth, best);
    recalculate();
    layout();
#else
    // Try and fit to one page
    fixed bodyHeight = pageHeight - headerHeight - footerHeight;
    resize(pageWidth, pageHeight > _height ? pageHeight : _height);
    recalculate();
    layout();
#endif

    // Output pages
    fixed xStart = leftMargin + (pageWidth - _width) / 2;
    fixed yBody = topMargin;
    fixed yFooter = pageHeight + topMargin - _height;
    int row1 = _headerRows;
    while (row1 < rows() - _footerRows) {
	// Find last row that fits on table
	int row2;
	fixed rowsHeight = 0;
	for (row2 = row1; row2 < rows() - _footerRows; ++row2) {
	    fixed rowHeight = 0;
	    for (int col = 0; col < _columns; ++col) {
		Cell& cell = _rows[row2][col];
		fixed height = 0;
		for (int i = 0; i < cell.rowSpan; ++i)
		    height += _rowInfo[row2 + i].size;
		if (height > rowHeight) rowHeight = height;
	    }

	    // Check if row makes things *larger* than the body height.
	    // The check is not >= since we want row2 to be on the row
	    // that puts it over the top.
	    if (int(rowsHeight.toDouble() + rowHeight.toDouble()) > int(bodyHeight.toDouble() + .5))
		break;
	    rowsHeight += rowHeight;
	}
	--row2;
	assert(row2 >= row1);
	_lastPage = (row2 >= rows() - _footerRows - 1);

	// Output header rows
	if (_headerRows > 0) {
	    p.translate(xStart.toInt(), topMargin.toInt());
	    paintRows(&p, 0, _headerRows - 1);
	    p.translate(-xStart.toInt(), -topMargin.toInt());
	}

	// Output body rows
	p.translate(xStart.toInt(), yBody.toInt());
	paintRows(&p, row1, row2);
	p.translate(-xStart.toInt(), -yBody.toInt());

	row1 = row2 + 1;
	yBody -= rowsHeight;

	// Output footer rows
	if (_footerRows > 0) {
	    p.translate(xStart.toInt(), yFooter.toInt());
	    paintRows(&p, rows() - _footerRows, rows() - 1);
	    p.translate(-xStart.toInt(), -yFooter.toInt());
	}

	// Add page number and date
	if (_showCreateDate && useMargins) {
	    p.save();
	    p.setFont(QFont("Helvetica", 8));
	    p.drawText(leftMargin.toInt(),
		       (topMargin + pageHeight).toInt(),
		       pageWidth.toInt(), minWidth.toInt(),
		       Qt::AlignLeft | Qt::AlignVCenter,
		       tr("Created: ") + QDate::currentDate().toString());
	    p.restore();
	}
	if (_showPageNumber && useMargins) {
	    p.save();
	    p.setFont(QFont("Helvetica", 8));
	    p.drawText(leftMargin.toInt(),
		       (topMargin + pageHeight).toInt(),
		       pageWidth.toInt(), minWidth.toInt(),
		       Qt::AlignRight | Qt::AlignVCenter,
		       tr("Page: ") + QString::number(_page));
	    p.restore();
	}

	++_page;
	if (!_lastPage) printer->newPage();
    }

    _painter = NULL;
    invalidate();
}

QPainter*
Grid::painter()
{
    return _painter;
}

bool
Grid::showPageNumber() const
{
    return _showPageNumber;
}

bool
Grid::showCreateDate() const
{
    return _showCreateDate;
}

void
Grid::setShowPageNumber(bool show)
{
    _showPageNumber = show;
}

void
Grid::setShowCreateDate(bool show)
{
    _showCreateDate = show;
}

int
Grid::pageNumber()
{
    return _page;
}

bool
Grid::lastPage()
{
    return _lastPage;
}

// Just call Frame::addChild but needs to be implemented since header
// file moves addChild() into protected.
void
Grid::addChild(Frame* child)
{
    Frame::addChild(child);
}

// Just call Frame::removeChild but needs to be implemented since header
// file moves removeChild() into protected.
void
Grid::removeChild(Frame* child)
{
    Frame::removeChild(child);
}

void
Grid::preferredSizes(QValueVector<fixed>& rowSize,
		     QValueVector<fixed>& columnSize)
{
    // Initialize vectors
    rowSize.clear();
    columnSize.clear();
    int row, col;
    for (row = 0; row < rows(); ++row)
	rowSize.push_back(0.0);
    for (col = 0; col < _columns; ++col)
	columnSize.push_back(0.0);

    // Check span 1,1 cells
    for (row = 0; row < rows(); ++row) {
	for (col = 0; col < _columns; ++col) {
	    Cell& cell = _rows[row][col];
	    if (cell.rowSpan != 1) continue;
	    if (cell.columnSpan != 1) continue;

	    fixed cellWidth, cellHeight;
	    cellPreferredSize(row, col, cellWidth, cellHeight);

	    if (cellWidth > columnSize[col])
		columnSize[col] = cellWidth;
	    if (cellHeight > rowSize[row])
		rowSize[row] = cellHeight;
	}
    }

    // Verify against other span cells
    for (row = 0; row < rows(); ++row) {
	for (col = 0; col < _columns; ++col) {
	    Cell& cell = _rows[row][col];
	    if (cell.rowSpan == 1 && cell.columnSpan == 1) continue;

	    fixed cellWidth, cellHeight;
	    cellPreferredSize(row, col, cellWidth, cellHeight);

	    // Get current total area using spans
	    fixed areaWidth = 0.0;
	    int i;
	    for (i = 0; i < cell.columnSpan; ++i)
		areaWidth += columnSize[col + i];
	    fixed areaHeight = 0.0;
	    for (i = 0; i < cell.rowSpan; ++i)
		areaHeight += rowSize[row + i];

	    // If current height too small, alloc more to rows using weight
	    if (areaHeight < cellHeight) {
		fixed heightDiff = cellHeight - areaHeight;
		fixed totalWeight = 0.0;
		fixed alloc = 0.0;

		for (i = 0; i < cell.rowSpan; ++i)
		    totalWeight += _rowInfo[row + i].weight;

		for (i = 0; i < cell.rowSpan; ++i) {
		    fixed f;
		    if (totalWeight == 0)
			f = 1 / double(cell.rowSpan);
		    else
			f = _rowInfo[row + i].weight / totalWeight;
		    fixed change = f * heightDiff;
		    rowSize[row + i] += change;
		    alloc += change;
		}

		if (alloc != heightDiff) {
		    int i = rand() % cell.rowSpan;
		    rowSize[row + i] += heightDiff - alloc;
		}
	    }

	    // If current width too small, alloc more to cols using weight
	    if (areaWidth < cellWidth) {
		fixed widthDiff = cellWidth - areaWidth;
		fixed totalWeight = 0.0;
		fixed alloc = 0.0;

		for (i = 0; i < cell.columnSpan; ++i)
		    totalWeight += _columnInfo[col + i].weight;

		for (i = 0; i < cell.columnSpan; ++i) {
		    fixed f;
		    if (totalWeight == 0)
			f = 1 / double(cell.columnSpan);
		    else
			f = _columnInfo[col + i].weight / totalWeight;
		    fixed change = f * widthDiff;
		    columnSize[col + i] += change;
		    alloc += change;
		}

		if (alloc != widthDiff) {
		    int i = rand() % cell.columnSpan;
		    columnSize[col + i] += widthDiff - alloc;
		}
	    }
	}
    }
}

// Get preferred size of a cell taking into account padding and min/max sizes
void
Grid::cellPreferredSize(int row, int col, fixed& width, fixed& height)
{
    width = 0.0;
    height = 0.0;
    Cell& cell = _rows[row][col];

    // Get content size if there is any
    if (cell.frame) cell.frame->preferredSize(width, height);

    // Add on the padding
    width += 2 * _columnInfo[col].pad;
    height += 2 * _rowInfo[row].pad;

    // Adjust for minimum sizes
    if (width < _columnInfo[col].minSize)
	width = _columnInfo[col].minSize;
    if (height < _rowInfo[row].minSize)
	height = _rowInfo[row].minSize;
    if (width > _columnInfo[col].maxSize)
	width = _columnInfo[col].maxSize;
    if (height > _rowInfo[row].maxSize)
	height = _rowInfo[row].maxSize;
}

// Recalculate the preferred size and change size to match.
void
Grid::recalculate()
{
    if (_valid) return;

    // Get new preferred size
    QValueVector<fixed> rowSize;
    QValueVector<fixed> columnSize;
    preferredSizes(rowSize, columnSize);

    // Set column widths and add up to set grid width
    fixed width = 0.0;
    for (int col = 0; col < _columns; ++col) {
	_columnInfo[col].size = columnSize[col];
	width += columnSize[col];
    }
    _width = width;
    _prefWidth = width;

    // Set row heights and add up to set grid height
    fixed height = 0.0;
    for (int row = 0; row < rows(); ++row) {
	_rowInfo[row].size = rowSize[row];
	height += rowSize[row];
    }
    _height = height;
    _prefHeight = height;

    _valid = true;
    _needLayout = true;
}

void
Grid::layout()
{
    if (!_needLayout) return;
    _needLayout = false;

    fixed totalHeight = 0.0;
    int row, col;
    for (row = 0; row < rows(); ++row) {
	totalHeight += _rowInfo[row].size;
    }

    fixed totalWidth = 0.0;
    for (col = 0; col < _columns; ++col) {
	totalWidth += _columnInfo[col].size;
    }

    fixed rowOffset = 0.0;
    fixed columnOffset = (_width - totalWidth) / 2;

    if (rowOffset < 0) rowOffset = 0;
    if (columnOffset < 0) columnOffset = 0;

    fixed y = rowOffset;
    for (row = 0; row < rows(); ++row) {
	fixed x = columnOffset;
	for (col = 0; col < _columns; ++col) {
	    Cell& cell = _rows[row][col];
	    if (cell.frame == NULL) {
		x += _columnInfo[col].size;
		continue;
	    }

	    fixed width = 0;
	    int i;
	    for (i = 0; i < cell.columnSpan; ++i)
		width += _columnInfo[col + i].size;

	    fixed height = 0;
	    for (i = 0; i < cell.rowSpan; ++i)
		height += _rowInfo[row + i].size;

	    fixed prefWidth, prefHeight;
	    cell.frame->preferredSize(prefWidth, prefHeight);

	    fixed cellX = 0.0;
	    fixed cellY = 0.0;
	    fixed cellWidth = prefWidth;
	    fixed cellHeight = prefHeight;
	    int sticky = cell.sticky;

	    // Adjust for Top/Bottom sticky
	    switch (sticky & (Top|Bottom)) {
	    case 0:
		// Center
		cellY = (height - prefHeight) / 2;
		break;
	    case Top:
		break;
	    case Bottom:
		cellY = height - prefHeight;
		break;
	    case Top|Bottom:
		// Expand
		cellHeight = height;
		break;
	    }

	    // Adjust for Right/Left sticky
	    switch (sticky & (Right|Left)) {
	    case 0:
		// Center
		cellX = (width - prefWidth) / 2;
		break;
	    case Left:
		break;
	    case Right:
		cellX = width - prefWidth;
		break;
	    case Left|Right:
		// Expand
		cellWidth = width;
		break;
	    }

	    if (cellX < 0) cellX = 0;
	    if (cellY < 0) cellY = 0;

	    cell.frame->resize(cellWidth, cellHeight);
	    cell.frame->move(x + cellX, y + cellY);
	    cell.frame->layout();

	    x += _columnInfo[col].size;
	}
	y += _rowInfo[row].size;
    }
}

void
Grid::checkRow(int& row)
{
    if (row == USE_NEXT) row = _lastRow + 1;
    if (row == USE_CURR && _lastRow == -1) row = 0;
    if (row == USE_CURR) row = _lastRow;

    assert(row >= 0);
    while (row >= rows()) {
	_rows.push_back(new Cell[_columns]);
	_rowInfo.push_back(Info(_defaultRowPad));
    }
}

void
Grid::checkColumn(int& column)
{
    if (column == USE_NEXT) column = _lastColumn + 1;
    if (column == USE_CURR && _lastColumn == -1) column = 0;
    if (column == USE_CURR) column = _lastColumn;

    assert(column >= 0 && column < _columns);
}

// Return a grid from a QListView.
Grid*
Grid::buildGrid(ListView* list, const QString& title)
{
    int columns = list->columns();
    int cols = columns * 2 - 1;

    // Setup grid
    Grid* grid = new Grid(cols);

    // Add header
    if (!title.isEmpty()) {
	QStringList lines = QStringList::split("\n", title);

	Grid* header = new Grid(1, grid);
	header->setColumnWeight(0, 1);

	int size = 20;
	for (unsigned int i = 0; i < lines.count(); ++i) {
	    TextFrame* text = new TextFrame(lines[i], header);
	    text->setFont(Font("Times", size));
	    header->set(USE_NEXT, 0, text);
	    size = 16;
	}

	grid->set(0, 0, 1, cols, header, Grid::AllSides);
	grid->set(USE_NEXT, 0, "");
    }

    // Setup columns
    int row = grid->rows();
    for (int column = 0; column < columns; ++column) {
	int col = column * 2;
	int align = list->columnAlignment(column);
	int sticky = 0;
	if (align & Qt::AlignLeft) sticky = Grid::Left;
	if (align & Qt::AlignRight) sticky = Grid::Right;
	if (align & Qt::AlignHCenter) sticky = Grid::Center;

	grid->setColumnSticky(col, sticky);
	grid->setColumnPad(col, 8);
	grid->set(row, col, list->columnText(column));
	grid->setBorder(row, col, Grid::Bottom);

	if (column < columns - 1)
	    grid->set(row, col + 1, " ");
    }
    grid->set(USE_NEXT, 0, " ");
    grid->setHeaderRows(grid->rows());

    // Add lines
    QListViewItemIterator it(list);
    for (; it.current(); ++it) {
	ListViewItem* item = (ListViewItem*)it.current();

	grid->set(USE_NEXT, 0, "");
	for (int column = 0; column < columns; ++column) {
	    int col = column * 2;
	    QString text = item->text(column);
	    if (item->value(column).type() == Variant::BOOL)
		text = item->value(column).toBool() ? "x" : " ";
	    grid->set(USE_CURR, col, text);
	}
    }

    return grid;
}

void
Grid::print()
{
    qDebug("%sgrid: %.2f,%.2f %.2fx%.2f rows=%d cols=%d", indentText(),
	   _x.toDouble(), _y.toDouble(), _width.toDouble(), _height.toDouble(),
	    rows(), _columns);
    _indent += 4;

    int row, col;
    for (row = 0; row < rows(); ++row) {
	for (col = 0; col < _columns; ++col) {
	    Cell& cell = _rows[row][col];
	    if (cell.frame == NULL) continue;

	    qDebug("%s%d,%d is:", indentText(), row, col);
	    _indent += 4;
	    cell.frame->print();
	    _indent -= 4;
	}
    }

    for (row = 0; row < rows(); ++row) {
	qDebug("%srow %d: weight=%.2f size=%.2f", indentText(), row,
		_rowInfo[row].weight.toDouble(),
		_rowInfo[row].size.toDouble());
    }

    for (col = 0; col < _columns; ++col) {
	qDebug("%scol %d: weight=%.2f size=%.2f", indentText(), col,
		_columnInfo[col].weight.toDouble(),
		_columnInfo[col].size.toDouble());
    }

    _indent -= 4;
}
