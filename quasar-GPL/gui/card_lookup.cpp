// $Id: card_lookup.cpp,v 1.27 2004/09/20 22:48:31 bpepers Exp $
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

#include "card_lookup.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "card_select.h"
#include "list_view_item.h"
#include "card.h"
#include "customer_master.h"
#include "employee_master.h"
#include "personal_master.h"
#include "vendor_master.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qlayout.h>
#include <assert.h>

CardLookup::CardLookup(MainWindow* main, QWidget* parent, int matchType)
    : QuasarLookup(main, parent, "CardLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(40);
    _list->addTextColumn(tr("Name"), 40);
    _list->addTextColumn(tr("Number"), 12);
    _list->addColumn(tr("Type"), fontMetrics().width("Employee_"));
    setCaption(tr("Card Lookup"));

    QLabel* typeLabel = new QLabel(tr("Type:"), _search);
    type = new ComboBox(_search);
    typeLabel->setBuddy(type);

    setTabOrder(_text, type);
    _searchGrid->addWidget(typeLabel, 1, 0);
    _searchGrid->addWidget(type, 1, 1, AlignLeft | AlignVCenter);

    type->insertItem(tr("All Types"));
    type->insertItem(tr("Customer and Vendor"));
    type->insertItem(DataObject::dataTypeName(DataObject::CUSTOMER));
    type->insertItem(DataObject::dataTypeName(DataObject::VENDOR));
    type->insertItem(DataObject::dataTypeName(DataObject::EMPLOYEE));
    type->insertItem(DataObject::dataTypeName(DataObject::PERSONAL));

    if (matchType != -1) {
	DataObject::DataType dataType = DataObject::DataType(matchType);
	type->setCurrentItem(DataObject::dataTypeName(dataType));
    }
}

CardLookup::CardLookup(QuasarClient* quasar, QWidget* parent, int matchType)
    : QuasarLookup(quasar, parent, "CardLookup"), activeOnly(true)
{
    _label->setText(tr("Name:"));
    _text->setLength(40);
    _list->addTextColumn(tr("Name"), 40);
    _list->addTextColumn(tr("Number"), 12);
    _list->addColumn(tr("Type"), fontMetrics().width("Employee_"));
    setCaption(tr("Card Lookup"));

    QLabel* typeLabel = new QLabel(tr("Type:"), _search);
    type = new ComboBox(_search);
    typeLabel->setBuddy(type);

    setTabOrder(_text, type);
    _searchGrid->addWidget(typeLabel, 1, 0);
    _searchGrid->addWidget(type, 1, 1, AlignLeft | AlignVCenter);

    type->insertItem(tr("All Types"));
    type->insertItem(tr("Customer and Vendor"));
    type->insertItem(DataObject::dataTypeName(DataObject::CUSTOMER));
    type->insertItem(DataObject::dataTypeName(DataObject::VENDOR));
    type->insertItem(DataObject::dataTypeName(DataObject::EMPLOYEE));
    type->insertItem(DataObject::dataTypeName(DataObject::PERSONAL));

    if (matchType != -1) {
	DataObject::DataType dataType = DataObject::DataType(matchType);
	type->setCurrentItem(DataObject::dataTypeName(dataType));
    }
}

CardLookup::~CardLookup()
{
}

QString
CardLookup::lookupById(Id card_id)
{
    Card card;
    if (card_id != INVALID_ID && _quasar->db()->lookup(card_id, card))
	return card.name();
    return "";
}

vector<DataPair>
CardLookup::lookupByText(const QString& text)
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    CardSelect conditions;
    vector<Card> cards;

    if (!text.isEmpty()) {
	conditions.number = text;
	conditions.type = matchType();
	conditions.activeOnly = activeOnly;
	_quasar->db()->select(cards, conditions);
    }

    if (cards.size() == 0 && !text.isEmpty()) {
	conditions.number = "";
	conditions.name = text + "%";
	_quasar->db()->select(cards, conditions);
    }

    vector<DataPair> data;
    for (unsigned int i = 0; i < cards.size(); ++i) {
	if (type->currentText() == tr("Customer and Vendor")) {
	    switch (cards[i].dataType()) {
	    case DataObject::CUSTOMER:
	    case DataObject::VENDOR:
		break;
	    default:
		continue;
	    }
	}
	data.push_back(DataPair(cards[i].id(), cards[i].name()));
    }

    QApplication::restoreOverrideCursor();
    return data;
}

