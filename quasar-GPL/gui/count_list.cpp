// $Id: count_list.cpp,v 1.40 2005/03/13 22:30:50 bpepers Exp $
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

#include "count_list.h"
#include "main_window.h"
#include "preview_grid.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "count_select.h"
#include "count_master.h"
#include "item_edit.h"
#include "store_lookup.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "group_lookup.h"
#include "location_lookup.h"
#include "account_lookup.h"
#include "item_lookup.h"
#include "item_adjust.h"
#include "account.h"
#include "company.h"
#include "store.h"
#include "dept.h"
#include "subdept.h"
#include "group.h"
#include "location.h"
#include "item.h"
#include "item_select.h"
#include "store_select.h"
#include "grid.h"
#include "text_frame.h"
#include "money_valcon.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qtabwidget.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qfile.h>

CountList::CountList(MainWindow* main)
    : ActiveList(main, "CountList")
{
    _helpSource = "count_list.html";

    QPopupMenu* process = new QPopupMenu(this);
    process->insertItem(tr("Search For Item"), this, SLOT(slotItemSearch()));
    process->insertItem("Items Not Counted", this, SLOT(slotNotCounted()));
    process->insertItem(tr("Discrepancy Report"), this,SLOT(slotDiscReport()));
    process->insertItem(tr("Process Counts"), this, SLOT(slotProcess()));
    menuBar()->insertItem(tr("&Process"), process, -1, 2);

    _list->addDateColumn(tr("Date"));
    _list->addTextColumn(tr("Number"), 14);
    _list->addTextColumn(tr("Description"), 30);
    _list->setSorting(0);

    QLabel* storeLabel = new QLabel(tr("&Store:"), _search);
    _store = new LookupEdit(new StoreLookup(_main, this), _search);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QGridLayout* grid = new QGridLayout(_search);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addWidget(storeLabel, 0, 0);
    grid->addWidget(_store, 0, 1, AlignLeft | AlignVCenter);

    _store->setId(_quasar->defaultStore());
    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    connect(_quasar->db(), SIGNAL(dataEvent(DataEvent*)),
	    this, SLOT(dataEvent(DataEvent*)));

    setCaption(tr("Count List"));
    finalize();
    _store->setFocus();
}

CountList::~CountList()
{
}

