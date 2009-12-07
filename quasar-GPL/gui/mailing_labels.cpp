// $Id: mailing_labels.cpp,v 1.29 2005/02/17 09:55:03 bpepers Exp $
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

#include "mailing_labels.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "list_view_item.h"
#include "card.h"
#include "card_select.h"
#include "grid.h"
#include "integer_edit.h"
#include "user_config.h"

#include <qlabel.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <assert.h>
#include <math.h>

struct LabelDef {
    LabelDef(char* nm, int rw, int cl, fixed pw, fixed ph, fixed lw,
	     fixed lh, fixed tm, fixed lm, fixed hg, fixed vg, int fs,
	     bool ls) : name(nm), rows(rw), columns(cl), paperWidth(pw),
			paperHeight(ph), labelWidth(lw), labelHeight(lh),
			topMargin(tm), leftMargin(lm), horizontalGap(hg),
			verticalGap(vg), fontSize(fs), landscape(ls) { };

    char* name;
    int rows;
    int columns;
    fixed paperWidth;
    fixed paperHeight;
    fixed labelWidth;
    fixed labelHeight;
    fixed topMargin;
    fixed leftMargin;
    fixed horizontalGap;
    fixed verticalGap;
    int fontSize;
    bool landscape;
};

LabelDef labelTypes[] = {
    LabelDef("Avery 5160", 10, 3, 612, 792, 189,  72, 36, 13.5,   9.0, 0, 10, false),
    LabelDef("Avery 5163",  5, 2, 612, 792, 288, 144, 36, 11.25, 13.5, 0, 16, false),
    LabelDef("Half Page",   2, 1, 612, 792, 576, 360, 36, 18.0,   0.0, 0, 24, false),
    LabelDef("Full Page",   1, 1, 792, 612, 720, 540, 36, 36.0,   0.0, 0, 32, true)
};
int labelTypeCnt = sizeof(labelTypes) / sizeof(LabelDef);

