// $Id: table.cpp,v 1.48 2004/02/03 00:56:02 arandell Exp $
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

#include "table.h"
#include "row_header.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qpopupmenu.h>

Table::Table(QWidget* parent, const char* name)
    : QScrollView(parent, name)
{
    setFocusPolicy(ClickFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));

    // Enable clipper and set background mode
    enableClipper(true);
    viewport()->setBackgroundMode(PaletteBase);
    setResizePolicy(Manual);

    _leftHeader = new RowHeader(this);
    _leftHeader->setOrientation(Vertical);
    _leftHeader->setTracking(true);
    _leftHeader->setMovingEnabled(false);

    _topHeader = new QHeader(this);
    _topHeader->setOrientation(Horizontal);
    _topHeader->setTracking(true);
    _topHeader->setMovingEnabled(false);

    setMargins(60, fontMetrics().height() + 4, 0, 0);

    _row = -1;
    _col = -1;
    _showGrid = true;
    _localTab = true;
    _defaultRowHeight = fontMetrics().height() + 10;
    _displayRows = 4;
    _editor = NULL;
    _ignoreFocus = false;
    _popupRow = -1;
    _dataColumns = -1;
    _userInsertDelete = true;

    connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
	    _topHeader, SLOT(setOffset(int)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
	    _leftHeader, SLOT(setOffset(int)));
    connect(_topHeader, SIGNAL(sizeChange(int,int,int)),
	    SLOT(columnWidthChanged(int,int,int)));
    connect(_leftHeader, SIGNAL(sizeChange(int,int,int)),
	    SLOT(rowHeightChanged(int)));

    installEventFilter(this);
    _leftHeader->installEventFilter(this);
    _topHeader->installEventFilter(this);
}

Table::~Table()
{
    disconnect();
    _leftHeader->removeEventFilter(this);
    _topHeader->removeEventFilter(this);
    removeEventFilter(this);

    // Delete the rows
    for (int row = 0; row < rows(); ++row)
	delete _rows[row];

    // Delete the columns
    for (int col = 0; col < columns(); ++col)
	delete _columns[col];
}

void
Table::stopUserInsertDelete()
{
    _userInsertDelete = false;
}

bool
Table::showGrid() const
{
    return _showGrid;
}

bool
Table::localTab() const
{
    return _localTab;
}

int
Table::displayRows() const
{
    return _displayRows;
}

void
Table::setShowGrid(bool show)
{
    _showGrid = show;
    updateContents(0, 0, contentsWidth(), contentsHeight());
}

void
Table::setLocalTab(bool flag)
{
    _localTab = flag;
}

void
Table::setDisplayRows(int rows)
{
    _displayRows = rows;
}

void
Table::addColumn(TableColumn* column)
{
    _columns.push_back(column);
    _topHeader->addLabel(column->name, column->width);
    if (_col < 0) _col = 0;
}

void
Table::setColumnName(int col, const QString& name)
{
    _topHeader->setLabel(col, name);
    _columns[col]->name = name;
}

int
Table::columns() const
{
    return _columns.size();
}

int
Table::dataColumns() const
{
    if (_dataColumns == -1) return columns();
    return _dataColumns;
}

void
Table::setDataColumns(int columns)
{
    _dataColumns = columns;
}

TableColumn*
Table::column(int col)
{
    return _columns[col];
}

void
Table::appendRow(TableRow* row)
{
    _rows.push_back(row);
    _leftHeader->addLabel(QString::null, _defaultRowHeight);
    emit rowInserted(rows() - 1);

    QRect lastCell(cellGeometry(rows() - 1, columns() - 1));
    resizeContents(lastCell.right() + 1, lastCell.bottom() + 1);
    updateGeometries();
    viewport()->update();

    if (_row == -1)
	setCurrentCell(0, 0);

    // TODO: check that there are editable columns
    setFocusPolicy(StrongFocus);
}

