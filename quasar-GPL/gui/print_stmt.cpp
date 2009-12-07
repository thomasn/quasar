// $Id: print_stmt.cpp,v 1.16 2005/01/30 04:25:31 bpepers Exp $
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

#include "print_stmt.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "lookup_edit.h"
#include "customer_lookup.h"
#include "store_lookup.h"
#include "group_lookup.h"
#include "invoice.h"
#include "receipt.h"
#include "customer.h"
#include "term.h"
#include "group.h"
#include "company.h"
#include "customer_select.h"
#include "gltx_select.h"
#include "line_edit.h"
#include "date_valcon.h"
#include "money_valcon.h"
#include "grid.h"
#include "page_number.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qvbox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qtimer.h>

PrintStmt::PrintStmt(MainWindow* main)
    : QuasarWindow(main, "PrintStmt")
{
    _helpSource = "print_stmt.html";

    QWidget* top = new QWidget(this);
    QWidget* frame = new QWidget(top);

    QLabel* storeLabel = new QLabel(tr("Store:"), frame);
    _store = new LookupEdit(new StoreLookup(_main, this), frame);
    _store->setFocusPolicy(ClickFocus);
    _store->setLength(20);
    storeLabel->setBuddy(_store);

    QGroupBox* printBy = new QGroupBox(tr("Print By"), frame);
    QGridLayout* printGrid = new QGridLayout(printBy, 4, 2,
					     printBy->frameWidth() * 2);
    printGrid->addRowSpacing(0, printBy->fontMetrics().height());

    _showAll = new QRadioButton(tr("All Customers"), printBy);
    _showGroup = new QRadioButton(tr("Group Match"), printBy);
    _showOne = new QRadioButton(tr("One Customer"), printBy);

    QLabel* groupLabel = new QLabel(tr("Group:"), printBy);
    _group = new LookupEdit(new GroupLookup(_main, this, Group::CUSTOMER),
			     printBy);
    groupLabel->setBuddy(_group);

    QLabel* custLabel = new QLabel(tr("Customer:"), printBy);
    _customer = new LookupEdit(new CustomerLookup(main, this), printBy);
    _customer->setLength(30);
    custLabel->setBuddy(_customer);

    QButtonGroup* showButtons = new QButtonGroup(this);
    showButtons->hide();
    showButtons->insert(_showAll);
    showButtons->insert(_showGroup);
    showButtons->insert(_showOne);
    connect(showButtons, SIGNAL(clicked(int)), SLOT(slotShowChanged(int)));

    printGrid->setColStretch(1, 1);
    printGrid->addWidget(_showAll, 1, 0, AlignLeft | AlignVCenter);
    printGrid->addWidget(_showGroup, 2, 0, AlignLeft | AlignVCenter);
    printGrid->addWidget(groupLabel, 2, 1, AlignLeft | AlignVCenter);
    printGrid->addWidget(_group, 2, 2, AlignLeft | AlignVCenter);
    printGrid->addWidget(_showOne, 3, 0, AlignLeft | AlignVCenter);
    printGrid->addWidget(custLabel, 3, 1, AlignLeft | AlignVCenter);
    printGrid->addWidget(_customer, 3, 2, AlignLeft | AlignVCenter);

    QGridLayout* frameGrid = new QGridLayout(frame);
    frameGrid->setSpacing(6);
    frameGrid->setMargin(6);
    frameGrid->setRowStretch(3, 1);
    frameGrid->setColStretch(1, 1);
    frameGrid->addWidget(storeLabel, 0, 0, AlignLeft | AlignVCenter);
    frameGrid->addWidget(_store, 0, 1, AlignLeft | AlignVCenter);
    frameGrid->addMultiCellWidget(printBy, 1, 1, 0, 1);

    QVBox* buttons = new QVBox(top);
    buttons->setMargin(4);
    buttons->setSpacing(4);

    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);
    connect(ok, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    QGridLayout* grid = new QGridLayout(top);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(0, 1);
    grid->addWidget(frame, 0, 0);
    grid->addWidget(buttons, 0, 1, AlignRight | AlignTop);

    _store->setId(_quasar->defaultStore());
    _showAll->setChecked(true);
    slotShowChanged(0);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    _showAll->setFocus();
    setCentralWidget(top);
    setCaption(tr("Print Statements"));
    finalize();
}

PrintStmt::~PrintStmt()
{
}

