// $Id: find_tx.cpp,v 1.29 2005/01/30 04:25:31 bpepers Exp $
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

#include "find_tx.h"
#include "main_window.h"
#include "screen_decl.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "shift.h"
#include "gltx_select.h"
#include "employee.h"
#include "store.h"
#include "station.h"
#include "tx_combo.h"
#include "list_view_item.h"
#include "id_edit.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "date_range.h"
#include "date_popup.h"
#include "employee_lookup.h"
#include "gltx_lookup.h"
#include "store_lookup.h"
#include "station_lookup.h"
#include "grid.h"
#include "text_frame.h"
#include "line_frame.h"

#include <qapplication.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qmessagebox.h>

FindTx::FindTx(MainWindow* main)
    : QuasarWindow(main, "FindTx")
{
    _helpSource = "find_tx.html";

    QFrame* frame = new QFrame(this);

    QGroupBox* select = new QGroupBox(tr("Data Selection"), frame);
    QGridLayout* grid1 = new QGridLayout(select, 2, 1, select->frameWidth()*2);
    grid1->addRowSpacing(0, select->fontMetrics().height());

    QLabel* typeLabel = new QLabel(tr("Type:"), select);
    _type = new TxCombo(_quasar, select);
    typeLabel->setBuddy(_type);
    connect(_type, SIGNAL(activated(int)), SLOT(slotTypeChanged()));

    _numberLabel = new QLabel(tr("Transaction #:"), select);
    _number = new LineEdit(14, select);
    _numberLabel->setBuddy(_number);

    QLabel* referenceLabel = new QLabel(tr("Reference #:"), select);
    _reference = new LineEdit(select);
    _reference->setLength(14, '9');
    referenceLabel->setBuddy(_reference);

    QLabel* rangeLabel = new QLabel(tr("Date Range:"), select);
    _range = new DateRange(select);

    QLabel* fromLabel = new QLabel(tr("From:"), select);
    _from = new DatePopup(select);
    _range->setFromPopup(_from);
    fromLabel->setBuddy(_from);

    QLabel* toLabel = new QLabel(tr("To:"), select);
    _to = new DatePopup(select);
    _range->setToPopup(_to);
    toLabel->setBuddy(_to);

    QLabel* storeLabel = new QLabel(tr("Store:"), select);
    _store = new LookupEdit(new StoreLookup(_main, this), select);
    _store->setLength(30);
    storeLabel->setBuddy(_store);

    QLabel* stationLabel = new QLabel(tr("Station:"), select);
    _station = new LookupEdit(new StationLookup(_main, this), select);
    _station->setLength(30);
    stationLabel->setBuddy(_station);

    QLabel* employeeLabel = new QLabel(tr("Employee:"), select);
    _employee = new LookupEdit(new EmployeeLookup(_main, this), select);
    _employee->setLength(30);
    employeeLabel->setBuddy(_employee);

    QLabel* shiftLabel = new QLabel(tr("Shift:"), select);
    _shift = new LookupEdit(new GltxLookup(_main, this, DataObject::SHIFT),
			    select);
    _shift->setSizeInfo(8, '9');
    shiftLabel->setBuddy(_shift);

    _cardLabel = new QLabel(tr("Card:"), select);
    _cardLookup = new CardLookup(_main, this);
    _card = new LookupEdit(_cardLookup, select);
    _card->setLength(30);
    _cardLabel->setBuddy(_card);

    QLabel* totalLabel = new QLabel(tr("Total Amount:"), select);
    _total = new MoneyEdit(select);
    totalLabel->setBuddy(_total);

    grid1->setRowStretch(6, 1);
    grid1->setColStretch(2, 1);
    grid1->addWidget(typeLabel, 1, 0);
    grid1->addWidget(_type, 1, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(_numberLabel, 2, 0);
    grid1->addWidget(_number, 2, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(referenceLabel, 3, 0);
    grid1->addWidget(_reference, 3, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(rangeLabel, 4, 0);
    grid1->addWidget(_range, 4, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(fromLabel, 5, 0);
    grid1->addWidget(_from, 5, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(toLabel, 6, 0);
    grid1->addWidget(_to, 6, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(storeLabel, 1, 3);
    grid1->addWidget(_store, 1, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(stationLabel, 2, 3);
    grid1->addWidget(_station, 2, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(employeeLabel, 3, 3);
    grid1->addWidget(_employee, 3, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(shiftLabel, 4, 3);
    grid1->addWidget(_shift, 4, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(_cardLabel, 5, 3);
    grid1->addWidget(_card, 5, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(totalLabel, 6, 3);
    grid1->addWidget(_total, 6, 4, AlignLeft | AlignVCenter);

    _list = new ListView(frame);
    _list->addTextColumn(tr("Type"), 20);
    _list->addTextColumn(tr("Tx #"), 12, AlignRight);
    _list->addTextColumn(tr("Store"), 6, AlignRight);
    _list->addTextColumn(tr("Reference #"), 12, AlignRight);
    _list->addDateColumn(tr("Date"));
    _list->addTextColumn(tr("Card"), 20);
    _list->addMoneyColumn(tr("Total"));
    _list->setAllColumnsShowFocus(true);
    _list->setShowSortIndicator(true);
    _list->setSorting(4);

    QFrame* box = new QFrame(frame);

    QPushButton* refresh = new QPushButton(tr("&Refresh"), box);
    refresh->setMinimumSize(refresh->sizeHint());
    connect(refresh, SIGNAL(clicked()), SLOT(slotFind()));

    QPushButton* print = new QPushButton(tr("&Print"), box);
    print->setMinimumSize(refresh->sizeHint());
    connect(print, SIGNAL(clicked()), SLOT(slotPrint()));

    QPushButton* close = new QPushButton(tr("Cl&ose"), box);
    close->setMinimumSize(refresh->sizeHint());
    connect(close, SIGNAL(clicked()), SLOT(close()));

    QGridLayout* boxGrid = new QGridLayout(box);
    boxGrid->setSpacing(6);
    boxGrid->setMargin(6);
    boxGrid->setColStretch(0, 1);
    boxGrid->addWidget(refresh, 0, 0, AlignRight | AlignVCenter);
    boxGrid->addWidget(print, 0, 1, AlignRight | AlignVCenter);
    boxGrid->addWidget(close, 0, 2, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setRowStretch(1, 1);
    grid->addWidget(select, 0, 0);
    grid->addWidget(_list, 1, 0);
    grid->addWidget(box, 2, 0);

    _type->setFocus();
    _range->setCurrentItem(DateRange::Today);

    if (_quasar->storeCount() == 1) {
	storeLabel->hide();
	_store->hide();
    }

    connect(_number, SIGNAL(returnPressed()), SLOT(slotFind()));
    connect(_reference, SIGNAL(returnPressed()), SLOT(slotFind()));
    connect(_from->dateWidget(), SIGNAL(returnPressed()), SLOT(slotFind()));
    connect(_to->dateWidget(), SIGNAL(returnPressed()), SLOT(slotFind()));
    connect(_total, SIGNAL(returnPressed()), SLOT(slotFind()));
    connect(_card, SIGNAL(returnPressed()), SLOT(slotFind()));
    connect(_store, SIGNAL(returnPressed()), SLOT(slotFind()));
    connect(_station, SIGNAL(returnPressed()), SLOT(slotFind()));
    connect(_employee, SIGNAL(returnPressed()), SLOT(slotFind()));
    connect(_shift, SIGNAL(returnPressed()), SLOT(slotFind()));

    connect(_list,SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEdit()));
    connect(_list,SIGNAL(returnPressed(QListViewItem*)),
	    SLOT(slotEdit()));

    setCentralWidget(frame);
    setCaption(tr("Find Transaction"));
    finalize();
}

FindTx::~FindTx()
{
}

void
FindTx::setType(int type)
{
    _type->setCurrentItem(DataObject::dataTypeName(type));
    slotTypeChanged();
}

void
FindTx::setStoreId(Id store_id)
{
    _store->setId(store_id);
}

void
FindTx::setCardId(Id card_id)
{
    setType(999);
    _card->setId(card_id);
    slotFind();
}

void
FindTx::slotTypeChanged()
{
    if (_type->currentItem() == 0) {
	_numberLabel->setText(tr("Transaction No."));
	_cardLabel->setText(tr("Card:"));
	_card->setEnabled(true);
	_from->setFocus();
	return;
    }

    switch (DataObject::dataType(_type->currentText())) {
    case DataObject::GENERAL:
	_numberLabel->setText(tr("Id No."));
	_cardLabel->setText(tr("Card:"));
	_card->setEnabled(false);
	_number->setFocus();
	break;
    case DataObject::CHEQUE:
	_numberLabel->setText(tr("Cheque No."));
	_cardLabel->setText(tr("Card:"));
	_card->setEnabled(true);
	_number->setFocus();
	break;
    case DataObject::ITEM_ADJUST:
	_numberLabel->setText(tr("Adjustment No."));
	_cardLabel->setText(tr("Card:"));
	_card->setEnabled(false);
	_number->setFocus();
	break;
    case DataObject::TEND_ADJUST:
	_numberLabel->setText(tr("Adjustment No."));
	_cardLabel->setText(tr("Card:"));
	_card->setEnabled(false);
	_number->setFocus();
	break;
    case DataObject::CARD_ADJUST:
	_numberLabel->setText(tr("Adjustment No."));
	_cardLabel->setText(tr("Card:"));
	_card->setEnabled(true);
	_number->setFocus();
	break;
    case DataObject::INVOICE:
    case DataObject::RETURN:
	_numberLabel->setText(tr("Invoice No."));
	_cardLabel->setText(tr("Customer:"));
	_card->setEnabled(true);
	_number->setFocus();
	break;
    case DataObject::RECEIPT:
	_numberLabel->setText(tr("Receipt No."));
	_cardLabel->setText(tr("Customer:"));
	_card->setEnabled(true);
	_number->setFocus();
	break;
    case DataObject::RECEIVE:
    case DataObject::CLAIM:
	_numberLabel->setText(tr("Invoice No."));
	_cardLabel->setText(tr("Vendor:"));
	_card->setEnabled(true);
	_number->setFocus();
	break;
    case DataObject::WITHDRAW:
	_numberLabel->setText(tr("Tx No."));
	_cardLabel->setText(tr("Customer:"));
	_card->setEnabled(true);
	_number->setFocus();
	break;
    case DataObject::PAYOUT:
	_numberLabel->setText(tr("Tx No."));
	_cardLabel->setText(tr("Card:"));
	_card->setEnabled(false);
	_number->setFocus();
	break;
    default:
	_numberLabel->setText(tr("Transaction No."));
	_cardLabel->setText(tr("Card:"));
	_card->setEnabled(true);
	_from->setFocus();
    }
}

void
FindTx::slotFind()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    Id current_id = INVALID_ID;
    if (_list->currentItem() != NULL)
	current_id = ((ListViewItem*)_list->currentItem())->id;
    QListViewItem* current = NULL;
    _list->clear();

    GltxSelect conditions;
    if (_type->currentItem() != 0)
	conditions.type = DataObject::dataType(_type->currentText());
    if (!_number->text().isEmpty())
	conditions.number = _number->text();
    conditions.reference = _reference->text();
    conditions.start_date = _from->getDate();
    conditions.end_date = _to->getDate();
    conditions.store_id = _store->getId();
    conditions.station_id = _station->getId();
    conditions.employee_id = _employee->getId();
    conditions.shift_id = _shift->getId();
    conditions.card_id = _card->getId();
    conditions.activeOnly = true;

    vector<Gltx> gltxs;
    _quasar->db()->select(gltxs, conditions);

    for (unsigned int i = 0; i < gltxs.size(); ++i) {
	const Gltx& gltx = gltxs[i];

	if (_total->getFixed() != 0.0)
	    if (gltx.amount() != _total->getFixed())
		continue;

	Card card;
	_quasar->db()->lookup(gltx.cardId(), card);

	Store store;
	_quasar->db()->lookup(gltx.storeId(), store);

	ListViewItem* lvi = new ListViewItem(_list, gltx.id());
	lvi->setValue(0, gltx.dataTypeName());
	lvi->setValue(1, gltx.number());
	lvi->setValue(2, store.number());
	lvi->setValue(3, gltx.reference());
	lvi->setValue(4, gltx.postDate());
	lvi->setValue(5, card.nameFL());
	lvi->setValue(6, gltx.amount());
	if (gltx.id() == current_id) current = lvi;
    }

    if (current == NULL) current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);

    QApplication::restoreOverrideCursor();
}

QString
FindTx::buildTitle(const QString& name)
{
    QDate from = _from->getDate();
    QDate to = _to->getDate();
    DateValcon valcon;
    QStringList lines;

    // Dates
    if (!from.isNull() && !to.isNull())
	lines << valcon.format(from) + tr("  to  ") + valcon.format(to);
    else if (!from.isNull())
	lines << tr("On/After  ") + valcon.format(from);
    else if (!to.isNull())
	lines << tr("On/Before  ") + valcon.format(to);

    // Shift
    if (_shift->getId() != INVALID_ID) {
	Shift shift;
	_quasar->db()->lookup(_shift->getId(), shift);
	lines << tr("Shift #") + shift.number();
    }

    // Station
    if (_station->getId() != INVALID_ID) {
	Station station;
	_quasar->db()->lookup(_station->getId(), station);
	lines << tr("Station: ") + station.name();
    }

    // Employee
    if (_employee->getId() != INVALID_ID) {
	Card employee;
	_quasar->db()->lookup(_employee->getId(), employee);
	lines << tr("Employee: ") + employee.nameFL();
    }

    // Card
    if (_card->getId() != INVALID_ID) {
	Card card;
	_quasar->db()->lookup(_card->getId(), card);
	lines << card.dataTypeName() + ": " + card.nameFL();
    }

    // Store
    if (_store->getId() != INVALID_ID) {
	Store store;
	_quasar->db()->lookup(_store->getId(), store);
	lines << tr("Store: ") + store.name();
    }

    // Total
    if (_total->getFixed() != 0.0) {
	lines << tr("Total Amount: ") + _total->getFixed().toString();
    }

    QString title = name;
    if (lines.count() > 0) title += "\n\n" + lines.join("\n");
    return title;
}

void
FindTx::slotPrint()
{
    if (_list->childCount() == 0) {
	QMessageBox::warning(this, tr("Warning"), tr("No data to print"));
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QString name = tr("Transactions");
    if (_type->currentItem() != 0)
	name = _type->currentText() + " " + name;

    Grid* grid = Grid::buildGrid(_list, buildTitle(name));

    QApplication::restoreOverrideCursor();
    grid->print(this);
    delete grid;
}

void
FindTx::slotEdit()
{
    ListViewItem* item = (ListViewItem*)_list->currentItem();
    if (item == NULL) return;

    Gltx gltx;
    Id gltx_id = item->id;
    if (!_quasar->db()->lookup(gltx_id, gltx)) return;

    QWidget* edit = editGltx(gltx, _main);
    if (edit != NULL) edit->show();
}
