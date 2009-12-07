// $Id: cheque_print.cpp,v 1.33 2004/09/20 17:35:55 bpepers Exp $
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

#include "cheque_print.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "company.h"
#include "account.h"
#include "card.h"
#include "receive.h"
#include "invoice.h"
#include "cheque_select.h"
#include "account_select.h"
#include "lookup_edit.h"
#include "account_lookup.h"
#include "list_view.h"
#include "list_view_item.h"
#include "date_valcon.h"
#include "money_valcon.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qdir.h>

static char* ones[] = {
    QT_TRANSLATE_NOOP("ChequePrint","Zero"),
    QT_TRANSLATE_NOOP("ChequePrint","One"),
    QT_TRANSLATE_NOOP("ChequePrint","Two"),
    QT_TRANSLATE_NOOP("ChequePrint","Three"),
    QT_TRANSLATE_NOOP("ChequePrint","Four"),
    QT_TRANSLATE_NOOP("ChequePrint","Five"),
    QT_TRANSLATE_NOOP("ChequePrint","Six"),
    QT_TRANSLATE_NOOP("ChequePrint","Seven"),
    QT_TRANSLATE_NOOP("ChequePrint","Eight"),
    QT_TRANSLATE_NOOP("ChequePrint","Nine"),
    QT_TRANSLATE_NOOP("ChequePrint","Ten"),
    QT_TRANSLATE_NOOP("ChequePrint","Eleven"),
    QT_TRANSLATE_NOOP("ChequePrint","Twelve"),
    QT_TRANSLATE_NOOP("ChequePrint","Thirteen"),
    QT_TRANSLATE_NOOP("ChequePrint","Forteen"),
    QT_TRANSLATE_NOOP("ChequePrint","Fifteen"),
    QT_TRANSLATE_NOOP("ChequePrint","Sixteen"),
    QT_TRANSLATE_NOOP("ChequePrint","Seventeen"),
    QT_TRANSLATE_NOOP("ChequePrint","Eighteen"),
    QT_TRANSLATE_NOOP("ChequePrint","Nineteen")
};
static char* tens[] = {
    "", "",
    QT_TRANSLATE_NOOP("ChequePrint","Twenty"),
    QT_TRANSLATE_NOOP("ChequePrint","Thirty"),
    QT_TRANSLATE_NOOP("ChequePrint","Forty"),
    QT_TRANSLATE_NOOP("ChequePrint","Fifty"),
    QT_TRANSLATE_NOOP("ChequePrint","Sixty"),
    QT_TRANSLATE_NOOP("ChequePrint","Seventy"),
    QT_TRANSLATE_NOOP("ChequePrint","Eighty"),
    QT_TRANSLATE_NOOP("ChequePrint","Ninety")
};
static char* hundred = QT_TRANSLATE_NOOP("ChequePrint","Hundred");
static char* thousand = QT_TRANSLATE_NOOP("ChequePrint","Thousand");
static char* million = QT_TRANSLATE_NOOP("ChequePrint","Million");
static char* zero = QT_TRANSLATE_NOOP("ChequePrint","Zero");
static char* dollars = QT_TRANSLATE_NOOP("ChequePrint","Dollars");
static char* andText = QT_TRANSLATE_NOOP("ChequePrint","and");

QString
moneySmall(int& value)
{
    QString text = "";

    if (value >= 100) {
	text += qApp->translate("ChequePrint", ones[value / 100]) +
	    " " + qApp->translate("ChequePrint", hundred) + " ";
	value = value % 100;
    }

    if (value >= 20) {
	text += qApp->translate("ChequePrint", tens[value / 10]) + " ";
	value = value % 10;
    }

    if ((text != "" && value != 0) || text.isEmpty())
	text += qApp->translate("ChequePrint", ones[value]);

    return text;
}

QString
moneyText(fixed money)
{
    QString text = "";
    int value = money.toInt();

    if (value >= 1000000) {
	int millions = value / 1000000;
	text += moneySmall(millions) + " " +
	    qApp->translate("ChequePrint", million) + " ";
	value = value % 1000000;
    }

    if (value >= 1000) {
	int thousands = value / 1000;
	text += moneySmall(thousands) + " " +
	    qApp->translate("ChequePrint", thousand) + " ";
	value = value % 1000;
    }

    if (value > 0)
	text += moneySmall(value);

    int cents = int((money.toDouble() - money.toInt()) * 100 + .5);
    if (cents != 0.0) {
	if (text != "") text += " " + qApp->translate("ChequePrint", andText)
			    + " ";
	text += QString::number(cents) + "/100";
    }

    if (text == "") text += qApp->translate("ChequePrint", zero);
    text += " " + qApp->translate("ChequePrint", dollars);

    return text;
}