void
Table::deleteRow(int row)
{
    if (row < 0 || row >= rows()) return;

    TableRow* data = _rows[row];
    _rows.erase(_rows.begin() + row);
    _leftHeader->removeLabel(row);
    delete data;
    emit rowDeleted(row);

    QRect lastCell(cellGeometry(rows() - 1, columns() - 1));
    resizeContents(lastCell.right() + 1, lastCell.bottom() + 1);
    updateGeometries();
    viewport()->update();

    if (row < _row) {
	--_row;
	editorResize();
    } else if (row == _row) {
	if (_row >= rows())
	    --_row;

	if (rows() == 0) {
	    _row = -1;
	    if (_editor != NULL) {
		CellEditor* editor = _editor;
		_editor = NULL;
		editor->hide();
	    }
	} else if (_editor != NULL) {
	    _editor->setValue(_rows[_row]->value(_col));
	    editorResize();
	}
    }

    if (rows() == 0) {
	setFocusPolicy(ClickFocus);
	//focusNextPrevChild(true);
    }
}

void
Table::insertRow(int before, TableRow* row)
{
    if (before < 0 || before >= rows()) return;

    if (before == _row)
	endEdit();

    _rows.insert(_rows.begin() + before, row);
    _leftHeader->addLabel(QString::null, _defaultRowHeight);
    emit rowInserted(before);

    QRect lastCell(cellGeometry(rows() - 1, columns() - 1));
    resizeContents(lastCell.right() + 1, lastCell.bottom() + 1);
    updateGeometries();
    viewport()->update();

    if (_row > before) {
	++_row;
	editorResize();
    } else if (_row == before) {
	if (_editor != NULL) {
	    ++_row;
	    editorResize();
	} else {
	    startEdit();
	}
    }
}

void
Table::clear()
{
    for (unsigned int i = 0; i < _rows.size(); ++i)
	delete _rows[i];
    _rows.clear();

    while (_leftHeader->count() > 0)
	_leftHeader->removeLabel(0);

    resizeContents(0, 0);
    updateGeometries();
    viewport()->update();

    emit cellMoved(-1, _col);
    _row = -1;
    if (_editor != NULL) {
	CellEditor* editor = _editor;
	_editor = NULL;
	editor->hide();
    }

    setFocusPolicy(ClickFocus);
}

int
Table::rows() const
{
    return _rows.size();
}

TableRow*
Table::row(int row)
{
    return _rows[row];
}

void
Table::setCurrentCell(int row, int col)
{
    if (row < 0 || row >= rows()) return;
    if (col < 0 || col >= columns()) return;

    if (!endEdit()) return;
    emit cellMoved(row, col);

    int oldRow = _row;
    int oldCol = _col;
    _row = row;
    _col = col;

    repaintCell(oldRow, oldCol);
    repaintCell(row, col);
    ensureCellVisible(row, col);

    if (hasFocus()) startEdit();
}

int
Table::currentRow() const
{
    return _row;
}

int
Table::currentColumn() const
{
    return _col;
}

void
Table::ensureCellVisible(int row, int col)
{
    int w = columnWidth(col);
    int h = rowHeight(row);
    ensureVisible(columnPos(col) + w / 2, rowPos(row) + h / 2, w / 2, h / 2);
}

Variant
Table::cellValue(int row, int col, bool current)
{
    if (row < 0 || row >= rows() || col < 0 || col >= columns()) {
	return Variant();
    }

    // If currently editting the cell, try to return its current value
    if (row == _row && col == _col && _editor != NULL && current) {
	if (_editor->valid()) {
	    Variant value = _editor->value();
	    Variant oldValue = _rows[_row]->value(_col);
	    if (value != oldValue) {
		bool ok = true;
		emit cellValidate(_row, _col, value, ok);
		if (!ok) {
		    qApp->beep();
		    return oldValue;
		}

		_rows[row]->setValue(col, value);
		repaintCell(row, col);
		emit cellChanged(_row, _col, oldValue);
	    }
	    return value;
	}
    }

    return _rows[row]->value(col);
}

void
Table::setCellValue(int row, int col, Variant value)
{
    if (row < 0 || row >= rows() || col < 0 || col >= columns())
	return;

    // If currently editting the cell, set it too
    if (row == _row && col == _col && _editor != NULL) {
	_editor->setValue(value);
    }

    _rows[row]->setValue(col, value);
    repaintCell(row, col);
}

QSize
Table::sizeHint() const
{
    int width = leftMargin() + 5;
    for (unsigned int i = 0; i < _columns.size(); ++i)
	width += _columns[i]->width;
    int height = _displayRows * _defaultRowHeight + topMargin() + 5;

    if (vScrollBarMode() == AlwaysOn)
	width += verticalScrollBar()->width();

    if (hScrollBarMode() == AlwaysOn)
	height += horizontalScrollBar()->height();

    return QSize(width, height);
}

