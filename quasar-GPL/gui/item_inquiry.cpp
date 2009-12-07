// $Id: item_inquiry.cpp,v 1.28 2005/01/30 04:25:31 bpepers Exp $
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

#include "item_inquiry.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "item.h"
#include "gltx.h"
#include "store.h"
#include "lookup_edit.h"
#include "item_lookup.h"
#include "list_view_item.h"
#include "date_valcon.h"
#include "money_valcon.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qlistview.h>
#include <qlabel.h>
#include <qprogressdialog.h>

ItemInquiry::ItemInquiry(MainWindow* main, QWidget* parent)
    : InquiryWindow(main, parent, "ItemInquiry", new ItemLookup(main, parent))
{
    search->setLength(18, '9');
    list->setColumnText(5, tr("Increase"));
    list->setColumnText(6, tr("Decrease"));

    // TODO: add description and size fields
}

ItemInquiry::~ItemInquiry()
{
}

void
ItemInquiry::slotRefresh(QDate from, QDate to, Id store_id)
{
    if (loading) return;
    loading = true;
    needsRefresh = false;
    delete _grid;
    _grid = NULL;

    // Clear data
    list->clear();
    for (int label = 0; label < 10; ++label)
	labels[label]->setText(" ");

    // Get item to show and return if none
    Id item_id = search->getId();
    if (item_id == INVALID_ID) {
	loading = false;
	return;
    }

    setEnabled(false);
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Get item information
    Item item;
    quasar->db()->lookup(item_id, item);

    fixed beginOnHand = 0.0;
    fixed beginCost = 0.0;
    fixed beginOnOrder = 0.0;
    fixed totalDebits = 0.0;
    fixed totalCredits = 0.0;

    labels[0]->setText(tr("Beginning Balance"));
    labels[1]->setText(tr("Total Increase"));
    labels[2]->setText(tr("Total Decrease"));
    labels[3]->setText(tr("Net Change"));
    labels[4]->setText(tr("Ending Balance"));

    // Get beginning balance
    if (!from.isNull())
	quasar->db()->itemGeneral(item.id(), "", store_id, from - 1,
				  beginOnHand, beginCost, beginOnOrder);

    // Select transactions
    vector<Gltx> gltxs;
    vector<fixed> quantities;
    vector<fixed> ext_costs;
    vector<fixed> ext_prices;
    vector<bool> void_flags;
    quasar->db()->selectItem(item.id(), store_id, from, to, gltxs,
			     quantities, ext_costs, ext_prices, void_flags);

    // Setup grid
    _grid = new Grid(8);
    Grid* header = new Grid(1, _grid);
    TextFrame* text;
    text = new TextFrame(tr("Item Inquiry"), header);
    text->setFont(Font("Times", 24));
    header->set(USE_NEXT, 0, text);
    text = new TextFrame(item.number() + " " + item.description(), header);
    text->setFont(Font("Times", 18));
    header->set(USE_NEXT, 0, text);
    text = new TextFrame(DateValcon().format(from) + tr(" to ") +
			 DateValcon().format(to), header);
    text->setFont(Font("Times", 18));
    header->set(USE_NEXT, 0, text);
    header->setColumnWeight(0, 1);

    _grid->set(0, 0, 1, _grid->columns(), header, Grid::AllSides);
    _grid->set(USE_NEXT, 0, "");
    for (int column = 0; column < 8; ++column) {
	_grid->setColumnSticky(column, column < 4 ? Grid::Left : Grid::Right);
	_grid->setColumnPad(column, 5);
    }
    _grid->set(2, 0, tr("Type"));
    _grid->set(3, 0, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 1, tr("Id #"));
    _grid->set(3, 1, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 2, tr("Store"));
    _grid->set(3, 2, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 3, tr("Date"));
    _grid->set(3, 3, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 4, tr("Memo"));
    _grid->set(3, 4, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 5, tr("Debit"));
    _grid->set(3, 5, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 6, tr("Credit"));
    _grid->set(3, 6, new LineFrame(_grid), Grid::LeftRight);
    _grid->set(2, 7, tr("Voided?"));
    _grid->set(3, 7, new LineFrame(_grid), Grid::LeftRight);
    _grid->setHeaderRows(_grid->rows());

    // Setup progress dialog
    QProgressDialog* progress = new QProgressDialog(tr("Loading data..."),
						    tr("Cancel"), gltxs.size(),
						    this, "Progress", true);
    progress->setMinimumDuration(1000);
    progress->setCaption(tr("Progress"));

    // Process each transaction
    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	const Gltx& gltx = gltxs[i];
	fixed ext_cost = ext_costs[i];
	// TODO: pick to show cost, qty, or price

	if (i % 50 == 0) {
	    progress->setProgress(i);
	    qApp->processEvents();
	    if (progress->wasCancelled()) {
		break;
	    }
	}

	// Don't show voided lines or quotes
	if (void_flags[i] || gltx.dataType() == DataObject::QUOTE)
	    continue;

	Store store;
	quasar->db()->lookup(gltx.storeId(), store);

	ListViewItem* item = new ListViewItem(list, gltx.id());
	item->setValue(0, gltx.dataTypeName());
	item->setValue(1, gltx.number());
	item->setValue(2, store.number());
	item->setValue(3, gltx.postDate());
	item->setValue(4, gltx.memo());
	item->setValue(7, !gltx.isActive());

	_grid->set(USE_NEXT, 0, gltx.dataTypeName());
	_grid->set(USE_CURR, 1, gltx.number());
	_grid->set(USE_CURR, 2, store.number());
	_grid->set(USE_CURR, 3, DateValcon().format(gltx.postDate()));
	_grid->set(USE_CURR, 4, gltx.memo());
	_grid->set(USE_CURR, 7, gltx.isActive() ? " " : "x");

	// Correct sign based on type
	switch (gltx.dataType()) {
	case DataObject::INVOICE:
	case DataObject::RETURN:
	    ext_cost = -ext_cost;
	    break;
	default:
	    break;
	}

	if (ext_cost >= 0.0) {
	    item->setValue(5, ext_cost);
	    _grid->set(USE_CURR, 5, MoneyValcon().format(ext_cost));
	    if (gltx.isActive()) totalDebits += ext_cost;
	} else {
	    item->setValue(6, -ext_cost);
	    _grid->set(USE_CURR, 6, MoneyValcon().format(-ext_cost));
	    if (gltx.isActive()) totalCredits += -ext_cost;
	}
    }
    progress->setProgress(gltxs.size());
    delete progress;

    // Set bottom totals
    fixed netChange = totalDebits - totalCredits;
    fixed endBalance = beginCost + netChange;

    MoneyValcon moneyValcon;
    labels[5]->setText(moneyValcon.format(beginCost));
    labels[6]->setText(moneyValcon.format(totalDebits));
    labels[7]->setText(moneyValcon.format(totalCredits));
    labels[8]->setText(moneyValcon.format(netChange));
    labels[9]->setText(moneyValcon.format(endBalance));

    _grid->set(USE_NEXT, 0, "");
    _grid->set(USE_NEXT, 0, 1, 5, tr("Beginning Balance"), Grid::Right);
    _grid->set(USE_CURR, 5, 1, 2, MoneyValcon().format(beginCost));
    _grid->set(USE_NEXT, 0, 1, 5, tr("Total Debits"), Grid::Right);
    _grid->set(USE_CURR, 5, 1, 2, MoneyValcon().format(totalDebits));
    _grid->set(USE_NEXT, 0, 1, 5, tr("Total Credits"), Grid::Right);
    _grid->set(USE_CURR, 5, 1, 2, MoneyValcon().format(totalCredits));
    _grid->set(USE_NEXT, 0, 1, 5, tr("Net Change"), Grid::Right);
    _grid->set(USE_CURR, 5, 1, 2, MoneyValcon().format(netChange));
    _grid->set(USE_NEXT, 0, 1, 5, tr("Ending Balance"), Grid::Right);
    _grid->set(USE_CURR, 5, 1, 2, MoneyValcon().format(endBalance));

    QApplication::restoreOverrideCursor();
    setEnabled(true);
    loading = false;
}
