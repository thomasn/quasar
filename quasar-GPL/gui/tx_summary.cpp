// $Id: tx_summary.cpp,v 1.10 2005/01/30 04:25:31 bpepers Exp $
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

#include "tx_summary.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "shift.h"
#include "invoice.h"
#include "tender_adjust.h"
#include "station.h"
#include "list_view_item.h"
#include "gltx_select.h"
#include "tender_select.h"
#include "tx_combo.h"
#include "lookup_edit.h"
#include "station_lookup.h"
#include "gltx_lookup.h"
#include "employee_lookup.h"
#include "store_lookup.h"
#include "account_lookup.h"
#include "tender_lookup.h"
#include "discount_lookup.h"
#include "tax_lookup.h"
#include "dept_lookup.h"
#include "subdept_lookup.h"
#include "date_range.h"
#include "date_popup.h"
#include "money_edit.h"
#include "integer_edit.h"
#include "combo_box.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtabwidget.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qtimer.h>
#include <assert.h>

TxSummary::TxSummary(MainWindow* main)
    : QuasarWindow(main, "TxSummary")
{
    _helpSource = "tx_summary.html";

    QFrame* frame = new QFrame(this);

    QGroupBox* select = new QGroupBox(tr("Data Selection"), frame);
    QGridLayout* grid1 = new QGridLayout(select, 2, 1, select->frameWidth()*2);
    grid1->addRowSpacing(0, select->fontMetrics().height());

    QLabel* rangeLabel = new QLabel(tr("Date Range:"), select);
    _range = new DateRange(select);

    QLabel* fromLabel = new QLabel(tr("From Date:"), select);
    _from = new DatePopup(select);
    _range->setFromPopup(_from);
    fromLabel->setBuddy(_from);

    QLabel* toLabel = new QLabel(tr("To Date:"), select);
    _to = new DatePopup(select);
    _range->setToPopup(_to);
    toLabel->setBuddy(_to);

    QLabel* storeLabel = new QLabel(tr("Store:"), select);
    _store = new LookupEdit(new StoreLookup(main, this), select);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QLabel* stationLabel = new QLabel(tr("Station:"), select);
    _station = new LookupEdit(new StationLookup(main, this), select);
    _station->setLength(30);
    stationLabel->setBuddy(_station);

    QLabel* employeeLabel = new QLabel(tr("Employee:"), select);
    _employee = new LookupEdit(new EmployeeLookup(main, this), select);
    _employee->setLength(30);
    employeeLabel->setBuddy(_employee);

    grid1->setColStretch(2, 1);
    grid1->addColSpacing(2, 20);
    grid1->addWidget(rangeLabel, 1, 0);
    grid1->addWidget(_range, 1, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(fromLabel, 2, 0);
    grid1->addWidget(_from, 2, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(toLabel, 3, 0);
    grid1->addWidget(_to, 3, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(storeLabel, 1, 3);
    grid1->addWidget(_store, 1, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(stationLabel, 2, 3);
    grid1->addWidget(_station, 2, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(employeeLabel, 3, 3);
    grid1->addWidget(_employee, 3, 4, AlignLeft | AlignVCenter);

    _tabs = new QTabWidget(frame);

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
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(2, 1);
    buttonGrid->addWidget(refresh, 0, 0, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(print, 0, 1, AlignLeft | AlignVCenter);
    buttonGrid->addWidget(ok, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->setColStretch(0, 1);
    grid->addWidget(select, 0, 0);
    grid->addWidget(_tabs, 1, 0);
    grid->addWidget(buttons, 2, 0);

    _range->setFocus();
    _range->setCurrentItem(DateRange::Today);
    _tabs->hide();

    TenderSelect conditions;
    _quasar->db()->select(_tenders, conditions);
    for (unsigned int i = 0; i < _tenders.size(); ++i)
	_tenderCnts.push_back(0);

    setCentralWidget(frame);
    setCaption(tr("Daily Summary"));
    finalize();
}

TxSummary::~TxSummary()
{
}

void
TxSummary::setDates(QDate from, QDate to)
{
    _from->setDate(from);
    _to->setDate(to);
}

void
TxSummary::refresh()
{
    QTimer::singleShot(500, this, SLOT(slotRefresh()));
}

void
TxSummary::slotPrint()
{
    // TODO: printing
}

void
TxSummary::slotRefresh()
{
    _tabs->hide();
    qApp->processEvents();
    adjustSize();

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    // Load transactions
    GltxSelect conditions;
    conditions.start_date = _from->getDate();
    conditions.end_date = _to->getDate();
    conditions.store_id = _store->getId();
    conditions.station_id = _station->getId();
    conditions.employee_id = _employee->getId();
    _quasar->db()->select(_gltxs, conditions);

    slotProcess();

    QApplication::restoreOverrideCursor();

    _tabs->show();
    qApp->processEvents();
    adjustSize();
}

void
TxSummary::slotProcess()
{
    unsigned int i;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    _employees.clear();
    for (i = 0; i < _frames.size(); ++i)
	delete _frames[i];
    _frames.clear();

    for (i = 0; i < _gltxs.size(); ++i) {
	const Gltx& gltx = _gltxs[i];
	processGltx(gltx);
    }

    EmployeeInfo total;
    total.employee.setLastName(tr("Total"));

    for (i = 0; i < _employees.size(); ++i) {
	EmployeeInfo& info = _employees[i];
	addFrame(info);
	total.addInfo(info);
    }
    addFrame(total);

    QApplication::restoreOverrideCursor();
}

void
TxSummary::processGltx(const Gltx& gltx)
{
    Employee employee;
    findEmployee(gltx.employeeId(), employee);

    EmployeeInfo& info = findEmployeeInfo(employee);

    // Voided transactions just get counted
    if (!gltx.isActive()) {
	info.voidTxCnt++;
	return;
    }

    // Further processing based on type
    switch (gltx.dataType()) {
    case DataObject::INVOICE:		processInvoice(info, gltx); break;
    case DataObject::RETURN:		processReturn(info, gltx); break;
    case DataObject::PAYOUT:		processPayout(info, gltx); break;
    case DataObject::RECEIPT:		processPayment(info, gltx); break;
    case DataObject::WITHDRAW:		processWithdraw(info, gltx); break;
    case DataObject::NOSALE:		processNosale(info, gltx); break;
    case DataObject::TEND_ADJUST:	processTenderAdjust(info, gltx); break;
    default:				processOther(info, gltx);
    }
}

void
TxSummary::processInvoice(EmployeeInfo& info, const Gltx& gltx)
{
    info.sales.count++;

    for (unsigned int i = 0; i < gltx.tenders().size(); ++i) {
	const TenderLine& line = gltx.tenders()[i];
	if (line.voided) continue;

	countTender(line.tender_id);
	info.sales.addAmount(line.tender_id, line.conv_amt);
    }

    Invoice invoice;
    _quasar->db()->lookup(gltx.id(), invoice);

    // TODO: process items and discounts
}

void
TxSummary::processReturn(EmployeeInfo& info, const Gltx& gltx)
{
    info.returns.count++;

    for (unsigned int i = 0; i < gltx.tenders().size(); ++i) {
	const TenderLine& line = gltx.tenders()[i];
	if (line.voided) continue;

	countTender(line.tender_id);
	info.returns.addAmount(line.tender_id, line.conv_amt);
    }

    Invoice invoice;
    _quasar->db()->lookup(gltx.id(), invoice);

    // TODO: process items and discounts
}

void
TxSummary::processPayout(EmployeeInfo& info, const Gltx& gltx)
{
    info.payouts.count++;

    for (unsigned int i = 0; i < gltx.tenders().size(); ++i) {
	const TenderLine& line = gltx.tenders()[i];
	if (line.voided) continue;

	countTender(line.tender_id);
	info.payouts.addAmount(line.tender_id, line.conv_amt);
    }
}

void
TxSummary::processPayment(EmployeeInfo& info, const Gltx& gltx)
{
    info.payments.count++;

    for (unsigned int i = 0; i < gltx.tenders().size(); ++i) {
	const TenderLine& line = gltx.tenders()[i];
	if (line.voided) continue;

	countTender(line.tender_id);
	info.payments.addAmount(line.tender_id, line.conv_amt);
    }
}

void
TxSummary::processWithdraw(EmployeeInfo& info, const Gltx& gltx)
{
    info.withdraws.count++;

    for (unsigned int i = 0; i < gltx.tenders().size(); ++i) {
	const TenderLine& line = gltx.tenders()[i];
	if (line.voided) continue;

	countTender(line.tender_id);
	info.withdraws.addAmount(line.tender_id, line.conv_amt);
    }
}

void
TxSummary::processNosale(EmployeeInfo& info, const Gltx&)
{
    info.nosales.count++;
}

void
TxSummary::processTenderAdjust(EmployeeInfo& info, const Gltx& gltx)
{
    TenderAdjust adjustment;
    _quasar->db()->lookup(gltx.id(), adjustment);

    EmployeeLine* type = NULL;
    QString memo = gltx.memo();
    if (memo == tr("Deposit to safe"))
	type = &info.deposits;
    else if (memo == tr("Over/Short"))
	type = &info.overShorts;
    else
	type = &info.others;

    for (unsigned int i = 0; i < gltx.tenders().size(); ++i) {
	const TenderLine& line = gltx.tenders()[i];
	if (line.voided) continue;

	countTender(line.tender_id);
	type->addAmount(line.tender_id, line.conv_amt);
    }
}

void
TxSummary::processOther(EmployeeInfo& info, const Gltx& gltx)
{
    info.others.count++;

    for (unsigned int i = 0; i < gltx.tenders().size(); ++i) {
	const TenderLine& line = gltx.tenders()[i];
	if (line.voided) continue;

	countTender(line.tender_id);
	info.others.addAmount(line.tender_id, line.conv_amt);
    }
}

void
TxSummary::countTender(Id tender_id)
{
    for (unsigned int i = 0; i < _tenders.size(); ++i) {
	if (_tenders[i].id() == tender_id) {
	    ++_tenderCnts[i];
	    return;
	}
    }

    Tender tender;
    if (_quasar->db()->lookup(tender_id, tender)) {
	_tenders.push_back(tender);
	_tenderCnts.push_back(1);
    }
}

bool
TxSummary::findTender(Id tender_id, Tender& tender)
{
    for (unsigned int i = 0; i < _tenders.size(); ++i) {
	if (_tenders[i].id() == tender_id) {
	    ++_tenderCnts[i];
	    tender = _tenders[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(tender_id, tender))
	return false;

    _tenders.push_back(tender);
    _tenderCnts.push_back(1);
    return true;
}

void
TxSummary::addFrame(EmployeeInfo& info)
{
    QString name = info.employee.name();
    if (name.isEmpty()) name = "<Blank>";

    QFrame* frame = new QFrame(_tabs);
    _tabs->addTab(frame, name);
    _frames.push_back(frame);

    ListView* list = new ListView(frame);
    list->setAllColumnsShowFocus(true);
    list->setSorting(-1);
    list->addTextColumn(tr(""), 12);
    for (unsigned int i = 0; i < _tenders.size(); ++i) {
	if (_tenderCnts[i] == 0)
	    continue;
	list->addMoneyColumn(_tenders[i].name());
    }
    list->addMoneyColumn(tr("Total"));

    QLabel* voidTxLabel = new QLabel(tr("Trans. Voids:"), frame);
    IntegerEdit* voidTxCnt = new IntegerEdit(frame);
    voidTxCnt->setLength(6, '9');
    voidTxCnt->setFocusPolicy(NoFocus);
    voidTxCnt->setInt(info.voidTxCnt);

    QLabel* voidItemLabel = new QLabel(tr("Item Voids:"), frame);
    IntegerEdit* voidItemCnt = new IntegerEdit(frame);
    voidItemCnt->setLength(6, '9');
    voidItemCnt->setFocusPolicy(NoFocus);
    voidItemCnt->setInt(info.voidItemCnt);

    QLabel* discountLabel = new QLabel(tr("Discounts:"), frame);
    MoneyEdit* discountTotal = new MoneyEdit(frame);
    discountTotal->setLength(8, '9');
    discountTotal->setFocusPolicy(NoFocus);
    discountTotal->setFixed(info.discountTotal);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(0, 1);
    grid->setColStretch(2, 1);
    grid->addMultiCellWidget(list, 0, 0, 0, 4);
    grid->addWidget(voidTxLabel, 1, 0);
    grid->addWidget(voidTxCnt, 1, 1);
    grid->addWidget(discountLabel, 1, 3);
    grid->addWidget(discountTotal, 1, 4);
    grid->addWidget(voidItemLabel, 2, 0);
    grid->addWidget(voidItemCnt, 2, 1);

    EmployeeLine total;
    total.addLine(info.sales);
    total.addLine(info.returns);
    total.addLine(info.payouts);
    total.addLine(info.payments);
    total.addLine(info.withdraws);
    total.addLine(info.nosales);
    total.addLine(info.others);

    EmployeeLine remain;
    remain.addLine(total);
    remain.addLine(info.deposits);
    remain.addLine(info.overShorts);

    _last = NULL;
    listItem(list, tr("Sales"), info.sales);
    listItem(list, tr("Returns"), info.returns);
    listItem(list, tr("Payouts"), info.payouts);
    listItem(list, tr("Payments"), info.payments);
    listItem(list, tr("Withdraws"), info.withdraws);
    listItem(list, tr("Nosales"), info.nosales);
    listItem(list, tr("Other"), info.others);
    listItem(list, tr("Total"), total);
    listItem(list, tr("Deposited"), info.deposits);
    listItem(list, tr("Over/Short"), info.overShorts);
    listItem(list, tr("Remain"), remain);
}

void
TxSummary::listItem(ListView* list, const QString& name,
		    const EmployeeLine& line)
{
    ListViewItem* item = new ListViewItem(list, _last);
    item->setValue(0, name);
    _last = item;

    int col = 1;
    fixed total = 0.0;
    for (unsigned int i = 0; i < _tenders.size(); ++i) {
	if (_tenderCnts[i] == 0) continue;
	fixed amount = line.getAmount(_tenders[i].id());

	item->setValue(col++, amount);
	total += amount;
    }
    item->setValue(col, total);
}

bool
TxSummary::findAccount(Id account_id, Account& account)
{
    for (unsigned int i = 0; i < _accountCache.size(); ++i) {
	if (_accountCache[i].id() == account_id) {
	    account = _accountCache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(account_id, account))
	return false;

    _accountCache.push_back(account);
    return true;
}

bool
TxSummary::findTax(Id tax_id, Tax& tax)
{
    for (unsigned int i = 0; i < _taxCache.size(); ++i) {
	if (_taxCache[i].id() == tax_id) {
	    tax = _taxCache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(tax_id, tax))
	return false;

    _taxCache.push_back(tax);
    return true;
}

bool
TxSummary::findCard(Id card_id, Card& card)
{
    for (unsigned int i = 0; i < _cardCache.size(); ++i) {
	if (_cardCache[i].id() == card_id) {
	    card = _cardCache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(card_id, card))
	return false;

    _cardCache.push_back(card);
    return true;
}

bool
TxSummary::findEmployee(Id employee_id, Employee& employee)
{
    for (unsigned int i = 0; i < _employeeCache.size(); ++i) {
	if (_employeeCache[i].id() == employee_id) {
	    employee = _employeeCache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(employee_id, employee))
	return false;

    _employeeCache.push_back(employee);
    return true;
}

bool
TxSummary::findStore(Id store_id, Store& store)
{
    for (unsigned int i = 0; i < _storeCache.size(); ++i) {
	if (_storeCache[i].id() == store_id) {
	    store = _storeCache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(store_id, store))
	return false;

    _storeCache.push_back(store);
    return true;
}

bool
TxSummary::findDept(Id dept_id, Dept& dept)
{
    for (unsigned int i = 0; i < _deptCache.size(); ++i) {
	if (_deptCache[i].id() == dept_id) {
	    dept = _deptCache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(dept_id, dept))
	return false;

    _deptCache.push_back(dept);
    return true;
}

bool
TxSummary::findSubdept(Id subdept_id, Subdept& subdept)
{
    for (unsigned int i = 0; i < _subdeptCache.size(); ++i) {
	if (_subdeptCache[i].id() == subdept_id) {
	    subdept = _subdeptCache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(subdept_id, subdept))
	return false;

    _subdeptCache.push_back(subdept);
    return true;
}

bool
TxSummary::findItem(Id item_id, Item& item)
{
    for (unsigned int i = 0; i < _itemCache.size(); ++i) {
	if (_itemCache[i].id() == item_id) {
	    item = _itemCache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(item_id, item))
	return false;

    _itemCache.push_back(item);
    return true;
}

bool
TxSummary::findDiscount(Id discount_id, Discount& discount)
{
    for (unsigned int i = 0; i < _discountCache.size(); ++i) {
	if (_discountCache[i].id() == discount_id) {
	    discount = _discountCache[i];
	    return true;
	}
    }

    if (!_quasar->db()->lookup(discount_id, discount))
	return false;

    _discountCache.push_back(discount);
    return true;
}

EmployeeInfo&
TxSummary::findEmployeeInfo(const Employee& employee)
{
    for (unsigned int i = 0; i < _employees.size(); ++i)
	if (_employees[i].employee == employee)
	    return _employees[i];

    EmployeeInfo info;
    info.employee = employee;
    _employees.push_back(info);
    return _employees[_employees.size() - 1];
}

EmployeeLine::EmployeeLine()
    : count(0)
{
}

void
EmployeeLine::addAmount(Id tender_id, fixed amount)
{
    for (unsigned int i = 0; i < tenders.size(); ++i) {
	if (tenders[i] == tender_id) {
	    amounts[i] += amount;
	    return;
	}
    }

    tenders.push_back(tender_id);
    amounts.push_back(amount);
}

void
EmployeeLine::addLine(const EmployeeLine& line)
{
    for (unsigned int i = 0; i < line.tenders.size(); ++i)
	addAmount(line.tenders[i], line.amounts[i]);
}

fixed
EmployeeLine::getAmount(Id tender_id) const
{
    for (unsigned int i = 0; i < tenders.size(); ++i)
	if (tenders[i] == tender_id)
	    return amounts[i];
    return 0.0;
}

EmployeeInfo::EmployeeInfo()
    : voidTxCnt(0), voidItemCnt(0)
{
}

void
EmployeeInfo::addInfo(const EmployeeInfo& info)
{
    sales.addLine(info.sales);
    returns.addLine(info.returns);
    payouts.addLine(info.payouts);
    payments.addLine(info.payments);
    withdraws.addLine(info.withdraws);
    nosales.addLine(info.nosales);
    others.addLine(info.others);
    deposits.addLine(info.deposits);
    overShorts.addLine(info.overShorts);

    voidTxCnt += info.voidTxCnt;
    voidItemCnt += info.voidItemCnt;
    discountTotal += info.discountTotal;
    itemQtyTotal += info.itemQtyTotal;
}