ChequePrint::ChequePrint(MainWindow* main)
    : QuasarWindow(main, "ChequePrint")
{
    _helpSource = "cheque_print.html";

    QFrame* frame = new QFrame(this);

    QLabel* accountLabel = new QLabel(tr("Account:"), frame);
    AccountLookup* acctLookup = new AccountLookup(main, this, Account::Bank);
    _account = new LookupEdit(acctLookup, frame);
    _account->setLength(30);
    accountLabel->setBuddy(_account);
    connect(_account, SIGNAL(validData()), SLOT(slotAccountChanged()));

    _cheques = new ListView(frame);
    _cheques->addTextColumn(tr("Number"), 10, AlignRight);
    _cheques->addTextColumn(tr("Name"), 20);
    _cheques->addDateColumn(tr("Date"));
    _cheques->addMoneyColumn(tr("Amount"));
    _cheques->addCheckColumn(tr("Printed?"));
    _cheques->setAllColumnsShowFocus(true);
    _cheques->setVScrollBarMode(QScrollView::AlwaysOn);
    _cheques->setSorting(0);
    _cheques->setShowSortIndicator(true);
    _cheques->setSelectionMode(QListView::Extended);

    QLabel* typeLabel = new QLabel(tr("Type:"), frame);
    _type = new QComboBox(frame);
    _type->setMinimumWidth(300);
    typeLabel->setBuddy(_type);

    _printed = new QCheckBox(tr("Show Printed?"), frame);
    connect(_printed, SIGNAL(toggled(bool)), SLOT(slotPrintChanged()));

    QFrame* buttons = new QFrame(frame);
    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);

    connect(ok, SIGNAL(clicked()), SLOT(slotOk()));
    connect(cancel, SIGNAL(clicked()), SLOT(close()));

    ok->setMinimumSize(cancel->sizeHint());
    cancel->setMinimumSize(cancel->sizeHint());

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 0, AlignRight | AlignVCenter);
    buttonGrid->addWidget(cancel, 0, 1, AlignRight | AlignVCenter);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->setColStretch(2, 1);
    grid->addWidget(accountLabel, 0, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_account, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_cheques, 1, 1, 0, 2);
    grid->addWidget(typeLabel, 2, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_type, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(_printed, 2, 2, AlignRight | AlignVCenter);
    grid->addMultiCellWidget(buttons, 3, 3, 0, 2);

    // Set default bank account
    Company company;
    _quasar->db()->lookup(company);
    _account->setId(company.bankAccount());

    loadCheques();
    _account->setFocus();

    setCentralWidget(frame);
    setCaption(tr("Print Cheques"));
    finalize();

    _quasar->resourceList("cheque_fmts", "name", _labelFiles, _labelNames);

    QStringList names = _labelNames;
    names.sort();
    for (unsigned int i = 0; i < names.size(); ++i)
	_type->insertItem(names[i]);
}

ChequePrint::~ChequePrint()
{
}

void
ChequePrint::setCheque(Id cheque_id)
{
    Cheque cheque;
    if (!_quasar->db()->lookup(cheque_id, cheque)) return;

    _account->setId(cheque.accountId());
    loadCheques();

    ListViewItem* child = (ListViewItem*)_cheques->firstChild();
    while (child != NULL) {
	if (child->id == cheque_id) {
	    _cheques->setSelected(child, true);
	    break;
	}
	child = (ListViewItem*)child->nextSibling();
    }
}

void
ChequePrint::slotAccountChanged()
{
    loadCheques();
}

void
ChequePrint::slotPrintChanged()
{
    loadCheques();
}