void
Table::setRowHeight(int row, int height)
{
    _leftHeader->resizeSection(row, height);
    _leftHeader->setResizeEnabled(height != 0, row);
    rowHeightChanged(row);
}

void
Table::setColumnWidth(int col, int width)
{
    int oldWidth = columnWidth(col);
    width = QMAX(width, columnMinWidth(col));
    width = QMIN(width, columnMaxWidth(col));
    if (width == oldWidth) return;

    _topHeader->resizeSection(col, width);
    _topHeader->setResizeEnabled(width != 0, col);
    columnWidthChanged(col, oldWidth, width);
}

QSize
Table::tableSize() const
{
    return QSize(columnPos(columns() - 1) + columnWidth(columns() - 1),
		 rowPos(rows() - 1) + rowHeight(rows() - 1));
}

QRect
Table::cellGeometry(int row, int col) const
{
    return QRect(columnPos(col), rowPos(row),
		 columnWidth(col), rowHeight(row));
}

int
Table::columnWidth(int col) const
{
    return _topHeader->sectionSize(col);
}

int
Table::rowHeight(int row) const
{
    return _leftHeader->sectionSize(row);
}

int
Table::columnPos(int col) const
{
    return _topHeader->sectionPos(col);
}

int
Table::rowPos(int row) const
{
    return _leftHeader->sectionPos(row);
}

int
Table::columnAt(int pos) const
{
    return _topHeader->sectionAt(pos);
}

int
Table::rowAt(int pos) const
{
    return _leftHeader->sectionAt(pos);
}

void
Table::setLeftMargin(int margin)
{
    setMargins(margin, topMargin(), rightMargin(), bottomMargin());
    updateGeometries();
}

void
Table::setTopMargin(int margin)
{
    setMargins(leftMargin(), margin, rightMargin(), bottomMargin());
    updateGeometries();
}

void
Table::rowHeightChanged(int row)
{
    updateContents(0, rowPos(row), contentsWidth(), contentsHeight());

    QSize size = tableSize();
    int oldHeight = contentsHeight();
    resizeContents(size.width(), size.height());
    if (contentsHeight() < oldHeight)
	repaintContents(0, contentsHeight(), contentsWidth(),
			oldHeight - size.height() + 1, true);
    else
	repaintContents(0, oldHeight, contentsWidth(),
			size.height() - oldHeight + 1, false);

    if (row <= _row && _editor != NULL) {
	editorResize();
    }

    updateGeometries();
}

void
Table::columnWidthChanged(int col, int, int toWidth)
{
    if (toWidth < columnMinWidth(col)) {
	setColumnWidth(col, columnMinWidth(col));
	return;
    }
    if (toWidth > columnMaxWidth(col)) {
	setColumnWidth(col, columnMaxWidth(col));
	return;
    }

    updateContents(columnPos(col), 0, contentsWidth(), contentsHeight());

    QSize size = tableSize();
    int oldWidth = contentsWidth();
    resizeContents(size.width(), size.height());
    if (contentsWidth() < oldWidth)
	repaintContents(size.width(), 0, oldWidth - size.width() + 1,
			contentsHeight(), true);
    else
	repaintContents(oldWidth, 0, size.width() - oldWidth + 1,
			contentsHeight(), false);

    if (col <= _col && _editor != NULL) {
	editorResize();
    }

    updateGeometries();
    //columnExpandShrink(col);
}

void
Table::editorResize()
{
    if (_editor == NULL) return;
    _editor->resize(columnWidth(_col) - 2, rowHeight(_row) - 2);
    _editor->move(columnPos(_col) + 1, rowPos(_row) + 1);
}

void
Table::updateGeometries()
{
    static bool inUpdateGeometries = false;
    if (inUpdateGeometries) return;
    inUpdateGeometries = true;

    QSize ts = tableSize();
    if (_topHeader->offset() &&
	 ts.width() < _topHeader->offset() + _topHeader->width())
	horizontalScrollBar()->setValue(ts.width() - _topHeader->width());
    if (_leftHeader->offset() &&
	ts.height() < _leftHeader->offset() + _leftHeader->height())
	verticalScrollBar()->setValue(ts.height() - _leftHeader->height());

    _leftHeader->setGeometry(2, topMargin() + 2,
			     leftMargin(), visibleHeight());
    _topHeader->setGeometry(leftMargin() + 2, 2,
			    visibleWidth(), topMargin());

    inUpdateGeometries = false;
}

