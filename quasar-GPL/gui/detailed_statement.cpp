// $Id: detailed_statement.cpp,v 1.16 2005/01/30 04:25:31 bpepers Exp $
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

#include "detailed_statement.h"
#include "main_window.h"
#include "screen_decl.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "customer.h"
#include "store.h"
#include "item.h"
#include "tax.h"
#include "tender.h"
#include "gltx.h"
#include "gltx_select.h"
#include "invoice.h"
#include "lookup_edit.h"
#include "customer_lookup.h"
#include "store_lookup.h"
#include "list_view_item.h"
#include "date_popup.h"
#include "money_edit.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <algorithm>

DetailedStatement::DetailedStatement(MainWindow* main)
    : QuasarWindow(main, "DetailedStatement")
{
    _helpSource = "detailed_statement.html";

    QFrame* frame = new QFrame(this);

    QLabel* customerLabel = new QLabel(tr("Customer:"), frame);
    _customer = new LookupEdit(new CustomerLookup(main, this), frame);
    _customer->setLength(30);
    customerLabel->setBuddy(_customer);

    QLabel* storeLabel = new QLabel(tr("Store:"), frame);
    _store = new LookupEdit(new StoreLookup(main, this), frame);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QLabel* fromLabel = new QLabel(tr("From:"), frame);
    _from = new DatePopup(frame);
    fromLabel->setBuddy(_from);

    QLabel* toLabel = new QLabel(tr("To:"), frame);
    _to = new DatePopup(frame);
    toLabel->setBuddy(_to);

    _lines = new ListView(frame);
    _lines->addDateColumn(tr("Date"));
    _lines->addTextColumn(tr("Description"), 30);
    _lines->addMoneyColumn(tr("Charges"));
    _lines->addMoneyColumn(tr("Credits"));
    _lines->setAllColumnsShowFocus(true);
    _lines->setShowSortIndicator(true);
    _lines->setSorting(-1);
    connect(_lines, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotPickLine()));

    QFrame* total = new QFrame(frame);

    _taxes = new ListView(total);
    _taxes->addTextColumn(tr("Name"), 20);
    _taxes->addMoneyColumn(tr("Total"));
    _taxes->setAllColumnsShowFocus(true);
    _taxes->setShowSortIndicator(true);

    QLabel* chargeLabel = new QLabel(tr("Total Charges:"), total);
    QLabel* creditLabel = new QLabel(tr("Total Credits:"), total);
    QLabel* dueLabel = new QLabel(tr("Balance Due:"), total);

    _chargeTotal = new MoneyEdit(total);
    _chargeTotal->setFocusPolicy(NoFocus);
    _chargeTotal->setLength(12, '9');

    _creditTotal = new MoneyEdit(total);
    _creditTotal->setFocusPolicy(NoFocus);
    _creditTotal->setLength(12, '9');

    //QLabel* spacer = new QLabel(" ", total);

    _dueTotal = new MoneyEdit(total);
    _dueTotal->setFocusPolicy(NoFocus);
    _dueTotal->setLength(12, '9');

    QGridLayout* totalGrid = new QGridLayout(total);
    totalGrid->setSpacing(3);
    totalGrid->setMargin(3);
    totalGrid->setColStretch(1, 1);
    totalGrid->addMultiCellWidget(_taxes, 0, 2, 0, 0);
    totalGrid->addWidget(chargeLabel, 0, 2);
    totalGrid->addWidget(_chargeTotal, 0, 3);
    totalGrid->addWidget(creditLabel, 1, 2);
    totalGrid->addWidget(_creditTotal, 1, 3);
    //totalGrid->addWidget(spacer, 2, 2);
    totalGrid->addWidget(dueLabel, 2, 2);
    totalGrid->addWidget(_dueTotal, 2, 3);

    QFrame* buttons = new QFrame(frame);
    QPushButton* refresh = new QPushButton(tr("&Refresh"), buttons);
    connect(refresh, SIGNAL(clicked()), SLOT(slotRefresh()));
    refresh->setMinimumSize(refresh->sizeHint());

    QPushButton* print = new QPushButton(tr("&Print"), buttons);
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));
    print->setMinimumSize(refresh->sizeHint());

    QPushButton* ok = new QPushButton(tr("&Close"), buttons);
    connect(ok, SIGNAL(clicked()), SLOT(close()));
    ok->setMinimumSize(refresh->sizeHint());

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(2, 1);
    buttonGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(print, 0, 1, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(ok, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(2, 1);
    grid->setRowStretch(2, 1);
    grid->addWidget(customerLabel, 0, 0);
    grid->addWidget(_customer, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(storeLabel, 1, 0);
    grid->addWidget(_store, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(fromLabel, 0, 3);
    grid->addWidget(_from, 0, 4, AlignLeft | AlignVCenter);
    grid->addWidget(toLabel, 1, 3);
    grid->addWidget(_to, 1, 4, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_lines, 2, 2, 0, 4);
    grid->addMultiCellWidget(total, 3, 3, 0, 4);
    grid->addMultiCellWidget(buttons, 4, 4, 0, 4);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    _customer->setFocus();
    _store->setId(_quasar->defaultStore());
    _from->setDate(QDate::currentDate());
    _to->setDate(QDate::currentDate());

    setCentralWidget(frame);
    setCaption(tr("Detailed Statement"));
    finalize();
}

DetailedStatement::~DetailedStatement()
{
}

void
DetailedStatement::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
DetailedStatement::setCustomerId(Id customer_id)
{
    _customer->setId(customer_id);
    slotRefresh();
}

void
DetailedStatement::slotPickLine()
{
    ListViewItem* item = (ListViewItem*)_lines->currentItem();
    if (item == NULL) return;

    Gltx gltx;
    Id gltx_id = item->id;
    if (!_quasar->db()->lookup(gltx_id, gltx)) return;

    QWidget* edit = editGltx(gltx, _main);
    if (edit != NULL) edit->show();
}

void
DetailedStatement::slotPrint()
{
    QDate from = _from->getDate();
    QDate to = _to->getDate();

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_lines, tr("Detailed Statement"));
    Grid* header = new Grid(1, grid);
    grid->set(0, 0, 1, grid->columns(), header, Grid::AllSides);

    header->setColumnWeight(0, 1);
    TextFrame* text = new TextFrame(tr("Detailed Statement"), header);
    text->setFont(Font("Times", 24));
    header->set(USE_NEXT, 0, text);
    if (_customer->getId() != INVALID_ID) {
	Card card;
	_quasar->db()->lookup(_customer->getId(), card);
	text = new TextFrame(card.nameFL(), header);
	text->setFont(Font("Times", 14));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }
    if (_store->getId() != INVALID_ID) {
	Store store;
	_quasar->db()->lookup(_store->getId(), store);
	text = new TextFrame(store.name(), header);
	text->setFont(Font("Times", 10));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }
    if (!from.isNull() || !to.isNull()) {
	QString range;
	DateValcon valcon;

	if (!from.isNull() && !to.isNull())
	    range = valcon.format(from) + tr("  to  ") + valcon.format(to);
	else if (!from.isNull())
	    range = tr("On/After  ") + valcon.format(from);
	else
	    range = tr("On/Before  ") + valcon.format(to);

	text = new TextFrame(range, header);
	text->setFont(Font("Times", 10));
	header->set(USE_NEXT, 0, " ");
	header->set(USE_NEXT, 0, text);
    }

    grid->set(grid->rows(), 4, _chargeTotal->text(), Grid::Right);
    grid->set(USE_CURR, 6, _creditTotal->text(), Grid::Right);
    grid->setBorder(USE_CURR, 4, Grid::Top);
    grid->setBorder(USE_CURR, 6, Grid::Top);

    ListViewItem* item = _taxes->firstChild();
    if (item != NULL) {
	grid->set(USE_NEXT, 0, " ");
	while (item != NULL) {
	    grid->set(USE_NEXT, 2, item->text(0) + tr(" Total"));
	    grid->set(USE_CURR, 6, item->text(1), Grid::Right);
	    item = item->nextSibling();
	}
    }
    grid->set(USE_NEXT, 0, " ");

    TextFrame* due = new TextFrame(tr("Balance Due"), grid);
    due->setFont(Font("Times", 18));
    grid->set(USE_NEXT, 2, due, Grid::Left);

    TextFrame* amt = new TextFrame(_dueTotal->text(), grid);
    amt->setFont(Font("Times", 18));
    grid->set(USE_CURR, 6, amt, Grid::Right);

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
DetailedStatement::slotRefresh()
{
    _lines->clear();
    _taxes->clear();

    if (_customer->getId() == INVALID_ID || _store->getId() == INVALID_ID) {
	_chargeTotal->setText("");
	_creditTotal->setText("");
	_dueTotal->setText("");
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    fixed chargeTotal = 0.0;
    fixed creditTotal = 0.0;
    vector<Id> tax_ids;
    vector<fixed> tax_amts;

    // Load transactions
    GltxSelect conditions;
    conditions.card_id = _customer->getId();
    conditions.store_id = _store->getId();
    conditions.start_date = _from->getDate();
    conditions.end_date = _to->getDate();
    conditions.activeOnly = true;
    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    // Sort by date
    std::sort(gltxs.begin(), gltxs.end());

    // Add transactions
    ListViewItem* last = NULL;
    unsigned int i;
    for (i = 0; i < gltxs.size(); ++i) {
	const Gltx& gltx = gltxs[i];
	int type = gltx.dataType();
	fixed cardTotal = gltx.cardTotal();
	if (cardTotal == 0.0) continue;

	if (type == DataObject::INVOICE || type == DataObject::RETURN) {
	    Invoice invoice;
	    _quasar->db()->lookup(gltx.id(), invoice);

	    fixed sign = 1.0;
	    if (type == DataObject::RETURN)
		sign = -1.0;

	    unsigned int j;
	    for (j = 0; j < invoice.items().size(); ++j) {
		const InvoiceItem& info = invoice.items()[j];
		if (info.voided) continue;

		QString description = info.description;
		if (description.isEmpty()) {
		    Item item;
		    _quasar->db()->lookup(info.item_id, item);
		    description = item.description();
		}

		fixed amount = info.sale_price;
		if (info.line_disc.account_id != INVALID_ID)
		    amount -= info.line_disc.total_amt;
		for (unsigned int k = 0; k < info.tdisc_nums.size(); ++k) {
		    int num = info.tdisc_nums[k];
		    InvoiceDisc& disc = invoice.discounts()[num];
		    if (disc.account_id == INVALID_ID) continue;
		    amount -= info.tdisc_amts[k];
		}

		ListViewItem* item = new ListViewItem(_lines, last, gltx.id());
		last = item;
		item->setValue(0, invoice.postDate());
		item->setValue(1, description);
		if (amount >= 0.0) {
		    item->setValue(2, amount);
		    chargeTotal += amount;
		} else {
		    item->setValue(3, -amount);
		    creditTotal += -amount;
		}

		if (info.ext_deposit != 0.0) {
		    fixed amount;
		    
		    if (type == DataObject::RETURN)
		        amount = -info.ext_deposit;
		    else
		        amount = info.ext_deposit;
		    ListViewItem* item = new ListViewItem(_lines, last,
				    gltx.id());
		    last = item;
		    item->setValue(0, invoice.postDate());
		    item->setValue(1, tr("Bottle Deposit"));
		    if (amount >= 0.0) {
		        item->setValue(2, amount);
		        chargeTotal += amount;
		    } else {
		        item->setValue(3, -amount);
		        creditTotal += -amount;
		    }
		}
	    }

	    for (j = 0; j < invoice.taxes().size(); ++j) {
		const TaxLine& info = invoice.taxes()[j];
		fixed amount = (info.amount + info.inc_amount) * sign;

		Tax tax;
		_quasar->db()->lookup(info.tax_id, tax);

		ListViewItem* item = new ListViewItem(_lines, last, gltx.id());
		last = item;
		item->setValue(0, invoice.postDate());
		item->setValue(1, tax.name());
		if (amount >= 0.0) {
		    item->setValue(2, amount);
		    chargeTotal += amount;
		} else {
		    item->setValue(3, -amount);
		    creditTotal += -amount;
		}

		bool found = false;
		for (unsigned int k = 0; k < tax_ids.size(); ++k) {
		    if (tax_ids[k] == info.tax_id) {
			tax_amts[k] += amount;
			found = true;
			break;
		    }
		}

		if (!found) {
		    tax_ids.push_back(info.tax_id);
		    tax_amts.push_back(amount);
		}
	    }

	    for (j = 0; j < invoice.tenders().size(); ++j) {
		const TenderLine& info = invoice.tenders()[j];
		if (info.voided) continue;

		Tender tender;
		_quasar->db()->lookup(info.tender_id, tender);
		fixed amount = info.amount;

		ListViewItem* item = new ListViewItem(_lines, last, gltx.id());
		last = item;
		item->setValue(0, invoice.postDate());
		item->setValue(1, tender.name() + tr(" payment"));
		if (amount >= 0.0) {
		    item->setValue(3, amount);
		    creditTotal += amount;
		} else {
		    item->setValue(2, -amount);
		    chargeTotal += -amount;
		}
	    }
	} else if (type == DataObject::RECEIPT) {
	    for (unsigned int i = 0; i < gltx.tenders().size(); ++i) {
		const TenderLine& info = gltx.tenders()[i];
		if (info.voided) continue;

		Tender tender;
		_quasar->db()->lookup(info.tender_id, tender);

		ListViewItem* item = new ListViewItem(_lines, last, gltx.id());
		last = item;
		item->setValue(0, gltx.postDate());
		item->setValue(1, tender.name() + tr(" payment"));
		if (info.amount >= 0.0) {
		    item->setValue(3, info.amount);
		    creditTotal += info.amount;
		} else {
		    item->setValue(2, -info.amount);
		    chargeTotal += -info.amount;
		}
	    }
	} else {
	    QString desc = gltx.memo();
	    if (desc.isEmpty())
		desc = gltx.dataTypeName();

	    ListViewItem* item = new ListViewItem(_lines, last, gltx.id());
	    last = item;
	    item->setValue(0, gltx.postDate());
	    item->setValue(1, desc);
	    if (cardTotal >= 0.0) {
		item->setValue(2, cardTotal);
		chargeTotal += cardTotal;
	    } else {
		item->setValue(3, -cardTotal);
		creditTotal += -cardTotal;
	    }
	}
    }

    // Set taxes
    for (i = 0; i < tax_ids.size(); ++i) {
	Id tax_id = tax_ids[i];
	fixed tax_amt = tax_amts[i];

	Tax tax;
	_quasar->db()->lookup(tax_id, tax);

	ListViewItem* item = new ListViewItem(_taxes);
	item->setValue(0, tax.name());
	item->setValue(1, tax_amt);
    }

    // Set totals
    _chargeTotal->setFixed(chargeTotal);
    _creditTotal->setFixed(creditTotal);
    _dueTotal->setFixed(chargeTotal - creditTotal);

    QApplication::restoreOverrideCursor();
}