void
CountList::slotItemSearch()
{
    Id store_id = _store->getId();

    CountSelect conditions;
    conditions.activeOnly = true;
    conditions.store_id = store_id;
    vector<Count> counts;
    _quasar->db()->select(counts, conditions);

    if (counts.size() == 0) {
	QMessageBox::critical(this, tr("Error"), tr("No counts exist"));
	return;
    }

    QDialog* dialog = new QDialog(this, "Search", true);
    dialog->setCaption(tr("Item Search"));

    QString msg = tr("Select the item you wish to find.");
    QLabel* msgLabel = new QLabel(msg, dialog);

    QLabel* itemLabel = new QLabel(tr("Item Number:"), dialog);
    LineEdit* itemNumber = new LineEdit(18, dialog);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    ok->connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    cancel->connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    ok->setDefault(true);

    QGridLayout* grid1 = new QGridLayout(buttons);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(0, 1);
    grid1->addWidget(ok, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
    grid1->addWidget(cancel, 0, 1, Qt::AlignRight | Qt::AlignVCenter);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addMultiCellWidget(msgLabel, 0, 0, 0, 1);
    grid->addWidget(itemLabel, 1, 0);
    grid->addWidget(itemNumber, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    itemNumber->setFocus();
    if (dialog->exec() != QDialog::Accepted) {
	delete dialog;
	return;
    }

    QString number = itemNumber->text();
    delete dialog;
    if (number.isEmpty()) return;

    QString message;
    for (unsigned int i = 0; i < counts.size(); ++i) {
	const Count& count = counts[i];

	for (unsigned int j = 0; j < count.items().size(); ++j) {
	    const CountItem& line = count.items()[j];
	    if (line.number != number) continue;
	    fixed qty = line.quantity;

	    if (message.isEmpty())
		message = tr("Item %1 found in:\n\n").arg(number);

	    message += tr("   Count %1 (line %2, counted %3)\n")
		.arg(count.number()).arg(j + 1).arg(qty.toString());;
	}
    }

    if (message.isEmpty())
	message = tr("Item %1 wasn't found in any counts").arg(number);

    QMessageBox::information(this, tr("Item Search"), message);
}

void
CountList::slotNotCounted()
{
    if (_store->getId() == INVALID_ID) {
        QMessageBox::critical(this, tr("Error"), tr("No store selected"));
        return;
    }

    QDialog* dialog = new QDialog(this, "ItemSelection", true);
    dialog->setCaption(tr("Item Selection"));

    QString msg = tr("Limit the items by:");
    QLabel* msgLabel = new QLabel(msg, dialog);

    QLabel* deptLabel = new QLabel(tr("Department:"), dialog);
    DeptLookup* deptLookup = new DeptLookup(_main, this);
    LookupEdit* dept = new LookupEdit(deptLookup, dialog);
    dept->setLength(30);
    deptLabel->setBuddy(dept);

    QLabel* subdeptLabel = new QLabel(tr("Subdepartment:"), dialog);
    SubdeptLookup* subdeptLookup = new SubdeptLookup(_main, this);
    LookupEdit* subdept = new LookupEdit(subdeptLookup, dialog);
    subdept->setLength(30);
    subdeptLabel->setBuddy(subdept);

    QLabel* groupLabel = new QLabel(tr("Group:"), dialog);
    GroupLookup* groupLookup = new GroupLookup(_main, this, Group::ITEM);
    LookupEdit* group = new LookupEdit(groupLookup, dialog);
    group->setLength(30);
    groupLabel->setBuddy(group);

    QLabel* locationLabel = new QLabel(tr("Location:"), dialog);
    LocationLookup* locLookup = new LocationLookup(_main, this);
    LookupEdit* location = new LookupEdit(locLookup, dialog);
    location->setLength(30);
    locationLabel->setBuddy(location);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    ok->connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    cancel->connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    ok->setDefault(true);

    QGridLayout* grid1 = new QGridLayout(buttons);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(0, 1);
    grid1->addWidget(ok, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
    grid1->addWidget(cancel, 0, 1, Qt::AlignRight | Qt::AlignVCenter);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addMultiCellWidget(msgLabel, 0, 0, 0, 1);
    grid->addWidget(deptLabel, 1, 1);
    grid->addWidget(dept, 1, 2);
    grid->addWidget(groupLabel, 1, 3);
    grid->addWidget(group, 1, 4);
    grid->addWidget(subdeptLabel, 2, 1);
    grid->addWidget(subdept, 2, 2);
    grid->addWidget(locationLabel, 2, 3);
    grid->addWidget(location, 2, 4);
    grid->addMultiCellWidget(buttons, 4, 4, 3, 4);

    dept->setFocus();
    if (dialog->exec() != QDialog::Accepted) {
	delete dialog;
	return;
    }

    Id store_id = _store->getId();
    Id dept_id = dept->getId();
    Id subdept_id = subdept->getId();
    Id location_id = location->getId();
    Id group_id = group->getId();
    delete dialog;

    CountSelect conditions;
    conditions.store_id = store_id;
    conditions.activeOnly = true;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    vector<Count> counts;
    _quasar->db()->select(counts, conditions);

    if (counts.size() == 0) {
	QApplication::restoreOverrideCursor();
	QMessageBox::critical(this, tr("Error"), tr("No counts exist"));
	return;
    }

    ItemSelect itemConditions;
    itemConditions.store_id = store_id;
    itemConditions.dept_id = dept_id;
    itemConditions.subdept_id = subdept_id;
    itemConditions.location_id = location_id;
    itemConditions.group_id = group_id;
    itemConditions.activeOnly = true;

    QDate date = QDate::currentDate();
    vector<Id> item_ids;
    vector<fixed> on_hands;
    vector<fixed> total_costs;
    _quasar->db()->itemBalances(date, itemConditions, item_ids, on_hands,
				total_costs);

    vector<Item> items;
    vector<fixed> item_on_hands;
    unsigned int i;
    for (i = 0; i < item_ids.size(); ++i) {
	Id item_id = item_ids[i];
	fixed on_hand = on_hands[i];
	if (on_hand == 0.0) continue;

	bool found = false;
	for (unsigned int j = 0; j < counts.size(); ++j) {
	    const Count& count = counts[j];
	    for (unsigned int k = 0; k < count.items().size(); ++k) {
		if (count.items()[k].item_id == INVALID_ID) continue;
		if (count.items()[k].item_id == item_id) {
		    found = true;
		    break;
		}
	    }
	    if (found) break;
	}

	if (!found) {
	    Item item;
	    _quasar->db()->lookup(item_id, item);

	    items.push_back(item);
	    item_on_hands.push_back(on_hand);
	}
    }

    if (items.size() == 0) {
	QApplication::restoreOverrideCursor();
	QMessageBox::information(this, tr("Items Not Counted"), tr("All items counted"));
	return;
    }

    // Create grid and header
    Grid* output = new Grid(3);
    TextFrame* text = new TextFrame(tr("Items Not Counted"), output);
    text->setFont(Font("Times", 24));
    output->set(0, 0, 1, 3, text);
    output->set(USE_NEXT, 0, " ");

    // Set weights centering
    int col;
    for (col = 0; col < 3; ++col) {
	output->setColumnWeight(col, 1);
	if (col != 2)
	    output->setColumnSticky(col, Grid::Left);
	else
	    output->setColumnSticky(col, Grid::Right);
    }
    output->setColumnWeight(1, 10);

    // Column headers
    Font::defaultFont = Font("Times", 12, true);
    output->set(USE_NEXT, 0, tr("Item Number"));
    output->set(USE_CURR, 1, tr("Description"));
    output->set(USE_CURR, 2, tr("On Hand"));
    for (col = 0; col < 3; ++col)
	output->setBorder(USE_CURR, col, Grid::Bottom);
    output->setHeaderRows(output->rows());

    // Default font for data is Helvetica 10
    Font::defaultFont = Font("Helvetica", 10);

    // Process items not counted
    for (i = 0; i < items.size(); ++i) {
	const Item& item = items[i];
	fixed on_hand = item_on_hands[i];

	output->set(USE_NEXT, 0, item.number());
	output->set(USE_CURR, 1, item.description());
	output->set(USE_CURR, 2, on_hand.toString());
    }

    QApplication::restoreOverrideCursor();

    PreviewGrid* preview = new PreviewGrid(_main, output);
    preview->show();
}

void CountList::slotDiscReport()
{
    Id store_id = _store->getId();
    if (store_id == INVALID_ID) {
	QMessageBox::critical(this, tr("Error"), tr("No store selected"));
	return;
    }

    CountSelect conditions;
    conditions.activeOnly = true;
    conditions.store_id = store_id;
    vector<Count> counts;
    _quasar->db()->select(counts, conditions);

    if (counts.size() == 0) {
	QMessageBox::critical(this, tr("Error"), tr("No counts exist"));
	return;
    }

    QDialog* dialog = new QDialog(this, "DiscReport", true);
    dialog->setCaption(tr("Discrepancy Report"));

    QString msg = tr("Do you wish to include items with no\n"
	"discrepancy on the printout?");
    QLabel* msgLabel = new QLabel(msg, dialog);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* yes = new QPushButton(tr("Yes"), buttons);
    QPushButton* no = new QPushButton(tr("No"), buttons);

    connect(yes, SIGNAL(clicked()), dialog, SLOT(accept()));
    connect(no, SIGNAL(clicked()), dialog, SLOT(reject()));
    no->setDefault(true);

    QGridLayout* grid1 = new QGridLayout(buttons);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(0, 1);
    grid1->addWidget(yes, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
    grid1->addWidget(no, 0, 1, Qt::AlignRight | Qt::AlignVCenter);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addMultiCellWidget(msgLabel, 0, 0, 0, 1);
    grid->addMultiCellWidget(buttons, 1, 1, 0, 1);

    no->setFocus();
    bool showZero = (dialog->exec() == QDialog::Accepted);
    delete dialog;

    Store store;
    _quasar->db()->lookup(store_id, store);

    // Create grid and header
    Grid* output = new Grid(9);
    TextFrame* text = new TextFrame(tr("Discrepancy Report"), output);
    text->setFont(Font("Times", 24));
    output->set(0, 0, 1, 8, text);
    output->set(USE_NEXT, 0, " ");

    // Set weights centering
    int col;
    for (col = 0; col < 9; ++col) {
	output->setColumnWeight(col, 1);
	if (col == 1 || col == 2)
	    output->setColumnSticky(col, Grid::Left);
	else
	    output->setColumnSticky(col, Grid::Right);
    }
    output->setColumnWeight(1, 10);

    // Column headers
    Font::defaultFont = Font("Times", 12, true);
    output->set(USE_NEXT, 0, tr("Item Number"));
    output->set(USE_CURR, 1, tr("Description"));
    output->set(USE_CURR, 2, tr("Size"));
    output->set(USE_CURR, 3, tr("Size Qty"));
    output->set(USE_CURR, 4, tr("On Hand"));
    output->set(USE_CURR, 5, tr("Counted"));
    output->set(USE_CURR, 6, tr("Difference"));
    output->set(USE_CURR, 7, tr("Cost"));
    output->set(USE_CURR, 8, tr("Deposit"));
    for (col = 0; col < 9; ++col)
	output->setBorder(USE_CURR, col, Grid::Bottom);
    output->setHeaderRows(output->rows());

    // Default font for data is Helvetica 10
    Font::defaultFont = Font("Helvetica", 10);

    // Get discrepancies
    vector<Item> items;
    vector<QString> numbers;
    vector<QString> sizes;
    vector<fixed> on_hands;
    vector<fixed> qtys;
    vector<fixed> costs;
    vector<fixed> deposits;
    discrepancies(counts, showZero, store_id, items, numbers, sizes,
		  on_hands, qtys, costs, deposits);

    // Process discrepancies
    fixed total_cost = 0.0;
    fixed total_deposit = 0.0;
    for (unsigned int i = 0; i < items.size(); ++i) {
	const Item& item = items[i];
	QString number = numbers[i];
	QString size = sizes[i];
	fixed size_qty = item.sizeQty(size);
	fixed on_hand = on_hands[i];
	fixed cost = costs[i];
	fixed deposit = deposits[i];
	fixed diff = qtys[i];
	fixed counted = on_hand + diff;

	output->set(USE_NEXT, 0, number);
	output->set(USE_CURR, 1, item.description());
	output->set(USE_CURR, 2, size);
	output->set(USE_CURR, 3, size_qty.toString());
	output->set(USE_CURR, 4, on_hand.toString());
	output->set(USE_CURR, 5, counted.toString());
	output->set(USE_CURR, 6, diff.toString());
	output->set(USE_CURR, 7, MoneyValcon().format(cost));
	output->set(USE_CURR, 8, MoneyValcon().format(deposit));

	total_cost += cost;
	total_deposit += deposit;
    }

    // Total font is larger and bold
    Font::defaultFont = Font("Helvetica", 14, true);
    output->set(USE_NEXT, 0, " ");
    output->set(USE_NEXT, 1, tr("Total"));
    output->set(USE_CURR, 7, MoneyValcon().format(total_cost));
    output->set(USE_CURR, 8, MoneyValcon().format(total_deposit));

    PreviewGrid* preview = new PreviewGrid(_main, output);
    preview->show();
}

void
CountList::slotProcess()
{
    Id store_id = _store->getId();
    if (store_id == INVALID_ID) {
	QMessageBox::critical(this, tr("Error"), tr("No store selected"));
	return;
    }

    CountSelect conditions;
    conditions.activeOnly = true;
    conditions.store_id = store_id;
    vector<Count> counts;
    _quasar->db()->select(counts, conditions);

    if (counts.size() == 0) {
	QMessageBox::critical(this, tr("Error"), tr("No counts exist"));
	return;
    }

    Company company;
    _quasar->db()->lookup(company);

    QDialog* dialog = new QDialog(this, "Process", true);
    dialog->setCaption(tr("Process Counts"));

    QString msg = tr("Select the account you wish to post the "
	"discrepancy to.");
    QLabel* msgLabel = new QLabel(msg, dialog);

    QLabel* accountLabel = new QLabel(tr("Account:"), dialog);
    AccountLookup* lookup = new AccountLookup(_main, this, Account::Expense);
    LookupEdit* accountEdit = new LookupEdit(lookup, dialog);
    accountEdit->setLength(30);

    QFrame* buttons = new QFrame(dialog);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    ok->connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    cancel->connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
    ok->setDefault(true);

    QGridLayout* grid1 = new QGridLayout(buttons);
    grid1->setSpacing(6);
    grid1->setMargin(6);
    grid1->setColStretch(0, 1);
    grid1->addWidget(ok, 0, 0, Qt::AlignRight | Qt::AlignVCenter);
    grid1->addWidget(cancel, 0, 1, Qt::AlignRight | Qt::AlignVCenter);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(1, 1);
    grid->addMultiCellWidget(msgLabel, 0, 0, 0, 1);
    grid->addWidget(accountLabel, 1, 0);
    grid->addWidget(accountEdit, 1, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 1);

    accountEdit->setId(company.physicalAccount());
    accountEdit->setFocus();
    if (dialog->exec() != QDialog::Accepted) {
	delete dialog;
	return;
    }

    Id account_id = accountEdit->getId();
    delete dialog;
    if (account_id == INVALID_ID) return;

    ItemAdjust tx;
    tx.setPostDate(QDate::currentDate());
    tx.setPostTime(QTime::currentTime());
    tx.setMemo(tr("Physical Inventory"));
    tx.setStoreId(store_id);

    // Get discrepancies
    vector<Count> origs = counts;
    vector<Item> items;
    vector<QString> numbers;
    vector<QString> sizes;
    vector<fixed> on_hands;
    vector<fixed> qtys;
    vector<fixed> costs;
    vector<fixed> deposits;
    discrepancies(counts, false, store_id, items, numbers, sizes,
		  on_hands, qtys, costs, deposits);

    // Process discrepancies
    fixed total = 0.0;
    for (unsigned int i = 0; i < items.size(); ++i) {
	ItemLine item_line;
	item_line.item_id = items[i].id();
	item_line.number = numbers[i];
	item_line.size = sizes[i];
	item_line.size_qty = items[i].sizeQty(sizes[i]);
	item_line.quantity = qtys[i];
	item_line.inv_cost = costs[i];
	item_line.ext_deposit = deposits[i];
	tx.items().push_back(item_line);

	AccountLine acct_line(items[i].assetAccount(), costs[i]);
	tx.accounts().push_back(acct_line);

	if (deposits[i] != 0.0) {
	    AccountLine acct_line(company.depositAccount(), deposits[i]);
	    tx.accounts().push_back(acct_line);
	}
	total += costs[i] + deposits[i];
    }

    // If any discrepancies then set expense and create adjustment
    QString message;
    if (items.size() == 0) {
	message = tr("There was no adjustment created because there\n"
	    "were no discrepancies.");
    } else {
	tx.setAccountId(account_id);
	if (total != 0.0) {
	    AccountLine acct_line(account_id, -total);
	    tx.accounts().push_back(acct_line);
	}

	if (!_quasar->db()->create(tx)) {
	    QMessageBox::critical(this, tr("Error"),
				  tr("Posting adjustment failed"));
	    return;
	}

	message = tr("Created adjustment #%1 for discrepancies.")
	    .arg(tx.number());
    }

    // Update counts to set inactive and on_hand
    for (unsigned int c = 0; c < counts.size(); ++c)
	_quasar->db()->update(origs[c], counts[c]);

    if (company.physicalAccount() == INVALID_ID) {
	Company orig = company;
	company.setPhysicalAccount(account_id);
	_quasar->db()->update(orig, company);
    }

    slotRefresh();
    QMessageBox::information(this, tr("Processing Complete"), message);
}

void
CountList::discrepancies(vector<Count>& counts, bool inc_zero,
			 Id store_id, vector<Item>& items,
			 vector<QString>& numbers, vector<QString>& sizes,
			 vector<fixed>& on_hands, vector<fixed>& qtys,
			 vector<fixed>& costs, vector<fixed>& deposits)
{
    items.clear();
    numbers.clear();
    sizes.clear();
    on_hands.clear();
    qtys.clear();
    costs.clear();
    deposits.clear();

    // Total up amounts counted by item
    vector<Item> i_items;
    vector<fixed> i_qtys;
    vector<fixed> i_on_hands;
    unsigned int i;
    for (i = 0; i < counts.size(); ++i) {
	Count& count = counts[i];
	_quasar->db()->setActive(count, false);

	for (unsigned int j = 0; j < count.items().size(); ++j) {
	    CountItem& line = count.items()[j];
	    if (line.item_id == INVALID_ID) continue;
	    fixed qty = line.quantity * line.size_qty;

	    int found = -1;
	    for (unsigned int k = 0; k < i_items.size(); ++k) {
		if (line.item_id != i_items[k].id()) continue;
		found = k;
		break;
	    }

	    if (found != -1) {
		i_qtys[found] += qty;
	    } else {
		Item item;
		_quasar->db()->lookup(line.item_id, item);

		fixed on_hand, total_cost, on_order;
		QDate now = QDate::currentDate();
		_quasar->db()->itemGeneral(line.item_id, "", store_id,
					   now, on_hand, total_cost, on_order);

		i_items.push_back(item);
		i_on_hands.push_back(on_hand);
		i_qtys.push_back(qty);
	    }
	}
    }

    // TODO: sort items?

    // Process items
    for (i = 0; i < i_items.size(); ++i) {
	const Item& item = i_items[i];
	fixed counted = i_qtys[i];
	fixed on_hand = i_on_hands[i];
	fixed diff = counted - on_hand;

	if (diff == 0.0) {
	    if (inc_zero) {
		items.push_back(item);
		numbers.push_back(item.number());
		sizes.push_back(item.sellSize());
		on_hands.push_back(on_hand);
		qtys.push_back(counted);
		costs.push_back(0.0);
		deposits.push_back(0.0);
	    }
	    continue;
	}

	int sign = diff.sign();
	diff = diff.abs();
	bool useSellSize = true;

	int count = 0;
	while (diff != 0.0 && ++count < 10) {
	    int sizeNum = -1;
	    int smallest = -1;
	    for (unsigned int j = 0; j < item.sizes().size(); ++j) {
		const ItemSize& info = item.sizes()[j];
		if (smallest == -1 || item.sizes()[smallest].qty > info.qty)
		    smallest = j;
		if (count == 1 && info.name == item.sellSize() && useSellSize){
		    sizeNum = j;
		    break;
		}
		if (info.qty > diff)
		    continue;
		if (sizeNum != -1 && item.sizes()[sizeNum].qty > info.qty)
		    continue;
		sizeNum = j;
	    }

	    // Try to allocate fixed units or use smallest size
	    QString size;
	    fixed size_qty, alloc;
	    if (sizeNum != -1) {
		size = item.sizes()[sizeNum].name;
		size_qty = item.sizes()[sizeNum].qty;
		alloc = int((diff / size_qty).toDouble());
	    } else {
		size = item.sizes()[smallest].name;
		size_qty = item.sizes()[smallest].qty;
		alloc = diff / size_qty;
	    }

	    // Get cost and deposit
	    fixed cost;
	    _quasar->db()->itemSellingCost(item, size, store_id, alloc, 0,
					   cost);
	    fixed deposit = item.deposit() * size_qty * alloc;

	    // Add line to discrepancy
	    items.push_back(item);
	    numbers.push_back(item.number());
	    sizes.push_back(size);
	    on_hands.push_back(on_hand / size_qty);
	    qtys.push_back(alloc * sign);
	    costs.push_back(cost * sign);
	    deposits.push_back(deposit * sign);

	    diff -= alloc * size_qty;
	}
    }
}

bool
CountList::isActive(Id count_id)
{
    Count count;
    _quasar->db()->lookup(count_id, count);
    return count.isActive();
}

void
CountList::setActive(Id count_id, bool active)
{
    if (count_id == INVALID_ID) return;

    Count count;
    _quasar->db()->lookup(count_id, count);

    Count orig = count;
    _quasar->db()->setActive(count, active);
    _quasar->db()->update(orig, count);
}

void
CountList::dataEvent(DataEvent* e)
{
    if (e->dataType() == DataObject::COUNT)
	slotRefresh();
}

void
CountList::performRefresh()
{
    bool showInactive = _inactive->isChecked();
    Id count_id = currentId();
    QListViewItem* current = NULL;
    clearLists();

    CountSelect conditions;
    conditions.activeOnly = !showInactive;
    conditions.store_id = _store->getId();
    vector<Count> counts;
    _quasar->db()->select(counts, conditions);

    for (unsigned int i = 0; i < counts.size(); ++i) {
	Count& count = counts[i];

	ListViewItem* lvi = new ListViewItem(_list, count.id());
	lvi->setValue(0, count.date());
	lvi->setValue(1, count.number());
	lvi->setValue(2, count.description());
	if (showInactive) lvi->setValue(3, !count.isActive());
	if (count.id() == count_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();

    _list->setCurrentItem(current);
    _list->setSelected(current, true);
}

void
CountList::performPrint()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Grid* grid = Grid::buildGrid(_list, tr("Count List"));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
CountList::performNew()
{
    CountMaster* master = new CountMaster(_main, INVALID_ID);
    master->show();
}

void
CountList::performEdit()
{
    Id count_id = currentId();
    CountMaster* master = new CountMaster(_main, count_id);
    master->show();
}