void
Table::drawContents(QPainter *p, int cx, int cy, int cw, int ch)
{
    int colfirst = columnAt(cx);
    int collast = columnAt(cx + cw);
    int rowfirst = rowAt(cy);
    int rowlast = rowAt(cy + ch);

    if (rowfirst == -1 || colfirst == -1) {
	paintEmptyArea(p, cx, cy, cw, ch);
	return;
    }

    if (rowlast == -1)
	rowlast = rows() - 1;
    if (collast == -1)
	collast = columns() - 1;

    // Go through the rows
    for (int r = rowfirst; r <= rowlast; ++r) {
	// Get row position and height
	int rowp = rowPos(r);
	int rowh = rowHeight(r);

	// Go through the columns in row r
	for (int c = colfirst; c <= collast; ++c) {
	    // Get position and width of column c
	    int colp = columnPos(c);
	    int colw = columnWidth(c);
	    int oldrp = rowp;
	    int oldrh = rowh;
	    if (colw == 0) continue;

	    // Translate painter and draw the cell
	    p->saveWorldMatrix();
	    p->translate(colp, rowp);
	    paintCell(p, r, c, QRect(colp, rowp, colw, rowh));
	    p->restoreWorldMatrix();

	    rowp = oldrp;
	    rowh = oldrh;
	}
    }
    // Paint empty rects
    paintEmptyArea(p, cx, cy, cw, ch);
}

void
Table::paintCell(QPainter* p, int row, int col, const QRect& cr)
{
    int w = cr.width();
    int h = cr.height();
    int x2 = w - 1;
    int y2 = h - 1;
    if (w == 0) return;

    // Draw indication of current cell
    p->save();
    p->setBrush(NoBrush);
    if (hasFocus() && row == _row && col == _col) {
	p->setPen(QPen(black, 1));
	p->drawRect(0, 0, w - 1, h - 1);
    } else {
	p->setPen(QPen(white, 1));
	p->drawRect(0, 0, w - 1, h - 1);
    }
    p->restore();

    _columns[col]->paint(row, col, p);

    if (_showGrid) {
	// Draw our lines
	QPen pen(p->pen());
	p->setPen(colorGroup().mid());
	p->drawLine(x2, 0, x2, y2);
	p->drawLine(0, y2, x2, y2);
	p->setPen(pen);
    }
}

void
Table::paintEmptyArea(QPainter *p, int cx, int cy, int cw, int ch)
{
    // Region of the rect we should draw
    QRegion reg(QRect(cx, cy, cw, ch));
    // Subtract the table from it
    reg = reg.subtract(QRect(QPoint(0, 0), tableSize()));
    p->save();
    p->setClipRegion(reg);
    p->fillRect(cx, cy, cw, ch, colorGroup().brush(QColorGroup::Base));
    p->restore();
}

void
Table::contentsMousePressEvent(QMouseEvent* e)
{
    int row = rowAt(e->pos().y());
    int col = columnAt(e->pos().x());
    if (row < 0) row = rows() - 1;

    setFocus();
    if (!endEdit()) return;
    setCurrentCell(row, col);
    emit cellClicked(row, col);

    if (row == _row && col == _col && _editor != NULL) {
#if 0
	QPoint pos(e->pos().x() - columnPos(col), e->pos().y() - rowPos(row));
	QMouseEvent* m = new QMouseEvent(e->type(), pos, e->globalPos(),
					 e->button(), e->state());
	qDebug("post click in editor");
	qApp->postEvent(_editor->widget(), m);
#endif
    }
}

void
Table::contentsMouseReleaseEvent(QMouseEvent* e)
{
    int row = rowAt(e->pos().y());
    int col = columnAt(e->pos().x());

    if (row == _row && col == _col && _editor != NULL) {
#if 0
	QPoint pos(e->pos().x() - columnPos(col), e->pos().y() - rowPos(row));
	QMouseEvent* m = new QMouseEvent(e->type(), pos, e->globalPos(),
					 e->button(), e->state());
	qDebug("post release to editor");
	qApp->postEvent(_editor->widget(), m);
#endif
    }
}

