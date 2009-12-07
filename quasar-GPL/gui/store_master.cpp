// $Id: store_master.cpp,v 1.18 2005/03/15 15:34:35 bpepers Exp $
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

#include "store_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "store_select.h"
#include "line_edit.h"
#include "lookup_edit.h"
#include "company_lookup.h"
#include "gltx_lookup.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qmessagebox.h>

StoreMaster::StoreMaster(MainWindow* main, Id store_id)
    : DataWindow(main, "StoreMaster", store_id)
{
    _helpSource = "store_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(30, _frame);
    nameLabel->setBuddy(_name);

    QLabel* numberLabel = new QLabel(tr("Number:"), _frame);
    _number = new LineEdit(5, _frame);
    numberLabel->setBuddy(_number);

    QLabel* contactLabel = new QLabel(tr("&Contact:"), _frame);
    _contact = new LineEdit(30, _frame);
    contactLabel->setBuddy(_contact);

    QLabel* companyLabel = new QLabel(tr("Company:"), _frame);
    _company = new LookupEdit(new CompanyLookup(main, _frame), _frame);
    companyLabel->setBuddy(_company);

    _has_addr = new QCheckBox(tr("Has own address?"), _frame);
    connect(_has_addr, SIGNAL(toggled(bool)), SLOT(slotHasAddrChanged()));

    _addr = new QGroupBox(tr("Address"), _frame);
    QGridLayout* grid1 = new QGridLayout(_addr, 7, 4,
					 _addr->frameWidth() * 2);
    grid1->addRowSpacing(0, _addr->fontMetrics().height());
    grid1->setColStretch(2, 1);

    QLabel* streetLabel = new QLabel(tr("Street:"), _addr);
    _street = new LineEdit(_addr);
    _street->setLength(60);
    streetLabel->setBuddy(_street);

    _street2 = new LineEdit(_addr);
    _street2->setLength(60);

    QLabel* cityLabel = new QLabel(tr("City:"), _addr);
    _city = new LineEdit(_addr);
    _city->setLength(20);
    cityLabel->setBuddy(_city);

    QLabel* provLabel = new QLabel(tr("Prov/State:"), _addr);
    _province = new LineEdit(_addr);
    _province->setLength(20);
    provLabel->setBuddy(_province);

    QLabel* postalLabel = new QLabel(tr("Postal/Zip:"), _addr);
    _postal = new LineEdit(_addr);
    _postal->setLength(20);
    postalLabel->setBuddy(_postal);

    QLabel* countryLabel = new QLabel(tr("Country:"), _addr);
    _country = new LineEdit(_addr);
    _country->setLength(20);
    countryLabel->setBuddy(_country);

    QLabel* phoneLabel = new QLabel(tr("Phone #:"), _addr);
    _phone_num = new LineEdit(20, _addr);
    phoneLabel->setBuddy(_phone_num);

    QLabel* phone2Label = new QLabel(tr("Phone2 #:"), _addr);
    _phone2_num = new LineEdit(20, _addr);
    phone2Label->setBuddy(_phone2_num);

    QLabel* faxLabel = new QLabel(tr("Fax #:"), _addr);
    _fax_num = new LineEdit(20, _addr);
    faxLabel->setBuddy(_fax_num);

    QLabel* emailLabel = new QLabel(tr("Email:"), _addr);
    _email = new LineEdit(_addr);
    _email->setLength(60);
    emailLabel->setBuddy(_email);

    QLabel* webLabel = new QLabel(tr("Web Page:"), _addr);
    _web_page = new LineEdit(_addr);
    _web_page->setLength(60);
    webLabel->setBuddy(_web_page);

    grid1->addWidget(streetLabel, 1, 0);
    grid1->addMultiCellWidget(_street, 1, 1, 1, 4, AlignLeft | AlignVCenter);
    grid1->addMultiCellWidget(_street2, 2, 2, 1, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(cityLabel, 3, 0);
    grid1->addWidget(_city, 3, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(provLabel, 3, 3);
    grid1->addWidget(_province, 3, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(postalLabel, 4, 0);
    grid1->addWidget(_postal, 4, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(countryLabel, 4, 3);
    grid1->addWidget(_country, 4, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(phoneLabel, 5, 0);
    grid1->addWidget(_phone_num, 5, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(phone2Label, 5, 3);
    grid1->addWidget(_phone2_num, 5, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(faxLabel, 6, 0);
    grid1->addWidget(_fax_num, 6, 1, AlignLeft | AlignVCenter);
    grid1->addWidget(emailLabel, 7, 0);
    grid1->addMultiCellWidget(_email, 7, 7, 1, 4, AlignLeft | AlignVCenter);
    grid1->addWidget(webLabel, 8, 0);
    grid1->addMultiCellWidget(_web_page, 8, 8, 1, 4, AlignLeft | AlignVCenter);

    _can_sell = new QCheckBox(tr("Can sell from store?"), _frame);

    QLabel* transferLabel = new QLabel(tr("Year End Transfer:"), _frame);
    _transfer = new LookupEdit(new GltxLookup(main, _frame), _frame);
    _transfer->setFocusPolicy(NoFocus);

    QGridLayout* grid = new QGridLayout(_frame, 1, 2);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(1, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 1, 0);
    grid->addWidget(_number, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(contactLabel, 2, 0);
    grid->addWidget(_contact, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(companyLabel, 3, 0);
    grid->addWidget(_company, 3, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_has_addr, 4, 4, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_addr, 5, 5, 0, 1);
    grid->addMultiCellWidget(_can_sell, 6, 6, 0, 1, AlignLeft | AlignVCenter);
    grid->addWidget(transferLabel, 7, 0);
    grid->addWidget(_transfer, 7, 1, AlignLeft | AlignVCenter);

    setCaption(tr("Store Master"));
    finalize();
}

StoreMaster::~StoreMaster()
{
}

void
StoreMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
StoreMaster::newItem()
{
    Store blank;
    _orig = blank;
    _orig.setNumber("#");

    _curr = _orig;
    _firstField = _name;
}

void
StoreMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("#");
    _curr.setYearEndTransferId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
StoreMaster::fileItem()
{
    QString number = _curr.number();
    if (number != "" && _orig.number() != number) {
	vector<Store> stores;
	StoreSelect conditions;
	conditions.number = number;
	_quasar->db()->select(stores, conditions);
	if (stores.size() != 0) {
	    QString message = tr("This store number is already used\n"
		"for another store. Are you sure\n"
		"you want to file this store?");
	    int ch = QMessageBox::warning(this, tr("Warning"), message,
					  QMessageBox::No, QMessageBox::Yes);
	    if (ch != QMessageBox::Yes)
		return false;
	}
    }

    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    // TODO: ask to clone locations and item info from another store

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
StoreMaster::deleteItem()
{
    bool ok = _quasar->db()->remove(_curr);
    return ok;
}

void
StoreMaster::restoreItem()
{
    _curr = _orig;
}

void
StoreMaster::cloneItem()
{
    StoreMaster* clone = new StoreMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
StoreMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
StoreMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _number->setText(_curr.number());
    _contact->setText(_curr.contact());
    _company->setId(_curr.companyId());
    _has_addr->setChecked(_curr.hasAddress());
    _street->setText(_curr.street());
    _street2->setText(_curr.street2());
    _city->setText(_curr.city());
    _province->setText(_curr.province());
    _country->setText(_curr.country());
    _postal->setText(_curr.postal());
    _phone_num->setText(_curr.phoneNumber());
    _phone2_num->setText(_curr.phone2Number());
    _fax_num->setText(_curr.faxNumber());
    _email->setText(_curr.email());
    _web_page->setText(_curr.webPage());
    _can_sell->setChecked(_curr.canSell());
    _transfer->setId(_curr.yearEndTransferId());
    _inactive->setChecked(!_curr.isActive());

    slotHasAddrChanged();
}

// Set the data object from the widgets.
void
StoreMaster::widgetToData()
{
    _curr.setName(_name->text());
    _curr.setNumber(_number->text());
    _curr.setContact(_contact->text());
    _curr.setCompanyId(_company->getId());
    _curr.setHasAddress(_has_addr->isChecked());
    _curr.setStreet(_street->text());
    _curr.setStreet2(_street2->text());
    _curr.setCity(_city->text());
    _curr.setProvince(_province->text());
    _curr.setCountry(_country->text());
    _curr.setPostal(_postal->text());
    _curr.setPhoneNumber(_phone_num->text());
    _curr.setPhone2Number(_phone2_num->text());
    _curr.setFaxNumber(_fax_num->text());
    _curr.setEmail(_email->text());
    _curr.setWebPage(_web_page->text());
    _curr.setCanSell(_can_sell->isChecked());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());
}

void
StoreMaster::slotHasAddrChanged()
{
    _addr->setEnabled(_has_addr->isChecked());
}