void
ChequePrint::slotOk()
{
    if (_type->count() == 0) {
	QMessageBox::critical(this, tr("Error"), tr("Invalid label type"));
	return;
    }

    QString name = _type->currentText();
    QString fileName;
    for (unsigned int i = 0; i < _labelNames.size(); ++i) {
	if (_labelNames[i] == name) {
	    fileName = _labelFiles[i];
	    break;
	}
    }
    if (fileName.isEmpty()) {
	QMessageBox::critical(this, tr("Error"), tr("Can't find label type"));
	return;
    }

    QString filePath;
    if (!_quasar->resourceFetch("cheque_fmts", fileName, filePath)) {
	QString msg = tr("Failed fetching cheque format '%1'").arg(fileName);
	QMessageBox::critical(this, tr("Error"), msg);
	return;
    }

    LabelDefn defn;
    if (!defn.load(filePath)) {
	QString msg = tr("Cheque format '%1' is invalid").arg(fileName);
	QMessageBox::critical(this, tr("Error"), msg);
	return;
    }

    // Get list of cheque ids
    vector<Id> cheque_ids;
    QListViewItem* child = _cheques->firstChild();
    while (child != NULL) {
	if (child->isSelected())
	    cheque_ids.push_back(((ListViewItem*)child)->id);
	child = child->nextSibling();
    }

    // Check thats something was selected
    if (cheque_ids.size() == 0) {
	QMessageBox::critical(this, tr("Error"),
			      tr("No cheques selected to print"));
	return;
    }

    QPrinter* printer = new QPrinter(QPrinter::HighResolution);
    printer->setColorMode(QPrinter::Color);
    printer->setPageSize(QPrinter::Letter);
    printer->setFullPage(true);
    if (defn.landscape)
	printer->setOrientation(QPrinter::Landscape);
    else
	printer->setOrientation(QPrinter::Portrait);
    if (!printer->setup(NULL)) {
        delete printer;
        return;
    }

    // TODO: if page size picked is not the same as the label definition
    // page size, adjust label definition using percents

    // Adjust for DPI
    QPaintDeviceMetrics metrics(printer);
    double scaleX = metrics.logicalDpiX() / 72.0;
    double scaleY = metrics.logicalDpiY() / 72.0;
    defn.dpiAdjust(scaleX, scaleY);

    // Check that paper size matches
    if (fabs(metrics.width() - defn.pageWidth) > 2.0 ||
	fabs(metrics.height() - defn.pageHeight) > 2.0) {
	QString message = tr("The paper size chosen doesn't match the size\n"
	    "used for the labels selected.");
	QMessageBox::critical(NULL, tr("Paper Size Error"), message);
	return;
    }

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    QPainter p;
    p.begin(printer);
    if (defn.scale != 1.0) p.scale(defn.scale, defn.scale);

    int skip = 0;
    unsigned int pos = 0;
    unsigned int pages = 0;
    fixed xOffset = defn.labelWidth + defn.horizontalGap;
    fixed yOffset = defn.labelHeight + defn.verticalGap;
    while (pos < cheque_ids.size()) {
	// Draw one page
	for (int row = 0; row < defn.rows; ++row) {
	    for (int column = 0; column < defn.columns; ++column) {
		if (skip > 0) { --skip; continue; }
		if (pos >= cheque_ids.size()) continue;

		Id cheque_id = cheque_ids[pos];
		Cheque cheque;
		_quasar->db()->lookup(cheque_id, cheque);

		Card card;
		_quasar->db()->lookup(cheque.cardId(), card);

		Account account;
		_quasar->db()->lookup(cheque.accountId(), account);

		QString date = DateValcon().format(cheque.postDate());
		QString amount = MoneyValcon().format(cheque.total());
		QString name = "";
		QString number = card.number();
		QString address = cheque.address();
		QStringList detailLeft;
		QStringList detailRight;

		// If no address, default from card
		if (address.isEmpty())
		    address = card.address();

		// Use first line of address for name or card name if blank
		QStringList lines = QStringList::split("\n", address);
		if (lines.count() > 0)
		    name = lines[0];
		else
		    name = card.nameFL();

		detailLeft << name;
		detailRight << date;
		unsigned int i;
		unsigned int size;
		switch (cheque.type()) {
		case Cheque::ACCOUNT:
		    size = cheque.accounts().size();
		    for (i = 1; i < size; ++i) {
			const AccountLine& line = cheque.accounts()[i];
			Account account;
			_quasar->db()->lookup(line.account_id, account);
			QString amount = MoneyValcon().format(line.amount);
			detailLeft << account.name();
			detailRight << amount;
		    }
		    break;
		case Cheque::VENDOR:
		    size = cheque.payments().size();
		    for (i = 0; i < size; ++i) {
			const PaymentLine& line = cheque.payments()[i];
			Gltx gltx;
			_quasar->db()->lookup(line.gltx_id, gltx);
			QString text = MoneyValcon().format(fabs(line.amount));
			QString desc = gltx.dataTypeName() + " #" +
			    gltx.number();
			if (gltx.dataType() == DataObject::RECEIVE) {
			    Receive receive;
			    _quasar->db()->lookup(line.gltx_id, receive);
			    if (!receive.reference().isEmpty())
				desc += " (" + tr("Vendor Invoice #") +
				    receive.reference() + ")";
			}

			detailLeft << desc;
			detailRight << text;
		    }

		    detailLeft << "";
		    detailRight << "";
		    detailLeft << account.name();
		    detailRight << amount;
		    break;
		case Cheque::CUSTOMER:
		    size = cheque.payments().size();
		    for (i = 0; i < size; ++i) {
			const PaymentLine& line = cheque.payments()[i];
			Gltx gltx;
			_quasar->db()->lookup(line.gltx_id, gltx);
			QString text = MoneyValcon().format(fabs(line.amount));
			QString desc = gltx.dataTypeName() + " #" +
			    gltx.number();
			if (gltx.dataType() == DataObject::INVOICE) {
			    Invoice invoice;
			    _quasar->db()->lookup(line.gltx_id, invoice);
			    if (!invoice.reference().isEmpty())
				desc += " (" + tr("Customer Invoice #") +
				    invoice.reference() + ")";
			}

			detailLeft << desc;
			detailRight << text;
		    }

		    detailLeft << "";
		    detailRight << "";
		    detailLeft << account.name();
		    detailRight << amount;
		    break;
		}

		TextMap mapping;
		mapping["number"] = cheque.number();
		mapping["memo"] = cheque.memo();
		mapping["date"] = date;
		mapping["amount"] = amount;
		mapping["text"] = moneyText(cheque.total());
		mapping["name"] = name;
		mapping["card_num"] = number;
		mapping["address"] = address;
		mapping["detailLeft"] = detailLeft.join("\n");
		mapping["detailRight"] = detailRight.join("\n");

		fixed x = defn.leftMargin + column * xOffset;
		fixed y = defn.topMargin + row * yOffset;
		defn.draw(&p, x, y, mapping);

		++pos;
	    }
	}

	// End the page
	if (pos < cheque_ids.size()) {
	    ++pages;
	    printer->newPage();
	}
    }

    p.end();
    delete printer;

    QApplication::restoreOverrideCursor();

    QString message = tr("Were the cheques printed properly?");
    int choice = QMessageBox::information(this, tr("Finished?"), message,
					  tr("Yes"), tr("No"));
    if (choice != 0)
	return;

    for (unsigned int i = 0; i < cheque_ids.size(); ++i) {
	Cheque orig, cheque;
	_quasar->db()->lookup(cheque_ids[i], cheque);
	orig = cheque;
	cheque.setPrinted(true);
	_quasar->db()->update(orig, cheque);
    }

    close();
}

void
ChequePrint::loadCheques()
{
    ChequeSelect conditions;
    conditions.unprinted = !_printed->isChecked();
    vector<Cheque> cheques;
    _quasar->db()->select(cheques, conditions);

    _cheques->clear();
    ListViewItem* after = NULL;
    for (unsigned int i = 0; i < cheques.size(); ++i) {
	const Cheque& cheque = cheques[i];
	if (cheque.accountId() != _account->getId()) continue;

	Card card;
	_quasar->db()->lookup(cheque.cardId(), card);

	QString name = card.nameFL();
	if (card.id() == INVALID_ID) {
	    QStringList lines = QStringList::split("\n", cheque.address());
	    if (lines.count() > 0)
		name = lines[0];
	}

	ListViewItem* item = new ListViewItem(_cheques, after, cheque.id());
	after = item;
	item->setValue(0, cheque.number());
	item->setValue(1, name);
	item->setValue(2, cheque.postDate());
	item->setValue(3, cheque.total());
	item->setValue(4, cheque.printed());
    }
}