void
Table::resizeEvent(QResizeEvent* e)
{
    QScrollView::resizeEvent(e);
    updateGeometries();

    // Calculate minimum width
    int targetWidth = visibleWidth() - 1;
    int minWidth = 0;
    int preferredWidth = 0;
    int col;
    for (col = 0; col < columns(); ++col) {
	minWidth += columnMinWidth(col);
	preferredWidth += _columns[col]->width;
    }

    // If no extra space, done
    if (minWidth > targetWidth)
	return;

    // Calculate new widths based on the current ratio
    int* width = new int[columns()];
    bool* widthDone = new bool[columns()];
    int allocated = 0;
    for (col = 0; col < columns(); ++col) {
	double ratio = _columns[col]->width / (double)preferredWidth;
	width[col] = int(ratio * targetWidth + .1);
	widthDone[col] = false;
	allocated += width[col];
    }
    width[0] += targetWidth - allocated;

    // Adjust for minimum/maximum fields
    while (1) {
	int remain = 0;
	int remainPreferred = 0;
	int remainCnt = 0;
	int badCnt = 0;

	// Add back any that are outside minimum/maximum
	for (col = 0; col < columns(); ++col) {
	    if (widthDone[col]) continue;
	    if (width[col] < columnMinWidth(col)) {
		remain -= columnMinWidth(col) - width[col];
		width[col] = columnMinWidth(col);
		widthDone[col] = true;
		++badCnt;
	    } else if (width[col] > columnMaxWidth(col)) {
		remain -= columnMaxWidth(col) - width[col];
		width[col] = columnMaxWidth(col);
		widthDone[col] = true;
		++badCnt;
	    } else {
		remain += width[col];
	    }

	    if (!widthDone[col]) {
		remainPreferred += _columns[col]->width;
		++remainCnt;
	    }
	}

	if (remain == 0 || remainCnt == 0 || badCnt == 0)
	    break;

	int allocated = 0;
	int first = -1;
	for (col = 0; col < columns(); ++col) {
	    if (widthDone[col]) continue;
	    if (first == -1) first = col;
	    double ratio = _columns[col]->width / (double)remainPreferred;
	    width[col] = int(ratio * remain + .1);
	    allocated += width[col];
	}
	width[first] += remain - allocated;
    }

    // Set widths
    for (col = 0; col < columns(); ++col) {
	if (width[col] != columnWidth(col))
	    setColumnWidth(col, width[col]);
    }

    delete [] width;
    delete [] widthDone;
}

void
Table::columnExpandShrink(int)
{
    if (contentsWidth() == 1) return;

    static bool inExpand = false;
    static int col = 0;

    if (inExpand) return;
    inExpand = true;

    int totalWidth = 0;
    for (int i = 0; i < columns(); ++i)
	totalWidth += _columns[i]->width;

    int difference = visibleWidth() - contentsWidth();
    while (difference > 0) {
	setColumnWidth(col, columnWidth(col) + 1);
	--difference;
	++col;
	if (col >= columns()) col = 0;
    }
    while (difference < 0) {
	setColumnWidth(col, columnWidth(col) - 1);
	++difference;
	++col;
	if (col >= columns()) col = 0;
    }
    inExpand = false;
}

void
Table::showEvent(QShowEvent* e)
{
    QScrollView::showEvent(e);
    QRect r(cellGeometry(rows() - 1, columns() - 1));
    resizeContents(r.right() + 1, r.bottom() + 1);
    updateGeometries();
}

void
Table::setUpdatesEnabled(bool flag)
{
    QScrollView::setUpdatesEnabled(flag);
}

void
Table::updateCell(int row, int col)
{
    static bool inUpdateCell = false;
    if (inUpdateCell || row == -1 || col == -1) return;
    inUpdateCell = true;

    QRect cg = cellGeometry(row, col);
    QRect r(contentsToViewport(QPoint(cg.x() - 2, cg.y() - 2)),
	    QSize(cg.width() + 4, cg.height() + 4));
    QApplication::postEvent(viewport(), new QPaintEvent(r, false));
    inUpdateCell = false;
}

void
Table::repaintCell(int row, int col)
{
    if (row == -1 || col == -1) return;

    QRect cg = cellGeometry(row, col);
    QRect rect(QPoint(cg.x() - 2, cg.y() - 2),
	       QSize(cg.width() + 4, cg.height() + 4));

    updateContents(rect);
}

void
Table::repaintRow(int row)
{
    if (row == -1) return;

    QRect cg = cellGeometry(row, 0);
    QRect rect(QPoint(0, cg.y() - 2),
	    QSize(contentsWidth(), rowHeight(row) + 4));

    updateContents(rect);
}