void
PrintStmt::slotShowChanged(int button)
{
    _group->setEnabled(button == 1);
    _customer->setEnabled(button == 2);
}

static Grid*
addressFrame(Grid* parent, const QString& text, bool topBig=false)
{
    Grid* grid = new Grid(1, parent);
    int start = 0;
    int index = -1;

    while (1) {
	index = text.find('\n', start);
	if (index == -1) break;

	QString line = text.mid(start, index - start);

	if (topBig) {
	    Font saveFont = Font::defaultFont;
	    Font::defaultFont = Font("Helvetica", 16, true);
	    grid->set(USE_NEXT, 0, line, Grid::Left);
	    Font::defaultFont = saveFont;
	    topBig = false;
	} else {
	    grid->set(USE_NEXT, 0, line, Grid::Left);
	}

	start = index + 1;
    }

    if (start != int(text.length()))
	grid->set(USE_NEXT, 0, text.right(text.length() - start), Grid::Left);

    return grid;
}

void
PrintStmt::slotOk()
{
    // Select customers
    vector<Customer> customers;
    CustomerSelect conditions;
    conditions.activeOnly = true;
    if (_showGroup->isChecked()) {
	conditions.group_id = _group->getId();
	if (conditions.group_id == INVALID_ID) return;
    }
    if (_showOne->isChecked()) {
	conditions.id = _customer->getId();
	if (conditions.id == INVALID_ID) return;
    }
    _quasar->db()->select(customers, conditions);

    Company company;
    _quasar->db()->lookup(company);
    QDate today = QDate::currentDate();

    // Process each customer
    QValueVector<Grid*> grids;
    for (unsigned int i = 0; i < customers.size(); ++i) {
	const Customer& customer = customers[i];
	if (!customer.printStatements()) continue;

	vector<Gltx> gltxs;
	GltxSelect conditions;
	conditions.activeOnly = true;
	conditions.unpaid = true;
	conditions.card_id = customer.id();
	_quasar->db()->select(gltxs, conditions);
	if (gltxs.size() == 0) continue;

	// Generate report
	int col;
	Grid* grid = new Grid(6);
	grid->setShowPageNumber(false);
	grid->setShowCreateDate(false);
	for (col = 0; col < 6; ++col) {
	    grid->setColumnWeight(col, 1);
	    grid->setColumnSticky(col, (col < 3) ? Grid::Left : Grid::Right);
	}
	grid->setColumnWeight(2, 10);

	// Top section for company, date, customer, ...
	Grid* top = new Grid(3, grid);
	top->setColumnWeight(1, 1);
	grid->set(0, 0, 1, 6, top, Grid::AllSides);

	// Top section for company name and address
	Font::defaultFont = Font("Helvetica", 10);
	Grid* companyGrid = new Grid(1, top);
	QString companyAddr = company.address(true);
	companyGrid->set(USE_NEXT, 0, addressFrame(companyGrid, companyAddr,
						   true), Grid::Left);
	top->set(0, 0, companyGrid, Grid::TopLeft);

	// Section for title, page, date
	Grid* title = new Grid(2, top);
	Font::defaultFont = Font("Times", 16, true);
	title->set(0, 0, 1, 2, tr("Customer Statement"), Grid::Left);
	Font::defaultFont = Font("Times", 12);
	title->set(1, 0, tr("Date:"), Grid::Left);
	title->set(1, 1, DateValcon().format(today),Grid::Right);
	title->set(2, 0, tr("Page:"), Grid::Left);
	title->set(2, 1, new PageNumber(title), Grid::Right);
	title->setRowWeight(3, 1);
	top->set(0, 2, title, Grid::TopLeft);

	// Section for customer name and address
	Font::defaultFont = Font("Helvetica", 10);
	Grid* cust = new Grid(1, top);
	cust->set(0, 0, addressFrame(cust, customer.address()));
	top->set(2, 0, cust, Grid::TopLeft);

	// Section for entering remittance amount
	Grid* remit = new Grid(2, top);
	remit->set(0, 0, tr("  Amount Remitted"));
	remit->set(1, 0, " ");
	remit->set(2, 0, tr("$ ______________"));
	top->set(2, 2, remit, Grid::TopRight);

	// Headers
	Font::defaultFont = Font("Times", 12, true);
	top->set(1, 0, " ");
	grid->set(1, 0, " ");
	grid->set(2, 0, tr("Date"));
	grid->set(2, 1, tr("Reference #"));
	grid->set(2, 2, tr("Description"));
	grid->set(2, 3, tr("Total"));
	grid->set(2, 4, tr("Paid"));
	grid->set(2, 5, tr("Due"));
	for (col = 0; col < 5; ++col)
	    grid->setBorder(2, col, Grid::TopLeft | Grid::Bottom);
	grid->setBorder(2, 5, Grid::AllSides);

	// Add transactions
	Font::defaultFont = Font("Helvetica", 9);
	fixed current, pd30, pd60, pd90, pd120, totalDue;
	for (unsigned int j = 0; j < gltxs.size(); ++j) {
	    const Gltx& gltx = gltxs[j];
	    QString memo = gltx.memo();
	    fixed total = gltx.cardTotal();
	    fixed paid = gltx.paymentTotal();
	    QDate date = gltx.postDate();

	    if (gltx.dataType() == DataObject::INVOICE) {
		memo = tr("Invoice");
	    } else if (gltx.dataType() == DataObject::RETURN) {
		memo = tr("Return");
	    } else if (gltx.dataType() == DataObject::RECEIPT) {
		memo = tr("Payment");
	    } else if (gltx.dataType() == DataObject::WITHDRAW) {
		memo = tr("Withdraw");
	    }

	    grid->set(USE_NEXT, 0, DateValcon().format(gltx.postDate()));
	    grid->set(USE_CURR, 1, gltx.number());
	    grid->set(USE_CURR, 2, memo);
	    grid->set(USE_CURR, 3, MoneyValcon().format(total));
	    grid->set(USE_CURR, 4, MoneyValcon().format(paid));
	    grid->set(USE_CURR, 5, MoneyValcon().format(total - paid));

	    for (col = 0; col < 5; ++col)
		grid->setBorder(USE_CURR, col, Grid::Left);
	    grid->setBorder(USE_CURR, 5, Grid::LeftRight);

	    totalDue += total - paid;
	    if (today - date < 30)
		current += total - paid;
	    else if (today - date < 60)
		pd30 += total - paid;
	    else if (today - date < 90)
		pd60 += total - paid;
	    else if (today - date < 120)
		pd90 += total - paid;
	    else
		pd120 += total - paid;
	}
	grid->set(USE_NEXT, 0, " ");
	for (col = 0; col < 5; ++col)
	    grid->setBorder(USE_CURR, col, Grid::Left);
	grid->setBorder(USE_CURR, 5, Grid::LeftRight);
	grid->setRowWeight(USE_CURR, 1);

	// Totals
	Font::defaultFont = Font("Times", 12);
	Grid* totals = new Grid(6, grid);
	grid->set(USE_NEXT, 0, 1, 6, totals, Grid::LeftRight);
	for (col = 0; col < 6; ++col) {
	    totals->setColumnWeight(col, 1);
	    totals->setBorder(0, col, Grid::TopLeft | Grid::Bottom);
	    totals->setBorder(1, col, Grid::BottomLeft);
	}
	totals->setBorder(0, 5, Grid::AllSides);
	totals->setBorder(1, 5, Grid::BottomLeft | Grid::Right);
	Font::defaultFont = Font("Times", 12, true);
	totals->set(0, 0, tr("Current"));
	totals->set(0, 1, tr("30 days"));
	totals->set(0, 2, tr("60 days"));
	totals->set(0, 3, tr("90 days"));
	totals->set(0, 4, tr("120+ days"));
	totals->set(0, 5, tr("Total Due"));

	Font::defaultFont = Font("Times", 16, true);
	if (current != 0.0)
	    totals->set(1, 0, MoneyValcon().format(current));
	if (pd30 != 0.0)
	    totals->set(1, 1, MoneyValcon().format(pd30));
	if (pd60 != 0.0)
	    totals->set(1, 2, MoneyValcon().format(pd60));
	if (pd90 != 0.0)
	    totals->set(1, 3, MoneyValcon().format(pd90));
	if (pd120 != 0.0)
	    totals->set(1, 4, MoneyValcon().format(pd120));
	totals->set(1, 5, MoneyValcon().format(totalDue));

	grid->setHeaderRows(3);
	grid->setFooterRows(1);
	grids.push_back(grid);
    }

    Grid::print(grids, this);
    close();
}

void
PrintStmt::slotCancel()
{
    close();
}
