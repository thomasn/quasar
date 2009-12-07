// $Id: employee_master.cpp,v 1.38 2005/02/03 08:19:28 bpepers Exp $
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

#include "employee_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "send_email.h"
#include "company.h"
#include "group.h"
#include "lookup_edit.h"
#include "extra_lookup.h"
#include "group_lookup.h"
#include "extra_select.h"
#include "line_edit.h"
#include "table.h"
#include "extra.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qlineedit.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qinputdialog.h>
#include <qmessagebox.h>

EmployeeMaster::EmployeeMaster(MainWindow* main, Id employee_id)
    : DataWindow(main, "EmployeeMaster", employee_id)
{
    _helpSource = "employee_master.html";

    QPushButton* email = new QPushButton(tr("Email"), _buttons);
    connect(email, SIGNAL(clicked()), SLOT(slotEmail()));

    // Create widgets
    _company = new QCheckBox(tr("&Company?"), _frame);
    connect(_company, SIGNAL(toggled(bool)), this, SLOT(companyChanged(bool)));

    _label1 = new QLabel(_frame);
    _entry1 = new LineEdit(_frame);
    _entry1->setLength(30);
    _label1->setBuddy(_entry1);

    _label2 = new QLabel(_frame);
    _entry2 = new LineEdit(_frame);
    _entry2->setLength(30);
    _label2->setBuddy(_entry2);

    QLabel* numberLabel = new QLabel(tr("Number:"), _frame);
    _number = new LineEdit(_frame);
    _number->setLength(12);
    numberLabel->setBuddy(_number);

    QGroupBox* addr = new QGroupBox(tr("Address"), _frame);
    QGridLayout* addrGrid = new QGridLayout(addr, 7, 4,
					    addr->frameWidth() * 2);
    addrGrid->addRowSpacing(0, addr->fontMetrics().height());
    addrGrid->setColStretch(2, 1);

    QLabel* streetLabel = new QLabel(tr("Street:"), addr);
    _street = new LineEdit(addr);
    _street->setLength(60);
    streetLabel->setBuddy(_street);

    _street2 = new LineEdit(addr);
    _street2->setLength(60);

    QLabel* cityLabel = new QLabel(tr("City:"), addr);
    _city = new LineEdit(addr);
    _city->setLength(20);
    cityLabel->setBuddy(_city);

    QLabel* provLabel = new QLabel(tr("Prov/State:"), addr);
    _province = new LineEdit(addr);
    _province->setLength(20);
    provLabel->setBuddy(_province);

    QLabel* postalLabel = new QLabel(tr("Postal/Zip:"), addr);
    _postal = new LineEdit(addr);
    _postal->setLength(20);
    postalLabel->setBuddy(_postal);

    QLabel* countryLabel = new QLabel(tr("Country:"), addr);
    _country = new LineEdit(addr);
    _country->setLength(20);
    countryLabel->setBuddy(_country);

    QLabel* phoneLabel = new QLabel(tr("Phone #:"), addr);
    _phone_num = new LineEdit(20, addr);
    phoneLabel->setBuddy(_phone_num);

    QLabel* phone2Label = new QLabel(tr("Phone2 #:"), addr);
    _phone2_num = new LineEdit(20, addr);
    phone2Label->setBuddy(_phone2_num);

    QLabel* faxLabel = new QLabel(tr("Fax #:"), addr);
    _fax_num = new LineEdit(20, addr);
    faxLabel->setBuddy(_fax_num);

    QLabel* emailLabel = new QLabel(tr("Email:"), addr);
    _email = new LineEdit(addr);
    _email->setLength(60);
    emailLabel->setBuddy(_email);

    QLabel* webLabel = new QLabel(tr("Web Page:"), addr);
    _web_page = new LineEdit(addr);
    _web_page->setLength(60);
    webLabel->setBuddy(_web_page);

    addrGrid->addWidget(streetLabel, 1, 0);
    addrGrid->addMultiCellWidget(_street, 1, 1, 1, 4, AlignLeft|AlignVCenter);
    addrGrid->addMultiCellWidget(_street2, 2, 2, 1, 4, AlignLeft|AlignVCenter);
    addrGrid->addWidget(cityLabel, 3, 0);
    addrGrid->addWidget(_city, 3, 1, AlignLeft|AlignVCenter);
    addrGrid->addWidget(provLabel, 3, 3);
    addrGrid->addWidget(_province, 3, 4, AlignLeft|AlignVCenter);
    addrGrid->addWidget(postalLabel, 4, 0);
    addrGrid->addWidget(_postal, 4, 1, AlignLeft|AlignVCenter);
    addrGrid->addWidget(countryLabel, 4, 3);
    addrGrid->addWidget(_country, 4, 4, AlignLeft|AlignVCenter);
    addrGrid->addWidget(phoneLabel, 5, 0);
    addrGrid->addWidget(_phone_num, 5, 1, AlignLeft|AlignVCenter);
    addrGrid->addWidget(phone2Label, 5, 3);
    addrGrid->addWidget(_phone2_num, 5, 4, AlignLeft | AlignVCenter);
    addrGrid->addWidget(faxLabel, 6, 0);
    addrGrid->addWidget(_fax_num, 6, 1, AlignLeft | AlignVCenter);
    addrGrid->addWidget(emailLabel, 7, 0);
    addrGrid->addMultiCellWidget(_email, 7, 7, 1, 4, AlignLeft|AlignVCenter);
    addrGrid->addWidget(webLabel, 8, 0);
    addrGrid->addMultiCellWidget(_web_page, 8, 8, 1, 4,AlignLeft|AlignVCenter);

    QTabWidget* tabs = new QTabWidget(_frame);
    QFrame* posTab = new QFrame(tabs);
    QFrame* groupTab = new QFrame(tabs);
    QFrame* extraTab = new QFrame(tabs);
    tabs->addTab(posTab, tr("Point-of-Sale"));
    tabs->addTab(groupTab, tr("Groups"));
    tabs->addTab(extraTab, tr("Extra Info"));

    QLabel* posPasswdLabel = new QLabel(tr("Password:"), posTab);
    _posPasswd = new LineEdit(posTab);
    _posPasswd->setLength(10);
    posPasswdLabel->setBuddy(_posPasswd);

    QLabel* posLevelLabel = new QLabel(tr("Security Level:"), posTab);
    _posLevel = new QComboBox(posTab);
    _posLevel->insertItem("No Access");
    _posLevel->insertItem("Cashier");
    _posLevel->insertItem("Manager");
    _posLevel->insertItem("Supervisor");

    QGridLayout* posGrid = new QGridLayout(posTab);
    posGrid->setSpacing(3);
    posGrid->setMargin(3);
    posGrid->setRowStretch(2, 1);
    posGrid->setColStretch(2, 1);
    posGrid->addWidget(posPasswdLabel, 0, 0);
    posGrid->addWidget(_posPasswd, 0, 1, AlignLeft | AlignVCenter);
    posGrid->addWidget(posLevelLabel, 1, 0);
    posGrid->addWidget(_posLevel, 1, 1, AlignLeft | AlignVCenter);

    _groups = new Table(groupTab);
    _groups->setVScrollBarMode(QScrollView::AlwaysOn);
    _groups->setDisplayRows(2);
    _groups->setLeftMargin(fontMetrics().width("999"));
    connect(_groups, SIGNAL(cellChanged(int,int,Variant)),
	    SLOT(groupCellChanged(int,int,Variant)));

    GroupLookup* groupLookup = new GroupLookup(_main, this, Group::EMPLOYEE);
    new LookupColumn(_groups, tr("Group Name"), 20, groupLookup);
    new LookupEditor(_groups, 0, new LookupEdit(groupLookup, _groups));

    QGridLayout* groupGrid = new QGridLayout(groupTab);
    groupGrid->setSpacing(6);
    groupGrid->setMargin(3);
    groupGrid->setRowStretch(0, 1);
    groupGrid->setColStretch(0, 1);
    groupGrid->addWidget(_groups, 0, 0);

    _extra = new Table(extraTab);
    _extra->setVScrollBarMode(QScrollView::AlwaysOn);
    _extra->setDisplayRows(2);
    _extra->setLeftMargin(fontMetrics().width("999"));
    connect(_extra, SIGNAL(focusNext(bool&,int&,int&,int)),
	    SLOT(extraFocusNext(bool&,int&,int&,int)));

    ExtraLookup* extraLookup = new ExtraLookup(_main, this, "Employee");

    new LookupColumn(_extra, tr("Data Name"), 30, extraLookup);
    new TextColumn(_extra, tr("Value"), 30, AlignLeft);

    new LineEditor(_extra, 1, new LineEdit(_extra));

    QPushButton* createData = new QPushButton(tr("Create Data"), extraTab);
    QPushButton* deleteData = new QPushButton(tr("Delete Data"), extraTab);
    QPushButton* renameData = new QPushButton(tr("Rename Data"), extraTab);

    connect(createData, SIGNAL(clicked()), SLOT(slotCreateData()));
    connect(deleteData, SIGNAL(clicked()), SLOT(slotDeleteData()));
    connect(renameData, SIGNAL(clicked()), SLOT(slotRenameData()));

    QGridLayout* extraGrid = new QGridLayout(extraTab);
    extraGrid->setSpacing(6);
    extraGrid->setMargin(3);
    extraGrid->setRowStretch(0, 1);
    extraGrid->setColStretch(0, 1);
    extraGrid->addMultiCellWidget(_extra, 0, 0, 0, 3);
    extraGrid->addWidget(createData, 1, 1);
    extraGrid->addWidget(deleteData, 1, 2);
    extraGrid->addWidget(renameData, 1, 3);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(2, 1);
    grid->addColSpacing(2, 10);
    grid->addWidget(_company, 0, 0);
    grid->addWidget(_label1, 1, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_entry1, 1, 1, AlignLeft | AlignVCenter);
    grid->addWidget(_label2, 2, 0, AlignLeft | AlignVCenter);
    grid->addWidget(_entry2, 2, 1, AlignLeft | AlignVCenter);
    grid->addWidget(numberLabel, 1, 3, AlignLeft | AlignVCenter);
    grid->addWidget(_number, 1, 4, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(addr, 3, 3, 0, 4);
    grid->addMultiCellWidget(tabs, 4, 4, 0, 4);

    setCaption(tr("Employee Master"));
    finalize();
}

EmployeeMaster::~EmployeeMaster()
{
}

void
EmployeeMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _company;
}