void
Table::startEdit()
{
    setFocus();
    if (_row < 0 || _row >= rows()) return;
    if (_col < 0 || _col >= columns()) return;

    CellEditor* editor = _columns[_col]->editor;
    if (editor == NULL) return;

    emit cellStartEdit(_row, _col);
    if (_editor == NULL) {
	_editor = editor;
	_editor->setValue(cellValue(_row, _col, false));
    }

    editorResize();
    _editor->show();
    _editor->setFocus();
}

bool
Table::endEdit(bool grabFocus)
{
    if (_editor == NULL) return true;

    if (!_editor->valid()) {
	qApp->beep();
	return false;
    }

    Variant value = _editor->value();
    Variant oldValue = _rows[_row]->value(_col);
    if (value != oldValue) {
	bool ok = true;
	emit cellValidate(_row, _col, value, ok);
	if (!ok) {
	    qApp->beep();
	    return false;
	}

	setCellValue(_row, _col, value);
	emit cellChanged(_row, _col, oldValue);
    }

    CellEditor* editor = _editor;
    _editor = NULL;

    _ignoreFocus = true;
    editor->hide();
    if (grabFocus) setFocus();
    _ignoreFocus = false;

    return true;
}

void
Table::adjustColumn(int col)
{
    int width = _topHeader->fontMetrics().width(_topHeader->label(col)) + 10;
    width = QMAX(width, 20);
    for (int i = 0; i < rows(); ++i) {
	// TODO: get width of data
    }
    setColumnWidth(col, width);
}

void
Table::adjustRow(int row)
{
    int height = 20;
    height = QMAX(height, _leftHeader->fontMetrics().height());
    for (int i = 0; i < columns(); ++i) {
	// TODO: get height of data
    }
    setRowHeight(row, height);
}

void
Table::focusInEvent(QFocusEvent*)
{
    repaintCell(_row, _col);

    if (!_ignoreFocus)
	startEdit();
}

void
Table::focusOutEvent(QFocusEvent*)
{
    repaintCell(_row, _col);
}