void
CardLookup::refresh()
{
    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    _list->clear();
    CardSelect conditions;
    if (!text().isEmpty())
	conditions.name = text() + "%";
    conditions.type = matchType();
    conditions.activeOnly = activeOnly;

    int count;
    _quasar->db()->count(count, conditions);
    if (count > 100) {
	QApplication::restoreOverrideCursor();

	QWidget* parent = this;
	if (isHidden() && parentWidget() != NULL)
	    parent = parentWidget();

	QString message = tr("This will select %1 cards\n"
			     "which may be slow.  Continue?").arg(count);
	int ch = QMessageBox::warning(parent, tr("Are You Sure?"), message,
				      QMessageBox::Yes, QMessageBox::No);
	if (ch != QMessageBox::Yes) return;

	QApplication::setOverrideCursor(waitCursor);
	qApp->processEvents();
    }

    vector<Card> cards;
    _quasar->db()->select(cards, conditions);

    for (unsigned int i = 0; i < cards.size(); ++i) {
	Card& card = cards[i];

	if (type->currentText() == tr("Customer and Vendor")) {
	    switch (card.dataType()) {
	    case DataObject::CUSTOMER:
	    case DataObject::VENDOR:
		break;
	    default:
		continue;
	    }
	}

	ListViewItem* item = new ListViewItem(_list, card.id());
	item->setValue(0, card.name());
	item->setValue(1, card.number());
	item->setValue(2, card.dataTypeName());
    }

    QListViewItem* current = _list->firstChild();
    _list->setCurrentItem(current);
    _list->setSelected(current, true);

    QApplication::restoreOverrideCursor();
}

QWidget*
CardLookup::slotNew()
{
    QString name = _text->text().simplifyWhiteSpace();

    int type = matchType();
    if (type == -1) {
        QDialog* dialog = new QDialog(parentWidget(), "Dialog", true);
	dialog->setCaption("Card Type");
	QLabel* label = new QLabel(tr("Card Type:"), dialog);
	QComboBox* types = new QComboBox(dialog);

	types->insertItem(DataObject::dataTypeName(DataObject::CUSTOMER));
	types->insertItem(DataObject::dataTypeName(DataObject::VENDOR));
	types->insertItem(DataObject::dataTypeName(DataObject::EMPLOYEE));
	types->insertItem(DataObject::dataTypeName(DataObject::PERSONAL));

	QFrame* buttons = new QFrame(dialog);
	QPushButton* ok = new QPushButton(tr("OK"), buttons);
	QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);
	connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
	connect(cancel, SIGNAL(clicked()), dialog, SLOT(reject()));
	ok->setDefault(true);

	QGridLayout* buttonGrid = new QGridLayout(buttons);
	buttonGrid->setSpacing(3);
	buttonGrid->setMargin(3);
	buttonGrid->setColStretch(0, 1);
	buttonGrid->addWidget(ok, 0, 1);
	buttonGrid->addWidget(cancel, 0, 2);

	QGridLayout* grid = new QGridLayout(dialog);
	grid->setSpacing(3);
	grid->setMargin(3);
	grid->setColStretch(1, 1);
	grid->addWidget(label, 0, 0);
	grid->addWidget(types, 0, 1);
	grid->addMultiCellWidget(buttons, 1, 1, 0, 1);

	int result = dialog->exec();
	type = DataObject::dataType(types->currentText());
	delete dialog;
	if (result != QDialog::Accepted) return NULL;
    }

    DataWindow* window = NULL;
    switch (type) {
    case DataObject::CUSTOMER:
	window = new CustomerMaster(_main);
	((CustomerMaster*)window)->setName(name);
	break;
    case DataObject::EMPLOYEE:
	window = new EmployeeMaster(_main);
	((EmployeeMaster*)window)->setName(name);
	break;
    case DataObject::PERSONAL:
	window = new PersonalMaster(_main);
	((PersonalMaster*)window)->setName(name);
	break;
    case DataObject::VENDOR:
	window = new VendorMaster(_main);
	((VendorMaster*)window)->setName(name);
	break;
    default:
        assert(false);
    }

    if (window) window->show();
    reject();
    return window;
}

QWidget*
CardLookup::slotEdit(Id id)
{
    Card card;
    if (!_quasar->db()->lookup(id, card)) return NULL;

    DataWindow* window = NULL;
    switch (card.dataType()) {
    case DataObject::CUSTOMER:
	window = new CustomerMaster(_main, id);
	break;
    case DataObject::EMPLOYEE:
	window = new EmployeeMaster(_main, id);
	break;
    case DataObject::PERSONAL:
	window = new PersonalMaster(_main, id);
	break;
    case DataObject::VENDOR:
	window = new VendorMaster(_main, id);
	break;
    default:
        assert(false);
    }

    if (window) window->show();
    reject();
    return window;
}

int
CardLookup::matchType()
{
    if (type->currentText() == tr("All Types"))
	return -1;
    if (type->currentText() == tr("Customer and Vendor"))
	return -1;
    return DataObject::dataType(type->currentText());
}