void
EmployeeMaster::newItem()
{
    Employee blank;
    _orig = blank;
    _orig.setCompany(false);

    _curr = _orig;
    _firstField = _company;
}

void
EmployeeMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _curr.setNumber("");
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
}

bool
EmployeeMaster::fileItem()
{
    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
EmployeeMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
EmployeeMaster::restoreItem()
{
    _curr = _orig;
}

void
EmployeeMaster::cloneItem()
{
    EmployeeMaster* clone = new EmployeeMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
EmployeeMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
EmployeeMaster::dataToWidget()
{
    _company->setChecked(_curr.company());
    _companyName = _curr.lastName();
    _lastName = _curr.lastName();
    _firstName = _curr.firstName();
    _contact = _curr.contact();
    if (_curr.company()) {
	_entry1->setText(_companyName);
	_entry2->setText(_contact);
    } else {
	_entry1->setText(_lastName);
	_entry2->setText(_firstName);
    }
    _number->setText(_curr.number());
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
    _posPasswd->setText(_curr.password());
    _posLevel->setCurrentItem(_curr.posLevel());
    _inactive->setChecked(!_curr.isActive());

    // Set groups
    _groups->clear();
    unsigned int i;
    for (i = 0; i < _curr.groups().size(); ++i) {
	VectorRow* row = new VectorRow(_groups->columns());
	row->setValue(0, _curr.groups()[i]);
	_groups->appendRow(row);
    }
    _groups->appendRow(new VectorRow(_groups->columns()));

    // Extra info
    _extra->clear();
    ExtraSelect extraSelect;
    extraSelect.activeOnly = true;
    extraSelect.table = "Employee";
    vector<Extra> extras;
    _quasar->db()->select(extras, extraSelect);
    for (i = 0; i < extras.size(); ++i) {
	Id extra_id = extras[i].id();
	QString value = _curr.value(extra_id);

	VectorRow* row = new VectorRow(_extra->columns());
	row->setValue(0, extra_id);
	row->setValue(1, value);
	_extra->appendRow(row);
    }
    if (_extra->rows() > 0)
	_extra->setCurrentCell(0, 1);

    companyChanged(_company->isChecked());
}

// Set the data object from the widgets.
void
EmployeeMaster::widgetToData()
{
    _curr.setCompany(_company->isChecked());
    _curr.setLastName(_entry1->text());
    if (_curr.company()) {
	_curr.setContact(_entry2->text());
	_curr.setFirstName("");
    } else {
	_curr.setContact("");
	_curr.setFirstName(_entry2->text());
    }
    _curr.setNumber(_number->text());
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
    _curr.setPassword(_posPasswd->text());
    _curr.setPosLevel(_posLevel->currentItem());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    _curr.groups().clear();
    int row;
    for (row = 0; row < _groups->rows(); ++row) {
	Id group_id = _groups->cellValue(row, 0).toId();
	if (group_id != INVALID_ID)
	    _curr.groups().push_back(group_id);
    }

    _curr.extra().clear();
    for (row = 0; row < _extra->rows(); ++row) {
	Id extra_id = _extra->cellValue(row, 0).toId();
	QString value = _extra->cellValue(row, 1).toString();
	_curr.setValue(extra_id, value);
    }
}

void
EmployeeMaster::companyChanged(bool company)
{
    if (company) {
	_label1->setText(tr("Company &Name:"));
	_label2->setText(tr("Contact:"));
	_lastName = _entry1->text();
	_firstName = _entry2->text();
	_entry1->setText(_companyName);
	_entry2->setText(_contact);
    } else {
	_label1->setText(tr("Last &Name:"));
	_label2->setText(tr("&First Name:"));
	_companyName = _entry1->text();
	_contact = _entry2->text();
	_entry1->setText(_lastName);
	_entry2->setText(_firstName);
    }
}

void
EmployeeMaster::groupCellChanged(int row, int col, Variant)
{
    // If not blank and last row, append row
    if (row == _groups->rows() - 1 && col == 0) {
	Id group_id = _groups->cellValue(row, col).toId();
	if (group_id != INVALID_ID)
	    _groups->appendRow(new VectorRow(_groups->columns()));
    }
}

void
EmployeeMaster::extraFocusNext(bool&, int& newRow, int& newCol, int type)
{
    int row = _extra->currentRow();
    int col = _extra->currentColumn();

    if (type == Table::MoveNext && col == 1) {
	newCol = 1;
    } else if (type == Table::MovePrev && col == 1) {
	newRow = row - 1;
	newCol = 1;
    }
}

void
EmployeeMaster::slotCreateData()
{
    bool ok = false;
    QString message = tr("Enter the new data name:");
    QString text = QInputDialog::getText(tr("Create Data"), message,
					 QLineEdit::Normal, QString::null,
					 &ok, this);
    if (!ok || text.isEmpty()) return;

    Extra extra;
    extra.setTable("Employee");
    extra.setName(text);
    if (!_quasar->db()->create(extra)) {
	QMessageBox::critical(this, tr("Error"),
			      tr("Creating new data failed."));
	return;
    }

    VectorRow* row = new VectorRow(_extra->columns());
    row->setValue(0, extra.id());
    row->setValue(1, "");
    _extra->appendRow(row);
}

void
EmployeeMaster::slotDeleteData()
{
    int row = _extra->currentRow();
    if (row == -1) return;

    Id extra_id = _extra->cellValue(row, 0).toId();
    if (extra_id == INVALID_ID) return;

    Extra extra;
    if (!_quasar->db()->lookup(extra_id, extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Data lookup failed."));
	return;
    }

    QString message = tr("Are you sure you want to delete the\n"
			 "%1 data?  It will be removed from\n"
			 "all items.").arg(extra.name());
    int choice = QMessageBox::warning(this, tr("Delete?"), message,
				      QMessageBox::No, QMessageBox::Yes);
    if (choice != QMessageBox::Yes)
	return;

    if (!_quasar->db()->remove(extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Deleting data failed."));
	return;
    }

    _extra->deleteRow(row);
}

void
EmployeeMaster::slotRenameData()
{
    int row = _extra->currentRow();
    if (row == -1) return;

    Id extra_id = _extra->cellValue(row, 0).toId();
    if (extra_id == INVALID_ID) return;

    Extra extra;
    if (!_quasar->db()->lookup(extra_id, extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Data lookup failed."));
	return;
    }

    bool ok = false;
    QString message = tr("Enter the new name for %1: ").arg(extra.name());
    QString text = QInputDialog::getText(tr("Rename Data"), message,
					 QLineEdit::Normal, QString::null,
					 &ok, this);
    if (!ok || text.isEmpty()) return;

    Extra orig = extra;
    extra.setName(text);
    if (!_quasar->db()->update(orig, extra)) {
	QMessageBox::critical(this, tr("Error"), tr("Renaming data failed."));
	return;
    }

    _extra->setCellValue(row, 0, extra.id());
}

void
EmployeeMaster::slotEmail()
{
    if (!saveItem(true)) return;

    Company company;
    _quasar->db()->lookup(company);

    SendEmail* email = new SendEmail(_main);
    email->setCard(_curr.id());
    email->show();
}