bool
Table::eventFilter(QObject* o, QEvent* e)
{
    // If object was top header, check for popup window
    if (o == _topHeader) {
	if (e->type() == QEvent::MouseButtonPress && _userInsertDelete) {
	    QMouseEvent* m = (QMouseEvent*)e;
	    if (m->button() == RightButton) {
		QPopupMenu* menu = new QPopupMenu(this);
		menu->insertItem(tr("Add Row"), this, SLOT(appendRow()));
		menu->popup(m->globalPos(), 0);
		return true;
	    }
	}
	return QScrollView::eventFilter(o, e);
    }

    // If object was left header, check for popup window
    if (o == _leftHeader) {
	if (e->type() == QEvent::MouseButtonPress && _userInsertDelete) {
	    QMouseEvent* m = (QMouseEvent*)e;
	    if (m->button() == RightButton) {
		int row = _leftHeader->sectionAt(m->pos().y() +
						 _leftHeader->offset());
		QPopupMenu* menu = new QPopupMenu(this);
		if (row == -1) {
		    menu->insertItem(tr("Add Row"), this, SLOT(appendRow()));
		} else {
		    _popupRow = row;
		    menu->insertItem(tr("Delete Row"), this,SLOT(deleteRow()));
		    menu->insertItem(tr("Insert Row"), this,SLOT(insertRow()));
		}
		menu->popup(m->globalPos(), 0);
		return true;
	    }
	}
	return QScrollView::eventFilter(o, e);
    }

    // If focus lost in editor, end the edit
    if (e->type() == QEvent::FocusOut && o != this && !_ignoreFocus) {
	QFocusEvent* f = (QFocusEvent*)e;
	if (f->reason() != QFocusEvent::ActiveWindow &&
	    f->reason() != QFocusEvent::Popup) {
	    endEdit(false);
	}
    }

    if (e->type() != QEvent::KeyPress || !_localTab)
	return QScrollView::eventFilter(o, e);

    QKeyEvent* k = (QKeyEvent*)e;

    // If the widget isn't the table and its a special key, forward to
    // the table and ignore it in the widget
    if (o != this) {
	switch (k->key()) {
	case Key_Up:
	case Key_Down:
	case Key_Return:
	case Key_Enter:
	case Key_Tab:
	case Key_Backtab:
	    QKeyEvent* ke = new QKeyEvent(k->type(), k->key(), k->ascii(),
					  k->state(), "", k->isAutoRepeat(),
					  k->count());
	    qApp->postEvent(this, ke);
	    return true;
	}

	if ((k->key() == Key_Insert || k->key() == Key_Delete) &&
		k->state() & ControlButton) {
	    QKeyEvent* ke = new QKeyEvent(k->type(), k->key(), k->ascii(),
					  k->state(), "", k->isAutoRepeat(),
					  k->count());
	    qApp->postEvent(this, ke);
	    return true;
	}

	return QScrollView::eventFilter(o, e);
    }

    // Delete row if Ctrl-Delete pressed
    if (k->key() == Key_Delete && k->state() == ControlButton &&
	    _userInsertDelete) {
	deleteRow(_row);
	return true;
    }

    // Insert row if Ctrl-Insert pressed
    if (k->key() == Key_Insert && k->state() == ControlButton &&
	    _userInsertDelete) {
	insertRow(_row, new VectorRow(dataColumns()));
	return true;
    }

    bool leave = false;
    int leaveState = k->state();
    bool move = false;
    int newRow = _row;
    int newCol = _col;
    int type;

    switch (k->key()) {
    case Key_Up:
	move = true;
	if (!endEdit()) return true;
	nextCell(leave, newRow, newCol, MoveUp);
	if (!(k->state() & ShiftButton))
	    leave = false;
	break;
    case Key_Down:
	move = true;
	if (!endEdit()) return true;
	nextCell(leave, newRow, newCol, MoveDown);
	if (!(k->state() & ShiftButton))
	    leave = false;
	else
	    leaveState &= ~ShiftButton;
	break;
    case Key_Tab:
    case Key_Return:
    case Key_Enter:
	move = true;
	if (!endEdit()) return true;
	type = (k->state() & ShiftButton) ? MovePrev : MoveNext;
	nextCell(leave, newRow, newCol, type);
	break;
    case Key_Backtab:
	move = true;
	if (!endEdit()) return true;
	nextCell(leave, newRow, newCol, MovePrev);
	break;
    }

    // If nothing special, just pass on to regular filter
    if (!move && !leave) return QScrollView::eventFilter(o, e);

    // If leaving, end edit and change key to Tab
    if (leave) {
	if (!endEdit()) {
	    _ignoreFocus = true;
	    _editor->hide();
	    setFocus();
	    _ignoreFocus = false;
	}

	int leaveKey = (k->key() == Key_Backtab) ? Key_Backtab : Key_Tab;

	QKeyEvent n(k->type(), leaveKey, '\t', leaveState,
		    "", k->isAutoRepeat(), k->count());
	*k = n;
	return QScrollView::eventFilter(o, e);
    }

//    if (newRow == _row && newCol == _col) return true;
//    if (!endEdit()) return true;
    setCurrentCell(newRow, newCol);
    return true;
}

void
Table::nextCell(bool& leave, int& newRow, int& newCol, int type)
{
    switch (type) {
    case MoveUp:
	if (newRow == 0)
	    leave = true;
	else
	    newRow = QMAX(0, newRow - 1);
	break;
    case MoveDown:
	if (newRow == rows() - 1)
	    leave = true;
	else
	    newRow = QMIN(rows() - 1, newRow + 1);
	break;
    case MoveNext:
	++newCol;
	if (newCol >= columns()) {
	    ++newRow;
	    if (newRow >= rows())
		leave = true;
	    else
		newCol = 0;
	}
	break;
    case MovePrev:
	--newCol;
	if (newCol < 0) {
	    --newRow;
	    if (newRow < 0)
		leave = true;
	    else
		newCol = columns() - 1;
	}
	break;
    }

    emit focusNext(leave, newRow, newCol, type);
}

int
Table::columnMinWidth(int col) const
{
    return _columns[col]->minWidth;
}

int
Table::columnMaxWidth(int col) const
{
    return _columns[col]->maxWidth;
}

void
Table::appendRow()
{
    appendRow(new VectorRow(dataColumns()));
}

void
Table::insertRow()
{
    if (_popupRow != -1)
	insertRow(_popupRow, new VectorRow(dataColumns()));
}

void
Table::deleteRow()
{
    if (_popupRow != -1)
	deleteRow(_popupRow);
}