MailingLabels::MailingLabels(MainWindow* main)
    : QuasarWindow(main, "MailingLabels")
{
    _helpSource = "mailing_labels.html";

    QFrame* frame = new QFrame(this);

    _tabs = new QTabWidget(frame);

    // List for customers tab
    _customer = new ListView(_tabs);
    _tabs->addTab(_customer, tr("&Customers"));
    _customer->addTextColumn(tr("Name"), 30);
    _customer->addTextColumn(tr("Phone Number"), 20);
    _customer->addTextColumn(tr("Postal Code"), 20);
    _customer->addMoneyColumn(tr("Balance"));
    _customer->setSorting(0);
    _customer->setSelectionMode(QListView::Multi);
    _customer->setAllColumnsShowFocus(true);
    _customer->setShowSortIndicator(true);

    // List for vendor tab
    _vendor = new ListView(_tabs);
    _tabs->addTab(_vendor, tr("&Vendors"));
    _vendor->addTextColumn(tr("Name"), 30);
    _vendor->addTextColumn(tr("Phone Number"), 20);
    _vendor->addTextColumn(tr("Postal Code"), 20);
    _vendor->addMoneyColumn(tr("Balance"));
    _vendor->setSorting(0);
    _vendor->setSelectionMode(QListView::Extended);
    _vendor->setAllColumnsShowFocus(true);
    _vendor->setShowSortIndicator(true);

    // List for employee tab
    _employee = new ListView(_tabs);
    _tabs->addTab(_employee, tr("&Employees"));
    _employee->addTextColumn(tr("Name"), 30);
    _employee->addTextColumn(tr("Phone Number"), 20);
    _employee->addTextColumn(tr("Postal Code"), 20);
    _employee->setSorting(0);
    _employee->setSelectionMode(QListView::Extended);
    _employee->setAllColumnsShowFocus(true);
    _employee->setShowSortIndicator(true);

    // List for personal tab
    _personal = new ListView(_tabs);
    _tabs->addTab(_personal, tr("&Personal"));
    _personal->addTextColumn(tr("Name"), 30);
    _personal->addTextColumn(tr("Phone Number"), 20);
    _personal->addTextColumn(tr("Postal Code"), 20);
    _personal->setSorting(0);
    _personal->setSelectionMode(QListView::Extended);
    _personal->setAllColumnsShowFocus(true);
    _personal->setShowSortIndicator(true);

    QFrame* box1 = new QFrame(frame);
    QPushButton* all = new QPushButton(tr("All"), box1);
    QPushButton* none = new QPushButton(tr("None"), box1);

    all->setMinimumSize(none->sizeHint());
    connect(all, SIGNAL(clicked()), SLOT(slotSelectAll()));
    none->setMinimumSize(none->sizeHint());
    connect(none, SIGNAL(clicked()), SLOT(slotSelectNone()));

    QGridLayout* grid1 = new QGridLayout(box1);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(1, 1);
    grid1->addWidget(all, 0, 0, AlignLeft | AlignVCenter);
    grid1->addWidget(none, 0, 1, AlignLeft | AlignVCenter);

    QLabel* typeLabel = new QLabel(tr("Type:"), frame);
    _type = new QComboBox(frame);
    for (int i = 0; i < labelTypeCnt; ++i)
	_type->insertItem(labelTypes[i].name);

    QLabel* numberLabel = new QLabel(tr("Labels/Card:"), frame);
    _number = new IntegerEdit(frame);
    _number->setLength(4);

    QLabel* rowLabel = new QLabel(tr("Start Row:"), frame);
    _row = new IntegerEdit(frame);
    _row->setLength(4);

    QLabel* colLabel = new QLabel(tr("Start Column:"), frame);
    _col = new IntegerEdit(frame);
    _col->setLength(4);

    QFrame* box2 = new QFrame(frame);
    QPushButton* refresh = new QPushButton(tr("&Refresh"), box2);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), box2);
    QPushButton* print = new QPushButton(tr("&Print"), box2);

    refresh->setMinimumSize(refresh->sizeHint());
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));
    cancel->setMinimumSize(refresh->sizeHint());
    connect(cancel, SIGNAL(clicked()), SLOT(close()));
    print->setMinimumSize(refresh->sizeHint());
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    QGridLayout* grid2 = new QGridLayout(box2);
    grid2->setSpacing(6);
    grid2->setMargin(6);
    grid2->setColStretch(0, 1);
    grid2->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    grid2->addWidget(cancel, 0, 1, AlignRight | AlignVCenter);
    grid2->addWidget(print, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(0, 1);
    grid->setColStretch(1, 1);
    grid->addMultiCellWidget(_tabs, 0, 0, 0, 3);
    grid->addMultiCellWidget(box1, 1, 1, 0, 3);
    grid->addWidget(typeLabel, 2, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_type, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 3, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_number, 3, 1, AlignLeft | AlignVCenter);
    grid->addWidget(rowLabel, 2, 2, AlignLeft | AlignVCenter);
    grid->addWidget(_row, 2, 3, AlignLeft | AlignVCenter);
    grid->addWidget(colLabel, 3, 2, AlignLeft | AlignVCenter);
    grid->addWidget(_col, 3, 3, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(box2, 4, 4, 0, 3);

    _type->setFocus();
    _number->setValue(1);
    _row->setValue(1);
    _col->setValue(1);

    setCaption(tr("Mailing Labels"));
    setCentralWidget(frame);
    finalize();

    QTimer::singleShot(0, this, SLOT(slotRefresh()));
}

MailingLabels::~MailingLabels()
{
}

void
MailingLabels::slotRefresh()
{
    _customer->clear();
    _vendor->clear();
    _employee->clear();
    _personal->clear();

    vector<Card> cards;
    vector<Id> card_ids;
    vector<fixed> balances;
    _quasar->db()->select(cards, CardSelect());
    _quasar->db()->cardBalances(QDate::currentDate(), INVALID_ID,
				card_ids, balances);

    for (unsigned int i = 0; i < cards.size(); ++i) {
	const Card& card = cards[i];

	fixed balance = 0.0;
	for (unsigned int j = 0; j < card_ids.size(); ++j) {
	    if (card_ids[j] == card.id()) {
		balance = balances[j];
		break;
	    }
	}

	ListViewItem* item;

	switch (card.dataType()) {
	case DataObject::CUSTOMER:
	    item = new ListViewItem(_customer, card.id());
	    item->setValue(0, card.name());
	    item->setValue(1, card.phoneNumber());
	    item->setValue(2, card.postal());
	    item->setValue(3, balance);
	    break;
	case DataObject::VENDOR:
	    item = new ListViewItem(_vendor, card.id());
	    item->setValue(0, card.name());
	    item->setValue(1, card.phoneNumber());
	    item->setValue(2, card.postal());
	    item->setValue(3, balance);
	    break;
	case DataObject::EMPLOYEE:
	    item = new ListViewItem(_employee, card.id());
	    item->setValue(0, card.name());
	    item->setValue(1, card.phoneNumber());
	    item->setValue(2, card.postal());
	    break;
	case DataObject::PERSONAL:
	    item = new ListViewItem(_personal, card.id());
	    item->setValue(0, card.name());
	    item->setValue(1, card.phoneNumber());
	    item->setValue(2, card.postal());
	    break;
	default:
	    assert(0);
	    break;
	}
    }
}

void
MailingLabels::slotSelectAll()
{
    QWidget* page = _tabs->currentPage();

    if (page == _customer)
	_customer->selectAll(true);
    else if (page == _vendor)
	_vendor->selectAll(true);
    else if (page == _employee)
	_employee->selectAll(true);
    else if (page == _personal)
	_personal->selectAll(true);
}

void
MailingLabels::slotSelectNone()
{
    QWidget* page = _tabs->currentPage();

    if (page == _customer)
	_customer->selectAll(false);
    else if (page == _vendor)
	_vendor->selectAll(false);
    else if (page == _employee)
	_employee->selectAll(false);
    else if (page == _personal)
	_personal->selectAll(false);
}

void
MailingLabels::slotPrint()
{
    int startRow = _row->getInt() - 1;
    int startColumn = _col->getInt() - 1;
    int count = _number->getInt();
    LabelDef defn = *(labelTypes + _type->currentItem());

    if (startRow < 0 || startRow >= defn.rows) return;
    if (startColumn < 0 || startColumn >= defn.columns) return;
    if (count < 1) return;

    // Add on customer selections
    vector<Id> card_ids;
    QListViewItem* child = _customer->firstChild();
    while (child != NULL) {
	if (child->isSelected())
	    card_ids.push_back(((ListViewItem*)child)->id);
	child = child->nextSibling();
    }

    // Add on vendor selections
    child = _vendor->firstChild();
    while (child != NULL) {
	if (child->isSelected())
	    card_ids.push_back(((ListViewItem*)child)->id);
	child = child->nextSibling();
    }

    // Add on employee selections
    child = _employee->firstChild();
    while (child != NULL) {
	if (child->isSelected())
	    card_ids.push_back(((ListViewItem*)child)->id);
	child = child->nextSibling();
    }

    // Add on personal selections
    child = _personal->firstChild();
    while (child != NULL) {
	if (child->isSelected())
	    card_ids.push_back(((ListViewItem*)child)->id);
	child = child->nextSibling();
    }

    // Check thats something was selected
    if (card_ids.size() == 0) {
	QMessageBox::critical(this, tr("Error"),
			      tr("No cards selected to print"));
	return;
    }

    UserConfig config;
    config.load();

    // Setup printer output
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setFullPage(true);
    printer.setColorMode(QPrinter::Color);

    if (config.preferA4)
	printer.setPageSize(QPrinter::A4);
    else
	printer.setPageSize(QPrinter::Letter);

    if (!printer.setup(this))
	return;

    if (printer.pageSize() == QPrinter::A4) {
	config.preferA4 = true;
	config.save(true);
    }

    // Adjust for DPI
    QPaintDeviceMetrics metrics(&printer);
    double xFactor = metrics.logicalDpiX() / 72.0;
    double yFactor = metrics.logicalDpiY() / 72.0;
    defn.paperWidth = defn.paperWidth * xFactor;
    defn.paperHeight = defn.paperHeight * yFactor;
    defn.labelWidth = defn.labelWidth * xFactor;
    defn.labelHeight = defn.labelHeight * yFactor;
    defn.leftMargin = defn.leftMargin * xFactor;
    defn.topMargin = defn.topMargin * yFactor;
    defn.horizontalGap = defn.horizontalGap * xFactor;
    defn.verticalGap = defn.verticalGap * yFactor;

    // Check that paper size matches
    if (fabs(metrics.width() - defn.paperWidth) > 2.0 ||
	fabs(metrics.height() - defn.paperHeight) > 2.0) {
	QString message = tr("The paper size chosen doesn't match the size\n"
	    "used for the labels selected.");
	QMessageBox::critical(NULL, tr("Paper Size Error"), message);
	return;
    }

    // Start generating grid
    Grid* grid = new Grid(defn.columns * 2 + 1);
    Font::defaultFont = Font("Times", defn.fontSize);

    // Calculate right and bottom margins
    fixed rightMargin = defn.paperWidth - defn.leftMargin -
	(defn.labelWidth * defn.columns) - (defn.horizontalGap *
					      (defn.columns - 1));
    fixed bottomMargin = defn.paperHeight - defn.topMargin -
	(defn.labelHeight * defn.rows) - (defn.verticalGap *
					    (defn.rows - 1));

    // Set column widths
    grid->setColumnWidth(0, defn.leftMargin);
    for (int col = 0; col < defn.columns; ++col) {
	grid->setColumnWidth(col * 2 + 1, defn.labelWidth);
	grid->setColumnWidth(col * 2 + 2, defn.horizontalGap);
    }
    grid->setColumnWidth(defn.columns * 2, rightMargin);

    unsigned int pos = 0;
    int pageRow = 0;
    int skip = startRow * defn.columns + startColumn;
    while (pos < card_ids.size()) {
	// Start a page
	grid->setRowHeight(pageRow, defn.topMargin);

	// Fill in all the cells
	for (int row = 0; row < defn.rows; ++row) {
	    for (int column = 0; column < defn.columns; ++column) {
		int gridRow = pageRow + row * 2 + 1;
		int gridColumn = column * 2 + 1;

		grid->setRowHeight(gridRow, defn.labelHeight);
		grid->setRowHeight(gridRow + 1, defn.verticalGap);

		if (pos >= card_ids.size()) continue;
		if (skip-- > 0) continue;

		Card card;
		_quasar->db()->lookup(card_ids[pos], card);
		grid->set(gridRow, gridColumn, card.address());

		--count;
		if (count == 0) {
		    count = _number->getInt();
		    ++pos;
		}
	    }
	}

	grid->setRowHeight(pageRow + defn.rows * 2, bottomMargin);
	pageRow += defn.rows * 2 + 1;
    }

    grid->print(this, &printer, false);
    delete grid;

    QString message = tr("Were the labels printed properly?");
    int choice = QMessageBox::information(this, tr("Finished?"), message,
					  tr("Yes"), tr("No"));
    if (choice == 0) close();
}
