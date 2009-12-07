// $Id: data_import.cpp,v 1.56 2005/06/11 02:51:31 bpepers Exp $
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

#include "data_import.h"
#include "object_cache.h"
#include "server_config.h"
#include "quasar_misc.h"

#include "account.h"
#include "account_select.h"
#include "adjust_reason.h"
#include "adjust_reason_select.h"
#include "card_adjust.h"
#include "charge.h"
#include "charge_select.h"
#include "company.h"
#include "company_select.h"
#include "customer.h"
#include "customer_select.h"
#include "customer_type.h"
#include "customer_type_select.h"
#include "dept.h"
#include "dept_select.h"
#include "discount.h"
#include "discount_select.h"
#include "employee.h"
#include "employee_select.h"
#include "expense.h"
#include "expense_select.h"
#include "extra.h"
#include "extra_select.h"
#include "general.h"
#include "group.h"
#include "group_select.h"
#include "invoice.h"
#include "item.h"
#include "item_select.h"
#include "item_price.h"
#include "item_price_select.h"
#include "label_batch.h"
#include "location.h"
#include "location_select.h"
#include "order.h"
#include "order_select.h"
#include "order_template.h"
#include "order_template_select.h"
#include "pat_group.h"
#include "pat_group_select.h"
#include "personal.h"
#include "personal_select.h"
#include "price_batch.h"
#include "security_type.h"
#include "security_type_select.h"
#include "slip.h"
#include "slip_select.h"
#include "station.h"
#include "station_select.h"
#include "store.h"
#include "store_select.h"
#include "subdept.h"
#include "subdept_select.h"
#include "tax.h"
#include "tax_select.h"
#include "tender.h"
#include "tender_select.h"
#include "term.h"
#include "term_select.h"
#include "user.h"
#include "vendor.h"
#include "vendor_select.h"

#include <qapplication.h>
#include <qfile.h>

DataImport::DataImport(const CompanyDefn& company, QObject* parent)
    : QObject(parent, "DataImport"), _company(company), _db(NULL)
{
    _count = 1;
    _errors = 0;
}

DataImport::~DataImport()
{
}

bool
DataImport::processFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) {
	QString message = tr("Can't open import file: %1").arg(filePath);
	logError(message);
	return false;
    }

    QString errorMsg;
    int errorLine = 0;
    int errorCol = 0;
    if (!_doc.setContent(&file, &errorMsg, &errorLine, &errorCol)) {
	QString message = tr("Error processing the import file on line\n"
			     "%1, column %2:\n\n%3").arg(errorLine)
			     .arg(errorCol).arg(errorMsg);
	logError(message);
	return false;
    }

    if (_doc.doctype().name() != "IMPORT") {
	QString message = tr("The import file is not the right type of\n"
			     "XML file for Quasar to use.");
	logError(message);
	return false;
    }

    Driver* driver = Driver::getDriver(_company.dbType());
    if (driver == NULL) {
	QString message = tr("Can't create driver: %1").arg(_company.dbType());
	logError(message);
	return false;
    }

    Connection* connection = driver->allocConnection();
    if (!connection->dbaConnect(_company.database())) {
	QString message = tr("Can't connect to company");
	logError(message);
	return false;
    }

    ServerConfig config;
    config.load();

    QuasarDB db(connection);
    db.setUsername("import");
    db.setSystemId(config.systemId);
    _db = &db;

    _count = 1;
    _errors = 0;
    _type = "";
    _name = "";

    QDomElement root = _doc.documentElement();
    unsigned int i;
    for (i = 0; i < root.childNodes().count(); ++i) {
	QDomNode node = root.childNodes().item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	QString tag = e.tagName();
	QString text = e.text();
	QDomNodeList nodes = e.childNodes();

	if (tag == "account" || tag == "accountInsert") {
	    if (!importAccount(nodes, false)) break;
	} else if (tag == "accountUpdate") {
	    if (!importAccount(nodes, true)) break;
	} else if (tag == "accountDelete") {
	    if (!importAccountDelete(nodes)) break;
	} else if (tag == "adjustReason" || tag == "adjustReasonInsert") {
	    if (!importAdjustReason(nodes, false)) break;
	} else if (tag == "adjustReasonUpdate") {
	    if (!importAdjustReason(nodes, true)) break;
	} else if (tag == "charge") {
	    if (!importCharge(nodes, false)) break;
	} else if (tag == "company" || tag == "companyInsert") {
	    if (!importCompany(nodes, false)) break;
	} else if (tag == "companyUpdate") {
	    if (!importCompany(nodes, true)) break;
	} else if (tag == "cost") {
	    if (!importPrice(nodes, true)) break;
	} else if (tag == "customer" || tag == "customerInsert") {
	    if (!importCustomer(nodes, false)) break;
	} else if (tag == "customerUpdate") {
	    if (!importCustomer(nodes, true)) break;
	} else if (tag == "customerType") {
	    if (!importCustomerType(nodes, false)) break;
	} else if (tag == "dept" || tag == "deptInsert") {
	    if (!importDept(nodes, false)) break;
	} else if (tag == "deptUpdate") {
	    if (!importDept(nodes, true)) break;
	} else if (tag == "discount") {
	    if (!importDiscount(nodes, false)) break;
	} else if (tag == "employee" || tag == "employeeInsert") {
	    if (!importEmployee(nodes, false)) break;
	} else if (tag == "employeeUpdate") {
	    if (!importEmployee(nodes, true)) break;
	} else if (tag == "expense" || tag == "expenseInsert") {
	    if (!importExpense(nodes, false)) break;
	} else if (tag == "expenseUpdate") {
	    if (!importExpense(nodes, true)) break;
	} else if (tag == "extra") {
	    if (!importExtra(nodes)) break;
	} else if (tag == "group") {
	    if (!importGroup(nodes)) break;
	} else if (tag == "item" || tag ==  "itemInsert") {
	    if (!importItem(nodes, false)) break;
	} else if (tag == "itemUpdate") {
	    if (!importItem(nodes, true)) break;
	} else if (tag == "itemDelete") {
	    if (!importItemDelete(nodes)) break;
	} else if (tag == "labelBatch") {
	    if (!importLabelBatch(nodes)) break;
	} else if (tag == "location") {
	    if (!importLocation(nodes, false)) break;
	} else if (tag == "order" || tag == "orderInsert") {
	    if (!importOrder(nodes, false)) break;
	} else if (tag == "orderUpdate") {
	    if (!importOrder(nodes, true)) break;
	} else if (tag == "orderTemplate" || tag == "orderTemplateInsert") {
	    if (!importTemplate(nodes, false)) break;
	} else if (tag == "orderTemplateUpdate") {
	    if (!importTemplate(nodes, true)) break;
	} else if (tag == "patgroup" || tag == "patgroupInsert") {
	    if (!importPatGroup(nodes, false)) break;
	} else if (tag == "patgroupUpdate") {
	    if (!importPatGroup(nodes, true)) break;
	} else if (tag == "patgroupDelete") {
	    if (!importPatGroupDelete(nodes)) break;
	} else if (tag == "personal" || tag == "personalInsert") {
	    if (!importPersonal(nodes, false)) break;
	} else if (tag == "personalUpdate") {
	    if (!importPersonal(nodes, true)) break;
	} else if (tag == "price") {
	    if (!importPrice(nodes, false)) break;
	} else if (tag == "priceBatch") {
	    if (!importPriceBatch(nodes)) break;
	} else if (tag == "priceChange") {
	    if (!importPriceChange(nodes)) break;
	} else if (tag == "securityType" || tag == "securityTypeInsert") {
	    if (!importSecurityType(nodes, false)) break;
	} else if (tag == "securityTypeUpdate") {
	    if (!importSecurityType(nodes, true)) break;
	} else if (tag == "slip" || tag == "slipInsert") {
	    if (!importSlip(nodes, false)) break;
	} else if (tag == "slipUpdate") {
	    if (!importSlip(nodes, true)) break;
	} else if (tag == "slipDelete") {
	    if (!importSlipDelete(nodes)) break;
	} else if (tag == "station" || tag == "stationInsert") {
	    if (!importStation(nodes, false)) break;
	} else if (tag == "stationUpdate") {
	    if (!importStation(nodes, true)) break;
	} else if (tag == "store" || tag == "storeInsert") {
	    if (!importStore(nodes, false)) break;
	} else if (tag == "storeUpdate") {
	    if (!importStore(nodes, true)) break;
	} else if (tag == "subdept" || tag == "subdeptInsert") {
	    if (!importSubdept(nodes, false)) break;
	} else if (tag == "subdeptUpdate") {
	    if (!importSubdept(nodes, true)) break;
	} else if (tag == "subdeptDelete") {
	    if (!importSubdeptDelete(nodes)) break;
	} else if (tag == "tax" || tag == "taxInsert") {
	    if (!importTax(nodes, false)) break;
	} else if (tag == "taxUpdate") {
	    if (!importTax(nodes, true)) break;
	} else if (tag == "tender" || tag == "tenderInsert") {
	    if (!importTender(nodes, false)) break;
	} else if (tag == "tenderUpdate") {
	    if (!importTender(nodes, true)) break;
	} else if (tag == "term") {
	    if (!importTerm(nodes, false)) break;
	} else if (tag == "user" || tag == "userInsert") {
	    if (!importUser(nodes, false)) break;
	} else if (tag == "userUpdate") {
	    if (!importUser(nodes, true)) break;
	} else if (tag == "vendor" || tag == "vendorInsert") {
	    if (!importVendor(nodes, false)) break;
	} else if (tag == "vendorUpdate") {
	    if (!importVendor(nodes, true)) break;
	} else if (tag == "journalEntry") {
	    if (!importJournalEntry(nodes)) break;
	} else if (tag == "cardAdjustment") {
	    if (!importCardAdjustment(nodes)) break;
	} else if (tag == "customerInvoice") {
	    if (!importCustomerInvoice(nodes)) break;
	} else {
	    _type = tr("Node Tag");
	    _name = tag;
	    logWarning(tr("Unknown tag"));
	}
    }

    // Process account fixups
    for (i = 0; i < _acctFixups.size(); ++i) {
	_type = "Account";
	_name = _acctFixups[i].first;

	Account account;
	findAccount(_name, account);

	Account parent;
	if (findAccount(_acctFixups[i].second, parent)) {
	    Account orig = account;
	    account.setParentId(parent.id());
	    if (!_db->update(orig, account)) {
		logError(tr("Setting parent failed"));
	    }
	} else {
	    logError(tr("Unknown parent account"));
	}
    }

    // For PostgreSQL you need to use vacuum
    if (_company.dbType() == "PostgreSQL")
	connection->execute("vacuum full analyze");

    delete connection;
    return true;
}

void
DataImport::logMessage(const QString& severity, const QString& text)
{
    emit message(_count++, severity, text);
    if (severity == tr("Error")) {
	_errors++;
	qDebug("Error: " + text);
    }
}

void
DataImport::logWarning(const QString& text)
{
    logMessage(tr("Warning"), text);
}

void
DataImport::logError(const QString& text)
{
    logMessage(tr("Error"), text);
}

void
DataImport::logInfo(const QString& text)
{
    logMessage(tr("Information"), text);
}

void
DataImport::logQuasarErrors(const QString& message)
{
    QString* error;
    while ((error = _db->nextError()) != NULL) {
	if (!error->isEmpty())
	    logError(*error);
	delete error;
    }
    logError(message);
}

bool
DataImport::importAccount(QDomNodeList& nodes, bool isUpdate)
{
    Account orig, account;
    QStringList links;

    _type = tr("Account");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findAccount(text, account)) {
		orig = account;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    account.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    account.setName(text);
	    _name = text;
	} else if (tag == "number") {
	    account.setNumber(text);
	} else if (tag == "type") {
	    account.setType(Account::Type(iconvInt(text)));
	} else if (tag == "header") {
	    account.setHeader(iconvBoolean(text));
	} else if (tag == "link") {
	    links.append(text);
	} else if (tag == "retained") {
	    links.append("retained");
	} else if (tag == "historical") {
	    links.append("historical");
	} else if (tag == "parent") {
	    account.setParentId(iconvAccount(text));
	    if (account.parentId() == INVALID_ID)
		_acctFixups.push_back(AccountFixup(account.name(), text));
	} else if (tag == "reconciled") {
	    account.setLastReconciled(iconvDate(text));
	} else if (tag == "nextnum") {
	    account.setNextNumber(iconvInt(text));
	} else if (tag == "group" || tag == "groupInsert") {
	    Id group_id = iconvGroup(text, Group::ACCOUNT);
	    if (group_id != INVALID_ID) {
		bool found = false;
		for (unsigned int i = 0; i < account.groups().size(); ++i) {
		    if (account.groups()[i] == group_id) {
			found = true;
			break;
		    }
		}
		if (!found)
		    account.groups().push_back(group_id);
	    } else {
		logWarning(tr("Unknown group: %1").arg(text));
	    }
	} else if (tag == "groupDelete" && isUpdate) {
	    Id group_id = iconvGroup(text, Group::ACCOUNT);
	    if (group_id != INVALID_ID) {
		bool found = false;
		for (unsigned int i = 0; i < account.groups().size(); ++i) {
		    if (account.groups()[i] == group_id) {
			found = true;
			account.groups().erase(account.groups().begin() + i);
			break;
		    }
		}
		if (!found)
		    logWarning(tr("Group not found: %1").arg(text));
	    } else {
		logWarning(tr("Unknown group: %1").arg(text));
	    }
	} else if (tag == "groupClear" && isUpdate) {
	    account.groups().clear();
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, account)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(account)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    if (account.id() != INVALID_ID && links.count() > 0) {
	Company orig, company;
	_db->lookup(orig);
	company = orig;

	for (unsigned int i = 0; i < links.count(); ++i) {
	    if (links[i] == "retained")
		company.setRetainedEarnings(account.id());
	    else if (links[i] == "historical")
		company.setHistoricalBalancing(account.id());
	    else if (links[i] == "bank")
		company.setBankAccount(account.id());
	    else if (links[i] == "customer")
		company.setCustomerAccount(account.id());
	    else if (links[i] == "vendor")
		company.setVendorAccount(account.id());
	    else if (links[i] == "customer_terms")
		company.setCustomerTermsAcct(account.id());
	    else if (links[i] == "vendor_terms")
		company.setVendorTermsAcct(account.id());
	    else if (links[i] == "deposit")
		company.setDepositAccount(account.id());
	    else if (links[i] == "transfer")
		company.setTransferAccount(account.id());
	    else if (links[i] == "physical")
		company.setPhysicalAccount(account.id());
	    else if (links[i] == "split")
		company.setSplitAccount(account.id());
	    else if (links[i] == "charge")
		company.setChargeAccount(account.id());
	    else if (links[i] == "over_short")
		company.setOverShortAccount(account.id());
	    else {
		QString message = tr("Unknown account link: %1").arg(links[i]);
		logWarning(message);
	    }
	}

	_db->update(orig, company);
    }

    return true;
}

bool
DataImport::importAccountDelete(QDomNodeList& nodes)
{
    _type = tr("Account");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "number" || tag == "name" || tag == "externalId") {
	    _name = text;
	    Account account;
	    if (!findAccount(text, account)) {
		logError(tr("Account not found for delete"));
		continue;
	    }

	    if (!_db->remove(account)) {
		logQuasarErrors(tr("Delete failed"));
	    } else {
		logInfo(tr("Data deleted"));
	    }
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    return true;
}

bool
DataImport::importAdjustReason(QDomNodeList& nodes, bool isUpdate)
{
    AdjustReason orig, reason;

    _type = tr("Reason");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findAdjustReason(text, reason)) {
		orig = reason;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    reason.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    reason.setName(text);
	    _name = text;
	} else if (tag == "number") {
	    reason.setNumber(text);
	} else if (tag == "account") {
	    reason.setAccountId(iconvAccount(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, reason)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(reason)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importCharge(QDomNodeList& nodes, bool isUpdate)
{
    Charge orig, charge;

    _type = tr("Charge");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findCharge(text, charge)) {
		orig = charge;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    charge.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    charge.setName(text);
	    _name = text;
	} else if (tag == "account") {
	    charge.setAccountId(iconvAccount(text));
	} else if (tag == "tax") {
	    charge.setTaxId(iconvTax(text));
	} else if (tag == "includeTax") {
	    charge.setIncludeTax(iconvBoolean(text));
	} else if (tag == "amount") {
	    charge.setAmount(iconvFixed(text));
	} else if (tag == "calcMethod") {
	    if (text == "manual")
		charge.setCalculateMethod(Charge::MANUAL);
	    else if (text == "cost")
		charge.setCalculateMethod(Charge::COST);
	    else if (text == "weight")
		charge.setCalculateMethod(Charge::WEIGHT);
	    else
		logError(tr("Unknown type: %1").arg(text));
	} else if (tag == "allocMethod") {
	    if (text == "none")
		charge.setAllocateMethod(Charge::NONE);
	    else if (text == "cost")
		charge.setAllocateMethod(Charge::COST);
	    else if (text == "weight")
		charge.setAllocateMethod(Charge::WEIGHT);
	    else
		logError(tr("Unknown type: %1").arg(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, charge)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(charge)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importCompany(QDomNodeList& nodes, bool isUpdate)
{
    Company orig, company;

    _type = tr("Company");
    _name = tr(company.name());

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findCompany(text, company)) {
		orig = company;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    company.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    company.setName(text);
	    _name = text;
	} else if (tag == "number") {
	    company.setNumber(text);
	} else if (tag == "street") {
	    company.setStreet(text);
	} else if (tag == "street2") {
	    company.setStreet2(text);
	} else if (tag == "city") {
	    company.setCity(text);
	} else if (tag == "province") {
	    company.setProvince(text);
	} else if (tag == "country") {
	    company.setCountry(text);
	} else if (tag == "postal") {
	    company.setPostal(text);
	} else if (tag == "phone") {
	    company.setPhoneNumber(text);
	} else if (tag == "fax") {
	    company.setFaxNumber(text);
	} else if (tag == "email") {
	    company.setEmail(text);
	} else if (tag == "webPage") {
	    company.setWebPage(text);
	} else if (tag == "store") {
	    company.setDefaultStore(iconvStore(text));
	} else if (tag == "shift") {
	    if (text == "station")
		company.setShiftMethod(Company::BY_STATION);
	    else if (text == "employee")
		company.setShiftMethod(Company::BY_EMPLOYEE);
	    else
		logWarning(tr("Unknown shift: %1").arg(text));
	} else if (tag == "startOfYear") {
	    company.setStartOfYear(iconvDate(text));
	} else if (tag == "closeDate") {
	    company.setCloseDate(iconvDate(text));
	} else if (tag == "lastSC") {
	    company.setLastServiceCharge(iconvDate(text));
	} else if (tag == "safeStore") {
	    company.setSafeStore(iconvStore(text));
	} else if (tag == "safeStation") {
	    company.setSafeStation(iconvStation(text));
	} else if (tag == "safeEmployee") {
	    company.setSafeEmployee(iconvEmployee(text));
	} else if (tag == "rounding") {
	    RoundingRule rounding;
	    rounding.endsIn = e.attribute("endsIn");
	    rounding.addAmt = iconvFixed(e.attribute("addAmt"));
	    company.rounding().push_back(rounding);
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, company))
	    logError(tr("Update failed"));
	else
	    logInfo(tr("Data updated"));
    } else {
	if (!_db->create(company)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importCustomer(QDomNodeList& nodes, bool isUpdate)
{
    Customer orig, customer;

    _type = tr("Customer");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findCustomer(text, customer)) {
		orig = customer;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    customer.setExternalId(iconvFixed(text));
	} else if (tag == "firstName") {
	    customer.setFirstName(text.left(30));
	    _name = customer.name();
	} else if (tag == "lastName") {
	    customer.setLastName(text.left(30));
	    _name = customer.name();
	} else if (tag == "name") {
	    customer.setLastName(text.left(30));
	    _name = customer.name();
	} else if (tag == "number") {
	    customer.setNumber(text);
	    _name = text;
	} else if (tag == "company") {
	    customer.setCompany(iconvBoolean(text));
	} else if (tag == "street") {
	    customer.setStreet(text);
	} else if (tag == "street2") {
	    customer.setStreet2(text);
	} else if (tag == "city") {
	    customer.setCity(text);
	} else if (tag == "province") {
	    customer.setProvince(text);
	} else if (tag == "country") {
	    customer.setCountry(text);
	} else if (tag == "postal") {
	    customer.setPostal(text);
	} else if (tag == "phone") {
	    customer.setPhoneNumber(text);
	} else if (tag == "fax") {
	    customer.setFaxNumber(text);
	} else if (tag == "email") {
	    customer.setEmail(text);
	} else if (tag == "webPage") {
	    customer.setWebPage(text);
	} else if (tag == "contact") {
	    customer.setContact(text);
	} else if (tag == "comments") {
	    customer.setComments(text);
	} else if (tag == "type") {
	    CustomerType type;
	    if (findCustomerType(text, type))
		customer.setFromType(type);
	    else
		logWarning(tr("Unknown type: %1").arg(text));
	} else if (tag == "account") {
	    customer.setAccountId(iconvAccount(text));
	} else if (tag == "terms") {
	    customer.setTermsId(iconvTerms(text));
	} else if (tag == "taxExempt") {
	    customer.setTaxExemptId(iconvTax(text));
	} else if (tag == "creditLimit") {
	    customer.setCreditLimit(iconvFixed(text));
	} else if (tag == "canCharge") {
	    customer.setCanCharge(iconvBoolean(text));
	} else if (tag == "canWithdraw") {
	    customer.setCanWithdraw(iconvBoolean(text));
	} else if (tag == "canPayment") {
	    customer.setCanPayment(iconvBoolean(text));
	} else if (tag == "serviceCharge") {
	    customer.setServiceCharge(iconvFixed(text));
	} else if (tag == "checkWithdrawBalance") {
	    customer.setCheckWithdrawBalance(iconvBoolean(text));
	} else if (tag == "printStmts") {
	    customer.setPrintStatements(iconvBoolean(text));
	} else if (tag == "creditHold") {
	    customer.setCreditHold(iconvBoolean(text));
	} else if (tag == "secondReceipt") {
	    customer.setSecondReceipt(iconvBoolean(text));
	} else if (tag == "group" || tag == "groupInsert") {
	    Id group_id = iconvGroup(text, Group::CUSTOMER);
	    if (group_id != INVALID_ID) {
		bool found = false;
		for (unsigned int i = 0; i < customer.groups().size(); ++i) {
		    if (customer.groups()[i] == group_id) {
			found = true;
			break;
		    }
		}
		if (!found)
		    customer.groups().push_back(group_id);
	    } else {
		logWarning(tr("Unknown group: %1").arg(text));
	    }
	} else if (tag == "groupDelete" && isUpdate) {
	    Id group_id = iconvGroup(text, Group::CUSTOMER);
	    if (group_id != INVALID_ID) {
		bool found = false;
		for (unsigned int i = 0; i < customer.groups().size(); ++i) {
		    if (customer.groups()[i] == group_id) {
			found = true;
			customer.groups().erase(customer.groups().begin() + i);
			break;
		    }
		}
		if (!found)
		    logWarning(tr("Group not found: %1").arg(text));
	    } else {
		logWarning(tr("Unknown group: %1").arg(text));
	    }
	} else if (tag == "groupClear" && isUpdate) {
	    customer.groups().clear();
	} else if (tag == "reference") {
	    customer.references().push_back(text.left(16));
	} else if (tag == "discount" || tag == "discountInsert") {
	    Id discount_id = iconvDiscount(text);
	    if (discount_id != INVALID_ID) {
		bool found = false;
		for (unsigned int i = 0; i < customer.discounts().size(); ++i) {
		    if (customer.discounts()[i] == discount_id) {
			found = true;
			break;
		    }
		}
		if (!found)
		    customer.discounts().push_back(discount_id);
	    } else {
		logWarning(tr("Unknown discount: %1").arg(text));
	    }
	} else if (tag == "discountDelete" && isUpdate) {
	    Id discount_id = iconvDiscount(text);
	    if (discount_id != INVALID_ID) {
		bool found = false;
		for (unsigned int i = 0; i < customer.discounts().size(); ++i) {
		    if (customer.discounts()[i] == discount_id) {
			found = true;
			customer.discounts().erase(customer.discounts().begin() + i);
			break;
		    }
		}
		if (!found)
		    logWarning(tr("Discount not found: %1").arg(text));
	    } else {
		logWarning(tr("Unknown discount: %1").arg(text));
	    }
	} else if (tag == "discountClear" && isUpdate) {
	    customer.discounts().clear();
	} else if (tag == "extra") {
	    Extra extra;
	    if (findExtra("Customer", e.attribute("name"), extra))
		customer.setValue(extra.id(), text);
	    else
		logWarning(tr("Unknown extra: %1,%21")
			   .arg("Customer").arg(e.attribute("name")));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, customer)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(customer)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importCustomerType(QDomNodeList& nodes, bool isUpdate)
{
    CustomerType type, orig;

    _type = tr("Customer Type");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findCustomerType(text, type)) {
		orig = type;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    type.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    type.setName(text);
	    _name = type.name();
	} else if (tag == "account") {
	    type.setAccountId(iconvAccount(text));
	} else if (tag == "terms") {
	    type.setTermsId(iconvTerms(text));
	} else if (tag == "taxExempt") {
	    type.setTaxExemptId(iconvTax(text));
	} else if (tag == "creditLimit") {
	    type.setCreditLimit(iconvFixed(text));
	} else if (tag == "canCharge") {
	    type.setCanCharge(iconvBoolean(text));
	} else if (tag == "canWithdraw") {
	    type.setCanWithdraw(iconvBoolean(text));
	} else if (tag == "canPayment") {
	    type.setCanPayment(iconvBoolean(text));
	} else if (tag == "serviceCharge") {
	    type.setServiceCharge(iconvFixed(text));
	} else if (tag == "checkWithdrawBalance") {
	    type.setCheckWithdrawBalance(iconvBoolean(text));
	} else if (tag == "printStmts") {
	    type.setPrintStatements(iconvBoolean(text));
	} else if (tag == "secondReceipt") {
	    type.setSecondReceipt(iconvBoolean(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, type)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(type)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importDept(QDomNodeList& nodes, bool isUpdate)
{
    Dept orig, dept;

    _type = tr("Dept");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findDept(text, dept)) {
		orig = dept;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    dept.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    dept.setName(text);
	    _name = text;
	} else if (tag == "number") {
	    dept.setNumber(text);
	} else if (tag == "pat_points") {
	    dept.setPatPoints(iconvFixed(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, dept)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(dept)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importDiscount(QDomNodeList& nodes, bool isUpdate)
{
    Discount discount, orig;

    _type = tr("Discount");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findDiscount(text, discount)) {
		orig = discount;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    discount.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    discount.setName(text);
	    _name = text;
	} else if (tag == "type") {
	    if (text == "line") {
		discount.setLineDiscount(true);
		discount.setTxDiscount(false);
	    } else if (text == "transaction") {
		discount.setLineDiscount(false);
		discount.setTxDiscount(true);
	    } else if (text == "both") {
		discount.setLineDiscount(true);
		discount.setTxDiscount(true);
	    } else {
		logError(tr("Unknown type: %1").arg(text));
	    }
	} else if (tag == "account") {
	    discount.setAccountId(iconvAccount(text));
	} else if (tag == "method") {
	    if (text == "percent")
		discount.setMethod(Discount::PERCENT);
	    else if (text == "dollar")
		discount.setMethod(Discount::DOLLAR);
	    else
		logError(tr("Unknown method: %1").arg(text));
	} else if (tag == "amount") {
	    discount.setAmount(iconvFixed(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, discount)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(discount)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importEmployee(QDomNodeList& nodes, bool isUpdate)
{
    Employee orig, employee;
    QStringList links;

    _type = tr("Employee");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findEmployee(text, employee)) {
		orig = employee;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    employee.setExternalId(iconvFixed(text));
	} else if (tag == "firstName") {
	    employee.setFirstName(text);
	    _name = employee.name();
	} else if (tag == "lastName") {
	    employee.setLastName(text);
	    _name = employee.name();
	} else if (tag == "company") {
	    employee.setCompany(iconvBoolean(text));
	} else if (tag == "number") {
	    employee.setNumber(text);
	} else if (tag == "street") {
	    employee.setStreet(text);
	} else if (tag == "street2") {
	    employee.setStreet2(text);
	} else if (tag == "city") {
	    employee.setCity(text);
	} else if (tag == "province") {
	    employee.setProvince(text);
	} else if (tag == "country") {
	    employee.setCountry(text);
	} else if (tag == "postal") {
	    employee.setPostal(text);
	} else if (tag == "phone") {
	    employee.setPhoneNumber(text);
	} else if (tag == "fax") {
	    employee.setFaxNumber(text);
	} else if (tag == "email") {
	    employee.setEmail(text);
	} else if (tag == "webPage") {
	    employee.setWebPage(text);
	} else if (tag == "contact") {
	    employee.setContact(text);
	} else if (tag == "comments") {
	    employee.setComments(text);
	} else if (tag == "posPassword") {
	    employee.setPassword(text);
	} else if (tag == "posLevel") {
	    employee.setPosLevel(iconvInt(text));
	} else if (tag == "extra") {
	    Extra extra;
	    if (findExtra("Employee", e.attribute("name"), extra))
		employee.setValue(extra.id(), text);
	    else
		logWarning(tr("Unknown extra: %1,%21")
			   .arg("Employee").arg(e.attribute("name")));
	} else if (tag == "link") {
	    links.append(text);
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, employee)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(employee)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    if (employee.id() != INVALID_ID && links.count() > 0) {
	Company orig, company;
	_db->lookup(orig);
	company = orig;

	for (unsigned int i = 0; i < links.count(); ++i) {
	    if (links[i] == "safe")
		company.setSafeEmployee(employee.id());
	    else {
		QString msg = tr("Unknown employee link: %1").arg(links[i]);
		logWarning(msg);
	    }
	}

	_db->update(orig, company);
    }

    return true;
}

bool
DataImport::importExpense(QDomNodeList& nodes, bool isUpdate)
{
    Expense expense, orig;

    _type = tr("Expense");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findExpense(text, expense)) {
		orig = expense;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    expense.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    expense.setName(text);
	    _name = text;
	} else if (tag == "number") {
	    expense.setNumber(text);
	} else if (tag == "account") {
	    expense.setAccountId(iconvAccount(text));
	} else if (tag == "tax") {
	    expense.setTaxId(iconvTax(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, expense)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(expense)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importExtra(QDomNodeList& nodes)
{
    Extra extra;

    _type = tr("Extra");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "externalId") {
	    extra.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    extra.setName(text);
	    _name = text;
	} else if (tag == "table") {
	    extra.setTable(text);
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (!_db->create(extra))
	logError(tr("Create failed"));
    else
	logInfo(tr("Data created"));

    return true;
}

bool
DataImport::importGroup(QDomNodeList& nodes)
{
    Group group;

    _type = tr("Group");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "externalId") {
	    group.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    group.setName(text);
	    _name = text;
	} else if (tag == "type") {
	    group.setType(iconvInt(text));
	} else if (tag == "description") {
	    group.setDescription(text);
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (!_db->create(group))
	logError(tr("Create failed"));
    else
	logInfo(tr("Data created"));

    return true;
}

bool
DataImport::importItem(QDomNodeList& nodes, bool isUpdate)
{
    Item orig, item;
    Subdept subdept;
    Store defaultStore;

    _type = tr("Item");
    _name = tr("Unknown");

    // Get list of stores
    StoreSelect select;
    select.activeOnly = true;
    vector<Store> all_stores;
    _db->select(all_stores, select);

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    _name = text;
	    if (findItem(text, item)) {
		orig = item;
	    } else {
		logWarning(tr("Item not found: %1").arg(text));
	    }
	} else if (tag == "externalId") {
	    item.setExternalId(iconvFixed(text));
	} else if (tag == "number" || tag == "numberInsert") {
	    ItemPlu info;
	    info.number = text;
	    info.size = e.attribute("size");

	    bool found = false;
	    for (unsigned int i = 0; i < item.numbers().size(); ++i) {
		if (item.numbers()[i].number == text) {
		    found = true;
		    break;
		}
	    }

	    if (found)
		logWarning(tr("Duplicate number: %1").arg(text));
	    else
		item.numbers().push_back(info);
	    if (_name == tr("Unknown") && !text.isEmpty())
		_name = text;
	} else if (tag == "numberUpdate" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < item.numbers().size(); ++i) {
		if (item.numbers()[i].number == text) {
		    found = true;
		    item.numbers()[i].size = e.attribute("size");
		    break;
		}
	    }
	    if (!found)
		logWarning(tr("Number not found: %1").arg(text));
	} else if (tag == "numberDelete" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < item.numbers().size(); ++i) {
		if (item.numbers()[i].number == text) {
		    found = true;
		    item.numbers().erase(item.numbers().begin() + i);
		    break;
		}
	    }
	    if (!found)
		logWarning(tr("Number not found: %1").arg(text));
	} else if (tag == "numberClear" && isUpdate) {
	    item.numbers().clear();
	    for (unsigned int i = 0; i < item.vendors().size(); ++i) {
		item.vendors()[i].number = "";
	    }
	} else if (tag == "description") {
	    item.setDescription(text);
	    if (_name == tr("Unknown") && !text.isEmpty())
		_name = text;
	} else if (tag == "dept") {
	    item.setDeptId(iconvDept(text));
	} else if (tag == "subdept") {
	    if (findSubdept(text, subdept))
		item.setFromSubdept(subdept);
	    else
		logWarning(tr("Unknown subdept: %1").arg(text));
	} else if (tag == "purchased") {
	    item.setPurchased(iconvBoolean(text));
	    if (!item.isInventoried() || !item.isSold())
		item.setExpenseAccount(INVALID_ID);
	} else if (tag == "sold") {
	    item.setSold(iconvBoolean(text));
	    item.setIncomeAccount(INVALID_ID);
	} else if (tag == "inventoried") {
	    item.setInventoried(iconvBoolean(text));
	    item.setAssetAccount(INVALID_ID);
	} else if (tag == "openDept") {
	    item.setOpenDept(iconvBoolean(text));
	} else if (tag == "sellSize") {
	    item.setSellSize(text);
	} else if (tag == "purchaseSize") {
	    item.setPurchaseSize(text);
	} else if (tag == "expenseAccount") {
	    item.setExpenseAccount(iconvAccount(text));
	} else if (tag == "incomeAccount") {
	    item.setIncomeAccount(iconvAccount(text));
	} else if (tag == "assetAccount") {
	    item.setAssetAccount(iconvAccount(text));
	} else if (tag == "sellTax") {
	    item.setSellTax(iconvTax(text));
	} else if (tag == "purchaseTax") {
	    item.setPurchaseTax(iconvTax(text));
	} else if (tag == "qtyEnforced") {
	    item.setQuantityEnforced(iconvBoolean(text));
	} else if (tag == "qtyDecimals") {
	    item.setQuantityDecimals(iconvBoolean(text));
	} else if (tag == "qtyFromPrice") {
	    item.setQtyFromPrice(iconvBoolean(text));
	} else if (tag == "weighed") {
	    item.setWeighed(iconvBoolean(text));
	} else if (tag == "discountable") {
	    item.setDiscountable(iconvBoolean(text));
	} else if (tag == "discontinued") {
	    item.setDiscontinued(iconvBoolean(text));
	} else if (tag == "deposit") {
	    item.setDeposit(iconvFixed(text));
	} else if (tag == "costIncTax") {
	    item.setCostIncludesTax(iconvBoolean(text));
	} else if (tag == "costIncDep") {
	    item.setCostIncludesDeposit(iconvBoolean(text));
	} else if (tag == "priceIncTax") {
	    item.setPriceIncludesTax(iconvBoolean(text));
	} else if (tag == "priceIncDep") {
	    item.setPriceIncludesDeposit(iconvBoolean(text));
	} else if (tag == "buildQty") {
	    item.setBuildQty(iconvFixed(text));
	} else if (tag == "costDiscount") {
	    item.setCostDiscount(iconvFixed(text));
	} else if (tag == "store") {
	    Store store;
	    if (text.isEmpty()) {
		defaultStore = store;
	    } else {
		if (findStore(text, store))
		    defaultStore = store;
		else
		    logError(tr("Unknown store: " + text));
	    }
	} else if (tag == "min") {
	    vector<Store> stores;
	    if (e.attribute("store").isEmpty()) {
		stores = all_stores;
	    } else if (defaultStore.id() != INVALID_ID) {
		stores.push_back(defaultStore);
	    } else {
		Store store;
		if (findStore(e.attribute("store"), store))
		    stores.push_back(store);
		else
		    logError(tr("Unknown store: " + text));
	    }
	    for (unsigned int i = 0; i < stores.size(); ++i) {
		bool found = false;
		for (unsigned int j = 0; j < item.stores().size(); ++j) {
		    if (item.stores()[j].store_id == stores[i].id()) {
			item.stores()[j].min_qty = iconvFixed(text);
			found = true;
			break;
		    }
		}
		if (!found) {
		    ItemStore info;
		    info.store_id = stores[i].id();
		    info.min_qty = iconvFixed(text);
		    item.stores().push_back(info);
		}
	    }
	} else if (tag == "max") {
	    vector<Store> stores;
	    if (e.attribute("store").isEmpty()) {
		stores = all_stores;
	    } else if (defaultStore.id() != INVALID_ID) {
		stores.push_back(defaultStore);
	    } else {
		Store store;
		if (findStore(e.attribute("store"), store))
		    stores.push_back(store);
		else
		    logError(tr("Unknown store: " + text));
	    }
	    for (unsigned int i = 0; i < stores.size(); ++i) {
		bool found = false;
		for (unsigned int j = 0; j < item.stores().size(); ++j) {
		    if (item.stores()[j].store_id == stores[i].id()) {
			item.stores()[j].max_qty = iconvFixed(text);
			found = true;
			break;
		    }
		}
		if (!found) {
		    ItemStore info;
		    info.store_id = stores[i].id();
		    info.max_qty = iconvFixed(text);
		    item.stores().push_back(info);
		}
	    }
	} else if (tag == "size" || tag == "sizeInsert") {
	    ItemSize info;
	    info.name = text;
	    info.qty = e.attribute("qty", "1").toDouble();
	    info.weight = e.attribute("weight", "0").toDouble();
	    info.multiple = e.attribute("multiple", "1").toDouble();

	    bool found = false;
	    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
		if (item.sizes()[i].name == text) {
		    found = true;
		    break;
		}
	    }

	    if (found)
		logWarning(tr("Duplicate size: %1").arg(text));
	    else
		item.sizes().push_back(info);
	} else if (tag == "sizeUpdate" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
		ItemSize& size = item.sizes()[i];
		if (size.name == text) {
		    found = true;
		    size.qty = e.attribute("qty", "1").toDouble();
		    size.weight = e.attribute("weight", "1").toDouble();
		    size.multiple = e.attribute("multiple", "1").toDouble();
		    break;
		}
	    }
	    if (!found)
		logWarning(tr("Size not found: %1").arg(text));
	} else if (tag == "sizeDelete" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < item.sizes().size(); ++i) {
		ItemSize& size = item.sizes()[i];
		if (size.name == text) {
		    found = true;
		    item.sizes().erase(item.sizes().begin() + i);
		    break;
		}
	    }
	    // TODO: remove size from other places?
	    if (!found)
		logWarning(tr("Size not found: %1").arg(text));
	} else if (tag == "sizeClear" && isUpdate) {
	    item.sizes().clear();
	    item.costs().clear();
	    item.setSellSize("");
	    item.setPurchaseSize("");
	    for (unsigned int i = 0; i < item.numbers().size(); ++i)
		item.numbers()[i].size = "";
	} else if (tag == "vendor" || tag == "vendorInsert") {
	    Vendor vendor;
	    if (findVendor(text, vendor)) {
		ItemVendor info;
		info.vendor_id = vendor.id();
		info.number = e.attribute("number");
		info.size = e.attribute("size");
		item.vendors().push_back(info);
	    } else {
		logWarning(tr("Unknown vendor: %1")
			   .arg(text));
	    }
	} else if (tag == "vendorUpdate" && isUpdate) {
	    Vendor vendor;
	    if (findVendor(text, vendor)) {
		bool found = false;
		for (unsigned int i = 0; i < item.vendors().size(); ++i) {
		    ItemVendor& line = item.vendors()[i];
		    if (line.vendor_id == vendor.id()) {
			found = true;
			line.number = e.attribute("number");
			line.size = e.attribute("size");
			break;
		    }
		}
		if (!found)
		    logWarning(tr("Vendor not found: %1")
			       .arg(text));
	    } else {
		logWarning(tr("Unknown vendor: %1")
			   .arg(text));
	    }
	} else if (tag == "vendorDelete" && isUpdate) {
	    Vendor vendor;
	    if (findVendor(text, vendor)) {
		bool found = false;
		for (unsigned int i = 0; i < item.vendors().size(); ++i) {
		    if (item.vendors()[i].vendor_id == vendor.id()) {
			found = true;
			item.vendors().erase(item.vendors().begin() + i);
			break;
		    }
		}
		if (!found)
		    logWarning(tr("Vendor not found: %1").arg(text));
	    } else {
		logWarning(tr("Unknown vendor: %1").arg(text));
	    }
	} else if (tag == "vendorClear" && isUpdate) {
	    item.vendors().clear();
	} else if (tag == "stocked") {
	    vector<Store> stores;
	    if (e.attribute("store").isEmpty()) {
		stores = all_stores;
	    } else if (defaultStore.id() != INVALID_ID) {
		stores.push_back(defaultStore);
	    } else {
		Store store;
		if (findStore(e.attribute("store"), store))
		    stores.push_back(store);
		else
		    logError(tr("Unknown store: " + text));
	    }
	    for (unsigned int i = 0; i < stores.size(); ++i) {
		bool found = false;
		for (unsigned int j = 0; j < item.stores().size(); ++j) {
		    if (item.stores()[j].store_id == stores[i].id()) {
			item.stores()[j].stocked = iconvBoolean(text);
			found = true;
			break;
		    }
		}
		if (!found) {
		    ItemStore info;
		    info.store_id = stores[i].id();
		    info.stocked = iconvBoolean(text);
		    item.stores().push_back(info);
		}
	    }
	} else if (tag == "location") {
	    vector<Store> stores;
	    if (e.attribute("store").isEmpty()) {
		stores = all_stores;
	    } else if (defaultStore.id() != INVALID_ID) {
		stores.push_back(defaultStore);
	    } else {
		Store store;
		if (findStore(e.attribute("store"), store))
		    stores.push_back(store);
		else
		    logError(tr("Unknown store: " + text));
	    }

	    for (unsigned int i = 0; i < stores.size(); ++i) {
		Location location;
		if (findLocation(text, stores[i].id(), location)) {
		    bool found = false;
		    for (unsigned int j = 0; j < item.stores().size(); ++j) {
			if (item.stores()[j].store_id == stores[i].id()) {
			    item.stores()[j].location_id = location.id();
			    found = true;
			    break;
			}
		    }
		    if (!found) {
			ItemStore info;
			info.store_id = stores[i].id();
			info.location_id = location.id();
			item.stores().push_back(info);
		    }
		} else {
		    logWarning(tr("Unknown location: ") + text);
		}
	    }
	} else if (tag == "price") {
	    vector<Store> stores;
	    if (e.attribute("store").isEmpty()) {
		stores = all_stores;
	    } else if (defaultStore.id() != INVALID_ID) {
		stores.push_back(defaultStore);
	    } else {
		Store store;
		if (findStore(e.attribute("store"), store))
		    stores.push_back(store);
		else
		    logError(tr("Unknown store: " + text));
	    }

	    vector<QString> sizes;
	    QString size = e.attribute("size");
	    if (size.isEmpty()) {
		for (unsigned int i = 0; i < item.sizes().size(); ++i)
		    sizes.push_back(item.sizes()[i].name);
	    } else if (size == "_SELL_") {
		sizes.push_back(item.sellSize());
	    } else if (size == "_PURCHASE_") {
		sizes.push_back(item.purchaseSize());
	    } else {
		sizes.push_back(size);
	    }

	    for (unsigned int i = 0; i < stores.size(); ++i) {
		const Store& store = stores[i];
		for (unsigned int j = 0; j < sizes.size(); ++j) {
		    QString size = sizes[j];
		    bool found = false;
		    for (unsigned int k = 0; k < item.costs().size(); ++k) {
			ItemCost& info = item.costs()[k];
			if (info.store_id != store.id()) continue;
			if (info.size != size) continue;

			info.price = iconvPrice(text);
			found = true;
			break;
		    }
		    if (!found) {
			ItemCost info;
			info.store_id = store.id();
			info.size = size;
			info.price = iconvPrice(text);
			item.costs().push_back(info);
		    }
		}
	    }
	} else if (tag == "cost") {
	    vector<Store> stores;
	    if (e.attribute("store").isEmpty()) {
		stores = all_stores;
	    } else if (defaultStore.id() != INVALID_ID) {
		stores.push_back(defaultStore);
	    } else {
		Store store;
		if (findStore(e.attribute("store"), store))
		    stores.push_back(store);
		else
		    logError(tr("Unknown store: " + text));
	    }

	    vector<QString> sizes;
	    QString size = e.attribute("size");
	    if (size.isEmpty()) {
		for (unsigned int i = 0; i < item.sizes().size(); ++i)
		    sizes.push_back(item.sizes()[i].name);
	    } else if (size == "_SELL_") {
		sizes.push_back(item.sellSize());
	    } else if (size == "_PURCHASE_") {
		sizes.push_back(item.purchaseSize());
	    } else {
		sizes.push_back(size);
	    }

	    for (unsigned int i = 0; i < stores.size(); ++i) {
		const Store& store = stores[i];
		for (unsigned int j = 0; j < sizes.size(); ++j) {
		    QString size = sizes[j];
		    bool found = false;
		    for (unsigned int k=0; k < item.costs().size(); ++k) {
			ItemCost& info = item.costs()[k];
			if (info.store_id != store.id()) continue;
			if (info.size != size) continue;

			info.cost = iconvPrice(text);
			found = true;
			break;
		    }
		    if (!found) {
			ItemCost info;
			info.store_id = store.id();
			info.size = size;
			info.cost = iconvPrice(text);
			item.costs().push_back(info);
		    }
		}
	    }
	} else if (tag == "targetGM") {
	    vector<Store> stores;
	    if (e.attribute("store").isEmpty()) {
		stores = all_stores;
	    } else if (defaultStore.id() != INVALID_ID) {
		stores.push_back(defaultStore);
	    } else {
		Store store;
		if (findStore(e.attribute("store"), store))
		    stores.push_back(store);
		else
		    logError(tr("Unknown store: " + text));
	    }

	    vector<QString> sizes;
	    if (e.attribute("size").isEmpty()) {
		for (unsigned int i = 0; i < item.sizes().size(); ++i)
		    sizes.push_back(item.sizes()[i].name);
	    } else {
		sizes.push_back(e.attribute("size"));
	    }

	    for (unsigned int i = 0; i < stores.size(); ++i) {
		const Store& store = stores[i];
		for (unsigned int j = 0; j < sizes.size(); ++j) {
		    QString size = sizes[j];
		    bool found = false;
		    for (unsigned int k = 0; k < item.costs().size(); ++k) {
			ItemCost& info = item.costs()[k];
			if (info.store_id != store.id()) continue;
			if (info.size != size) continue;

			info.target_gm = iconvFixed(text);
			found = true;
			break;
		    }
		    if (!found) {
			ItemCost info;
			info.store_id = store.id();
			info.size = size;
			info.target_gm = iconvFixed(text);
			item.costs().push_back(info);
		    }
		}
	    }
	} else if (tag == "allowedVariance") {
	    vector<Store> stores;
	    if (e.attribute("store").isEmpty()) {
		stores = all_stores;
	    } else if (defaultStore.id() != INVALID_ID) {
		stores.push_back(defaultStore);
	    } else {
		Store store;
		if (findStore(e.attribute("store"), store))
		    stores.push_back(store);
		else
		    logError(tr("Unknown store: " + text));
	    }

	    vector<QString> sizes;
	    if (e.attribute("size").isEmpty()) {
		for (unsigned int i = 0; i < item.sizes().size(); ++i)
		    sizes.push_back(item.sizes()[i].name);
	    } else {
		sizes.push_back(e.attribute("size"));
	    }

	    for (unsigned int i = 0; i < stores.size(); ++i) {
		const Store& store = stores[i];
		for (unsigned int j = 0; j < sizes.size(); ++j) {
		    QString size = sizes[j];
		    bool found = false;
		    for (unsigned int k = 0; k < item.costs().size(); ++k) {
			ItemCost& info = item.costs()[k];
			if (info.store_id != store.id()) continue;
			if (info.size != size) continue;

			info.allowed_var = iconvFixed(text);
			found = true;
			break;
		    }
		    if (!found) {
			ItemCost info;
			info.store_id = store.id();
			info.size = size;
			info.allowed_var = iconvFixed(text);
			item.costs().push_back(info);
		    }
		}
	    }
	} else if (tag == "component") {
	    Item component;
	    if (findItem(text, component)) {
		Component info;
		info.item_id = component.id();
		info.number = text;
		info.size = e.attribute("size");
		info.qty = e.attribute("qty", "1").toDouble();
		item.items().push_back(info);
	    } else {
		logWarning(tr("Unknown item: %1").arg(text));
	    }
	} else if (tag == "group" || tag == "groupInsert") {
	    Id group_id = iconvGroup(text, Group::ITEM);
	    if (group_id != INVALID_ID) {
		bool found = false;
		for (unsigned int i = 0; i < item.groups().size(); ++i) {
		    if (item.groups()[i] == group_id) {
			found = true;
			break;
		    }
		}
		if (!found)
		    item.groups().push_back(group_id);
	    } else {
		logWarning(tr("Unknown group: %1").arg(text));
	    }
	} else if (tag == "groupDelete" && isUpdate) {
	    Id group_id = iconvGroup(text, Group::ITEM);
	    if (group_id != INVALID_ID) {
		bool found = false;
		for (unsigned int i = 0; i < item.groups().size(); ++i) {
		    if (item.groups()[i] == group_id) {
			found = true;
			item.groups().erase(item.groups().begin() + i);
			break;
		    }
		}
		if (!found)
		    logWarning(tr("Group not found: %1").arg(text));
	    } else {
		logWarning(tr("Unknown group: %1").arg(text));
	    }
	} else if (tag == "groupClear" && isUpdate) {
	    item.groups().clear();
	} else if (tag == "extra") {
	    Extra extra;
	    if (findExtra("Item", e.attribute("name"), extra))
		item.setValue(extra.id(), text);
	    else
		logWarning(tr("Unknown extra: %1,%21")
			   .arg("Item").arg(e.attribute("name")));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    // Set target margin
    unsigned int j;
    for (j = 0; j < all_stores.size(); ++j) {
	const Store& store = all_stores[j];

	unsigned int k;
	for (k = 0; k < item.sizes().size(); ++k) {
	    QString size = item.sizes()[k].name;

	    fixed target = item.targetGM(store.id(), size);
	    if (target == 0.0) {
		bool found = false;
		for (unsigned int i = 0; i < item.costs().size(); ++i) {
		    ItemCost& cost = item.costs()[i];
		    if (cost.store_id != store.id()) continue;
		    if (cost.size != size) continue;

		    found = true;
		    cost.target_gm = subdept.targetGM();
		    cost.allowed_var = subdept.allowedVariance();
		    break;
		}

		if (!found) {
		    ItemCost cost;
		    cost.store_id = store.id();
		    cost.size = size;
		    cost.target_gm = subdept.targetGM();
		    cost.allowed_var = subdept.allowedVariance();
		    item.costs().push_back(cost);
		}
	    }
	}

	bool found = false;
	for (k = 0; k < item.stores().size(); ++k) {
	    if (item.stores()[k].store_id == store.id()) {
		found = true;
		break;
	    }
	}
	if (!found) {
	    ItemStore info;
	    info.store_id = store.id();
	    info.stocked = false;
	    item.stores().push_back(info);
	}
    }

    // Clear accounts/taxes as needed
    bool purchased = item.isPurchased();
    bool sold = item.isSold();
    bool inventoried = item.isInventoried();
    if (!(inventoried && sold) && !(purchased && !inventoried))
	item.setExpenseAccount(INVALID_ID);
    if (!sold) {
	item.setIncomeAccount(INVALID_ID);
	item.setSellTax(INVALID_ID);
    }
    if (!inventoried)
	item.setAssetAccount(INVALID_ID);
    if (!purchased)
	item.setPurchaseTax(INVALID_ID);
    if (!sold || !purchased || !inventoried)
	item.setOpenDept(false);

    if (isUpdate) {
	if (!_db->update(orig, item)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(item)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importItemDelete(QDomNodeList& nodes)
{
    _type = tr("Item");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "plu" || tag == "number" || tag == "externalId") {
	    _name = text;
	    Item item;
	    if (!findItem(text, item)) {
		logError(tr("Item not found for delete"));
	    } else {
		if (!_db->remove(item)) {
		    logQuasarErrors(tr("Delete failed"));
		} else {
		    logInfo(tr("Data deleted"));
		}
	    }
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    return true;
}

bool
DataImport::importLabelBatch(QDomNodeList& nodes)
{
    LabelBatch batch;

    _type = tr("Label Batch");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "externalId") {
	    batch.setExternalId(iconvFixed(text));
	} else if (tag == "number") {
	    batch.setNumber(text);
	    _name = text;
	} else if (tag == "description") {
	    batch.setDescription(text);
	} else if (tag == "type") {
	    batch.setType(text);
	} else if (tag == "store") {
	    batch.setStoreId(iconvStore(text));
	} else if (tag == "item") {
	    Item item;
	    if (findItem(text, item)) {
		QString size = e.attribute("size");
		if (size.isEmpty()) size = item.numberSize(text);
		if (size.isEmpty()) size = item.sellSize();
		int count = e.attribute("count", "1").toInt();

		LabelBatchItem line;
		line.item_id = item.id();
		line.number = text;
		line.size = size;
		line.count = count;
		batch.items().push_back(line);
	    } else {
		logWarning(tr("Unknown item: %1").arg(text));
	    }
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (!_db->create(batch)) {
	logError(tr("Create failed"));
    } else {
	_name = batch.number();
	logInfo(tr("Data created"));
    }

    return true;
}

bool
DataImport::importLocation(QDomNodeList& nodes, bool isUpdate)
{
    Location location, orig;

    _type = tr("Location");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "externalId") {
	    location.setExternalId(iconvFixed(text));
	} else if (tag == "section") {
	    location.setSection(text);
	    _name = location.canonicalName();
	} else if (tag == "fixture") {
	    location.setFixture(text);
	    _name = location.canonicalName();
	} else if (tag == "bin") {
	    location.setBin(text);
	    _name = location.canonicalName();
	} else if (tag == "store") {
	    location.setStoreId(iconvStore(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, location)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(location)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

static void
addTax(Order& order, fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	int found = -1;
	for (unsigned int j = 0; j < order.taxes().size(); ++j) {
	    OrderTax& line = order.taxes()[j];
	    if (line.tax_id == tax_id) {
		found = j;
		break;
	    }
	}

	if (found >= 0) {
	    order.taxes()[found].taxable += taxable;
	    order.taxes()[found].amount += tax_amt;
	} else {
	    OrderTax line(tax_id, taxable, tax_amt);
	    order.taxes().push_back(line);
	}
    }
}

static void
addTaxInc(Order& order,fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	int found = -1;
	for (unsigned int j = 0; j < order.taxes().size(); ++j) {
	    OrderTax& line = order.taxes()[j];
	    if (line.tax_id == tax_id) {
		found = j;
		break;
	    }
	}

	if (found >= 0) {
	    order.taxes()[found].inc_taxable += taxable;
	    order.taxes()[found].inc_amount += tax_amt;
	} else {
	    OrderTax line(tax_id, 0.0, 0.0, taxable, tax_amt);
	    order.taxes().push_back(line);
	}
    }
}

bool
DataImport::importOrder(QDomNodeList& nodes, bool isUpdate)
{
    Order order, orig;

    _type = tr("Order");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findOrder(text, order)) {
		orig = order;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    order.setExternalId(iconvFixed(text));
	} else if (tag == "number") {
	    order.setNumber(text);
	    _name = text;
	} else if (tag == "vendor") {
	    Vendor vendor;
	    if (findVendor(text, vendor)) {
		order.setVendorId(vendor.id());
		order.setVendorAddress(vendor.address());
		order.setTermsId(vendor.termsId());
	    } else {
		logWarning(tr("Unknown vendor: %1").arg(text));
	    }
	} else if (tag == "vendorAddr") {
	    order.setVendorAddress(text);
	} else if (tag == "shipTo") {
	    Customer customer;
	    if (findCustomer(text, customer)) {
		order.setShipId(customer.id());
		order.setShipAddress(customer.address());
	    } else {
		logWarning(tr("Unknown customer: %1").arg(text));
	    }
	} else if (tag == "shipAddr") {
	    order.setShipAddress(text);
	} else if (tag == "shipVia") {
	    order.setShipVia(text);
	} else if (tag == "terms") {
	    order.setTermsId(iconvTerms(text));
	} else if (tag == "store") {
	    order.setStoreId(iconvStore(text));
	} else if (tag == "comment") {
	    order.setComment(text);
	} else if (tag == "reference") {
	    order.setReference(text);
	} else if (tag == "date") {
	    order.setDate(iconvDate(text));
	} else if (tag == "item" || tag == "itemInsert") {
	    Item item;
	    if (findItem(text, item)) {
		QString size = e.attribute("size");
		if (size.isEmpty()) size = item.numberSize(text);
		if (size.isEmpty()) size = item.purchaseSize();

		fixed ordered = iconvFixed(e.attribute("qty", "1"));

		Id tax_id = item.purchaseTax();
		if (!e.attribute("tax").isEmpty())
		    tax_id = iconvTax(e.attribute("tax"));

		Price cost;
		fixed ext_cost;
		if (!e.attribute("ext_cost").isEmpty()) {
		    ext_cost = iconvFixed(e.attribute("ext_cost"));
		    cost = Price(ext_cost / ordered);
		} else if (!e.attribute("cost").isEmpty()) {
		    cost = iconvPrice(e.attribute("cost"));
		    ext_cost = cost.calculate(ordered);
		} else {
		    cost = item.cost(order.storeId(), size);
		    ext_cost = cost.calculate(ordered);
		}

		OrderItem line;
		line.item_id = item.id();
		line.number = text;
		line.description = item.description();
		line.size = size;
		line.size_qty = item.sizeQty(size);
		line.ordered = ordered;
		line.cost = cost;
		line.ext_cost = ext_cost;
		line.ext_deposit = item.deposit() * ordered * line.size_qty;
		line.tax_id = tax_id;
		line.item_tax_id = item.purchaseTax();
		line.include_tax = item.costIncludesTax();
		line.include_deposit = item.costIncludesDeposit();
		order.items().push_back(line);
	    } else {
		logWarning(tr("Unknown item: %1").arg(text));
	    }
	} else if (tag == "itemUpdate" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < order.items().size(); ++i) {
		OrderItem& line = order.items()[i];
		if (line.number != text) continue;

		Item item;
		findItem(text, item);
		fixed deposit = item.deposit();

		if (!e.attribute("qty").isEmpty()) {
		    line.ordered = iconvFixed(e.attribute("qty"));
		    line.ext_cost = line.cost.calculate(line.ordered);
		    line.ext_deposit = deposit * line.ordered * line.size_qty;
		}
		if (!e.attribute("ext_cost").isEmpty()) {
		    line.ext_cost = iconvFixed(e.attribute("ext_cost"));
		    line.cost = Price(line.ext_cost / line.ordered);
		} else if (!e.attribute("cost").isEmpty()) {
		    line.cost = iconvPrice(e.attribute("cost"));
		    line.ext_cost = line.cost.calculate(line.ordered);
		}
		if (!e.attribute("size").isEmpty()) {
		    line.size = e.attribute("size");
		    line.size_qty = item.sizeQty(line.size);
		    line.cost = item.cost(order.storeId(), line.size);
		    line.ext_cost = line.cost.calculate(line.ordered);
		    line.ext_deposit = deposit * line.ordered * line.size_qty;
		}
		if (!e.attribute("tax").isEmpty())
		    line.tax_id = iconvTax(e.attribute("tax"));

		found = true;
		break;
	    }
	    if (!found)
		logWarning(tr("Item not found: %1").arg(text));
	} else if (tag == "itemDelete" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < order.items().size(); ++i) {
		if (order.items()[i].number == text) {
		    found = true;
		    order.items().erase(order.items().begin() + i);
		    break;
		}
	    }
	    if (!found)
		logWarning(tr("Item not found: %1").arg(text));
	} else if (tag == "itemClear" && isUpdate) {
	    order.items().clear();
	} else if (tag == "charge" || tag == "chargeInsert") {
	    Charge charge;
	    if (findCharge(text, charge)) {
		Id tax_id = charge.taxId();
		if (!e.attribute("tax").isEmpty())
		    tax_id = iconvTax(e.attribute("tax"));

		fixed amount = iconvFixed(e.attribute("amount", "0"));
		bool internal = iconvBoolean(e.attribute("internal", "Yes"));

		OrderCharge line;
		line.charge_id = charge.id();
		line.tax_id = tax_id;
		line.amount = amount;
		line.internal = internal;
		order.charges().push_back(line);
	    } else {
		logWarning(tr("Unknown charge: %1").arg(text));
	    }
	} else if (tag == "chargeUpdate" && isUpdate) {
	    // TODO: charge update
	} else if (tag == "chargeDelete" && isUpdate) {
	    Charge charge;
	    if (findCharge(text, charge)) {
		bool found = false;
		for (unsigned int i = 0; i < order.charges().size(); ++i) {
		    if (order.charges()[i].charge_id == charge.id()) {
			found = true;
			order.charges().erase(order.charges().begin() + i);
			break;
		    }
		}
		if (!found)
		    logWarning(tr("Charge not found: %1").arg(text));
	    } else {
		logWarning(tr("Unknown charge: %1").arg(text));
	    }
	} else if (tag == "chargeClear" && isUpdate) {
	    order.charges().clear();
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (order.date().isNull())
	order.setDate(QDate::currentDate());

    // No vendor tax exempt yet but the code is here
    Tax exempt;

    ObjectCache cache(_db);

    // Process items
    fixed item_total = 0.0;
    fixed total_weight = 0.0;
    order.taxes().clear();
    for (unsigned int i = 0; i < order.items().size(); ++i) {
	OrderItem& line = order.items()[i];
	if (line.item_id == INVALID_ID) continue;

	Item item;
	_db->lookup(line.item_id, item);

	// Calculate base price
	line.ext_base = line.ext_cost;
	if (line.include_deposit)
	    line.ext_base -= line.ext_deposit;

	// Calculate taxes
	fixed ext_tax = 0.0;
	if (line.tax_id != INVALID_ID) {
	    fixed base = line.ext_base;
	    Tax tax;
	    _db->lookup(line.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (line.include_tax) {
		ext_tax = _db->calculateTaxOff(cache, tax, base, tax_ids,
					       tax_amts);
		addTaxInc(order, line.ext_base, tax_ids, tax_amts);
		line.ext_base -= ext_tax;
	    } else {
		ext_tax = _db->calculateTaxOn(cache, tax, base, exempt,
					      tax_ids, tax_amts);
		addTax(order, line.ext_base, tax_ids, tax_amts);
	    }
	}
	line.ext_tax = ext_tax;

	// Clear out old charges
	line.int_charges = 0.0;
	line.ext_charges = 0.0;

	// Add up totals
	item_total += line.ext_base;
	total_weight += item.weight(line.size) * line.ordered;;
    }

    // Process internal charges
    for (unsigned int i = 0; i < order.charges().size(); ++i) {
	OrderCharge& cline = order.charges()[i];
	if (!cline.internal) continue;
	if (cline.charge_id == INVALID_ID) continue;

	Charge charge;
	_db->lookup(cline.charge_id, charge);

	// Check if should calculate
	fixed amount = cline.amount;
	if (charge.calculateMethod() != Charge::MANUAL) {
	    switch (charge.calculateMethod()) {
	    case Charge::COST:
		amount = item_total * charge.amount() / 100.0;
		break;
	    case Charge::WEIGHT:
		amount = total_weight * charge.amount();
		break;
	    default:
		amount = 0.0;
	        break;
	    }
	    amount.moneyRound();
	}

	// Calculate tax
	fixed base = amount;
	if (cline.tax_id != INVALID_ID) {
	    Tax tax;
	    _db->lookup(cline.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (charge.includeTax()) {
		base -= _db->calculateTaxOff(cache, tax, amount, tax_ids,
					     tax_amts);
		addTaxInc(order, amount, tax_ids, tax_amts);
	    } else {
		_db->calculateTaxOn(cache, tax, amount, exempt, tax_ids,
				    tax_amts);
		addTax(order, amount, tax_ids, tax_amts);
	    }
	}

	cline.amount = amount;
	cline.base = base;

	// Check if should allocate
	if (charge.allocateMethod() != Charge::NONE) {
	    int last = -1;
	    fixed remain = cline.base;
	    for (unsigned int j = 0; j < order.items().size(); ++j) {
		OrderItem& line = order.items()[j];
		if (line.item_id == INVALID_ID) continue;

		Item item;
		_db->lookup(line.item_id, item);

		fixed weight = item.weight(line.size) * line.ordered;
		fixed ratio = 0.0;
		switch (charge.allocateMethod()) {
		case Charge::COST:
		    ratio = line.ext_base / item_total;
		    break;
		case Charge::WEIGHT:
		    ratio = weight / total_weight;
		    break;
		default:
		    break;
		}

		fixed alloc = cline.base * ratio;
		alloc.moneyRound();
		line.int_charges += alloc;
		remain -= alloc;
		last = j;
	    }
	    order.items()[last].int_charges += remain;
	}
    }

    // Process external charges
    for (unsigned int i = 0; i < order.charges().size(); ++i) {
	OrderCharge& cline = order.charges()[i];
	if (cline.internal) continue;
	if (cline.charge_id == INVALID_ID) continue;

	Charge charge;
	_db->lookup(cline.charge_id, charge);

	// Check if should calculate
	fixed amount = cline.amount;
	if (charge.calculateMethod() != Charge::MANUAL) {
	    switch (charge.calculateMethod()) {
	    case Charge::COST:
		amount = item_total * charge.amount() / 100.0;
		break;
	    case Charge::WEIGHT:
		amount = total_weight * charge.amount();
		break;
	    default:
		amount = 0.0;
		break;
	    }
	    amount.moneyRound();
	}

	// Calculate tax
	fixed base = amount;
	if (cline.tax_id != INVALID_ID) {
	    Tax tax;
	    _db->lookup(cline.tax_id, tax);
	    vector<Id> tax_ids;
	    vector<fixed> tax_amts;

	    if (charge.includeTax()) {
		base -= _db->calculateTaxOff(cache, tax, amount, tax_ids,
					     tax_amts);
	    }
	}

	cline.amount = amount;
	cline.base = base;

	// Check if should allocate
	if (charge.allocateMethod() != Charge::NONE) {
	    int last = -1;
	    fixed remain = cline.base;
	    for (unsigned int j = 0; j < order.items().size(); ++j) {
		OrderItem& line = order.items()[j];
		if (line.item_id == INVALID_ID) continue;

		Item item;
		_db->lookup(line.item_id, item);

		fixed weight = item.weight(line.size) * line.ordered;
		fixed ratio = 0.0;
		switch (charge.allocateMethod()) {
		case Charge::COST:
		    ratio = line.ext_base / item_total;
		    break;
		case Charge::WEIGHT:
		    ratio = weight / total_weight;
		    break;
		default:
		    break;
		}

		fixed alloc = cline.base * ratio;
		alloc.moneyRound();
		line.ext_charges += alloc;
		remain -= alloc;
		last = j;
	    }
	    order.items()[last].ext_charges += remain;
	}
    }

    // Round taxes to two decimal places
    for (unsigned int i = 0; i < order.taxes().size(); ++i)
	order.taxes()[i].amount.moneyRound();

    if (isUpdate) {
	if (!_db->update(orig, order)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(order))
	    logError(tr("Create failed"));
	else
	    _name = order.number();
	    logInfo(tr("Data created"));
    }

    return true;
}

bool
DataImport::importTemplate(QDomNodeList& nodes, bool isUpdate)
{
    OrderTemplate orig, templ;

    _type = tr("Order Template");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findTemplate(text, templ)) {
		orig = templ;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    templ.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    templ.setName(text);
	    _name = text;
	} else if (tag == "vendor") {
	    templ.setVendorId(iconvVendor(text));
	} else if (tag == "item" || tag == "itemInsert") {
	    Item item;
	    if (findItem(text, item)) {
		TemplateItem info;
		info.item_id = item.id();
		info.number = text;
		info.size = e.attribute("size", item.purchaseSize());
		templ.items().push_back(info);
	    } else {
		logWarning(tr("Unknown item: %1").arg(text));
	    }
	} else if (tag == "itemUpdate" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < templ.items().size(); ++i) {
		TemplateItem& line = templ.items()[i];
		if (line.number != text) continue;

		if (!e.attribute("size").isEmpty()) {
		    line.size = e.attribute("size");
		}

		found = true;
		break;
	    }
	    if (!found)
		logWarning(tr("Item not found: %1").arg(text));
	} else if (tag == "itemDelete" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < templ.items().size(); ++i) {
		if (templ.items()[i].number == text) {
		    found = true;
		    templ.items().erase(templ.items().begin() + i);
		    break;
		}
	    }
	    if (!found)
		logWarning(tr("Item not found: %1").arg(text));
	} else if (tag == "itemClear" && isUpdate) {
	    templ.items().clear();
	} else if (tag == "charge" || tag == "chargeInsert") {
	    Charge charge;
	    if (findCharge(text, charge)) {
		Id tax_id = charge.taxId();
		if (!e.attribute("tax").isEmpty())
		    tax_id = iconvTax(e.attribute("tax"));

		fixed amount = iconvFixed(e.attribute("amount", "0"));
		bool internal = iconvBoolean(e.attribute("internal", "Yes"));

		TemplateCharge line;
		line.charge_id = charge.id();
		line.tax_id = tax_id;
		line.amount = amount;
		line.internal = internal;
		templ.charges().push_back(line);
	    } else {
		logWarning(tr("Unknown charge: %1").arg(text));
	    }
	} else if (tag == "chargeUpdate" && isUpdate) {
	    // TODO: charge update
	} else if (tag == "chargeDelete" && isUpdate) {
	    Charge charge;
	    if (findCharge(text, charge)) {
		bool found = false;
		for (unsigned int i = 0; i < templ.charges().size(); ++i) {
		    if (templ.charges()[i].charge_id == charge.id()) {
			found = true;
			templ.charges().erase(templ.charges().begin() + i);
			break;
		    }
		}
		if (!found)
		    logWarning(tr("Charge not found: %1").arg(text));
	    } else {
		logWarning(tr("Unknown charge: %1").arg(text));
	    }
	} else if (tag == "chargeClear" && isUpdate) {
	    templ.charges().clear();
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, templ)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(templ))
	    logError(tr("Create failed"));
	else
	    logInfo(tr("Data created"));
    }

    return true;
}

bool
DataImport::importPatGroup(QDomNodeList& nodes, bool isUpdate)
{
    PatGroup orig, group;

    _type = tr("Patgroup");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    _name = text;
	    if (findPatGroup(text, group))
		orig = group;
	} else if (tag == "externalId") {
	    group.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    group.setName(text);
	    _name = text;
	} else if (tag == "number") {
	    group.setNumber(text);
	} else if (tag == "equity_card") {
	    group.setEquityId(iconvCustomer(text));
	} else if (tag == "credit_card") {
	    group.setCreditId(iconvCustomer(text));
	} else if (tag == "customer") {
	    group.card_ids().push_back(iconvCustomer(text));
	} else if (tag == "vendor") {
	    group.card_ids().push_back(iconvVendor(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, group)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(group)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importPatGroupDelete(QDomNodeList& nodes)
{
    _type = tr("Patgroup");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "number" || tag == "name" || tag == "externalId") {
	    _name = text;
	    PatGroup group;
	    if (!findPatGroup(text, group)) {
		logError(tr("Patgroup not found for delete"));
		continue;
	    }

	    if (!_db->remove(group)) {
		logQuasarErrors(tr("Delete failed"));
	    } else {
		logInfo(tr("Data deleted"));
	    }
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    return true;
}

bool
DataImport::importPersonal(QDomNodeList& nodes, bool isUpdate)
{
    Personal orig, personal;

    _type = tr("Personal");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findPersonal(text, personal)) {
		orig = personal;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    personal.setExternalId(iconvFixed(text));
	} else if (tag == "firstName") {
	    personal.setFirstName(text);
	    _name = personal.name();
	} else if (tag == "lastName") {
	    personal.setLastName(text);
	    _name = personal.name();
	} else if (tag == "company") {
	    personal.setCompany(iconvBoolean(text));
	} else if (tag == "number") {
	    personal.setNumber(text);
	} else if (tag == "street") {
	    personal.setStreet(text);
	} else if (tag == "street2") {
	    personal.setStreet2(text);
	} else if (tag == "city") {
	    personal.setCity(text);
	} else if (tag == "province") {
	    personal.setProvince(text);
	} else if (tag == "country") {
	    personal.setCountry(text);
	} else if (tag == "postal") {
	    personal.setPostal(text);
	} else if (tag == "phone") {
	    personal.setPhoneNumber(text);
	} else if (tag == "fax") {
	    personal.setFaxNumber(text);
	} else if (tag == "email") {
	    personal.setEmail(text);
	} else if (tag == "webPage") {
	    personal.setWebPage(text);
	} else if (tag == "contact") {
	    personal.setContact(text);
	} else if (tag == "comments") {
	    personal.setComments(text);
	} else if (tag == "extra") {
	    Extra extra;
	    if (findExtra("Personal", e.attribute("name"), extra))
		personal.setValue(extra.id(), text);
	    else
		logWarning(tr("Unknown extra: %1,%21")
			   .arg("Personal").arg(e.attribute("name")));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, personal)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(personal)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importPrice(QDomNodeList& nodes, bool isCost)
{
    ItemPrice price;
    price.setIsCost(isCost);

    _type = tr("Price");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "externalId") {
	    price.setExternalId(iconvFixed(text));
	} else if (tag == "item") {
	    Item item;
	    if (findItem(text, item)) {
		price.setItemId(item.id());
		price.setNumber(text);
	    } else {
		logWarning(tr("Unknown item: %1").arg(text));
	    }
	} else if (tag == "itemGroup") {
	    price.setItemGroup(iconvGroup(text, Group::ITEM));
	} else if (tag == "dept") {
	    price.setDeptId(iconvDept(text));
	} else if (tag == "subdept") {
	    price.setSubdeptId(iconvSubdept(text));
	} else if (tag == "size") {
	    price.setSize(text);
	} else if (tag == "customer" && !isCost) {
	    price.setCardId(iconvCustomer(text));
	} else if (tag == "customerGroup" && !isCost) {
	    price.setCardGroup(iconvGroup(text, Group::CUSTOMER));
	} else if (tag == "vendor" && isCost) {
	    price.setCardId(iconvVendor(text));
	} else if (tag == "vendorGroup" && isCost) {
	    price.setCardGroup(iconvGroup(text, Group::VENDOR));
	} else if (tag == "store") {
	    price.setStoreId(iconvStore(text));
	} else if (tag == "promotion") {
	    price.setPromotion(iconvBoolean(text));
	} else if (tag == "discountable") {
	    price.setDiscountable(iconvBoolean(text));
	} else if (tag == "startDate") {
	    price.setStartDate(iconvDate(text));
	} else if (tag == "stopDate") {
	    price.setStopDate(iconvDate(text));
	} else if (tag == "qtyLimit") {
	    price.setQtyLimit(iconvFixed(text));
	} else if (tag == "minQty") {
	    price.setMinimumQty(iconvFixed(text));
	} else if (tag == "dayMask") {
	    price.setDayOfWeek(text);
	} else if (tag == "price") {
	    price.setMethod(ItemPrice::PRICE);
	    price.setPrice(iconvPrice(text));
	} else if (tag == "costPlus") {
	    price.setMethod(ItemPrice::COST_PLUS);
	    price.setPercentDiscount(iconvFixed(text));
	} else if (tag == "percentOff") {
	    price.setMethod(ItemPrice::PERCENT_OFF);
	    price.setPercentDiscount(iconvFixed(text));
	} else if (tag == "dollarOff") {
	    price.setMethod(ItemPrice::DOLLAR_OFF);
	    price.setDollarDiscount(iconvFixed(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (!_db->create(price))
	logError(tr("Create failed"));
    else
	logInfo(tr("Data created"));

    return true;
}

bool
DataImport::importPriceBatch(QDomNodeList& nodes)
{
    PriceBatch batch;

    _type = tr("Price Batch");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "externalId") {
	    batch.setExternalId(iconvFixed(text));
	} else if (tag == "number") {
	    batch.setNumber(text);
	    _name = text;
	} else if (tag == "description") {
	    batch.setDescription(text);
	} else if (tag == "store") {
	    batch.setStoreId(iconvStore(text));
	} else if (tag == "item") {
	    Item item;
	    if (findItem(text, item)) {
		QString size = e.attribute("size");
		if (size.isEmpty()) size = item.numberSize(text);
		if (size.isEmpty()) size = item.sellSize();

		Id store_id = batch.storeId();
		Price cost = item.cost(store_id, size);
		Price price = item.price(store_id, size);
		Price newPrice = iconvPrice(e.attribute("price"));

		if (cost.isNull()) {
		    Price purch_cost = item.cost(store_id,item.purchaseSize());
		    fixed purch_qty = item.sizeQty(item.purchaseSize());
		    fixed size_qty = item.sizeQty(size);
		    cost = Price(purch_cost.calculate(1.0)*size_qty/purch_qty);
		}

		PriceBatchItem line;
		line.item_id = item.id();
		line.number = text;
		line.size = size;
		line.old_cost = cost;
		line.old_price = price;
		line.new_price = newPrice;
		batch.items().push_back(line);
	    } else {
		logWarning(tr("Unknown item: %1").arg(text));
	    }
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (!_db->create(batch)) {
	logError(tr("Create failed"));
    } else {
	_name = batch.number();
	logInfo(tr("Data created"));
    }

    return true;
}

bool
DataImport::importPriceChange(QDomNodeList& nodes)
{
    Item item;
    QString size;
    Id store_id;
    Price price;
    Price cost;
    bool changePrice = false;
    bool changeCost = false;

    _type = tr("Price");
    _name = tr("Unknown");

    unsigned int i;
    for (i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "number") {
	    if (!findItem(text, item))
		logWarning(tr("Unknown item: %1").arg(text));
	} else if (tag == "size") {
	    size = text;
	} else if (tag == "store") {
	    store_id = iconvStore(text);
	} else if (tag == "price") {
	    changePrice = true;
	    price = iconvPrice(text);
	} else if (tag == "cost") {
	    changeCost = true;
	    cost = iconvPrice(text);
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (item.id() == INVALID_ID) return false;
    if (store_id == INVALID_ID) return false;
    if (size.isEmpty()) return false;

    Item orig = item;
    for (i = 0; i < item.costs().size(); ++i) {
	ItemCost& info = item.costs()[i];
	if (info.store_id != store_id) continue;
	if (info.size != size) continue;
	if (changePrice) info.price = price;
	if (changeCost) info.cost = cost;
    }

    return _db->update(orig, item);
}

bool
DataImport::importSecurityType(QDomNodeList& nodes, bool isUpdate)
{
    SecurityType orig, security;
    QStringList links;

    _type = tr("SecurityType");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findSecurityType(text, security)) {
		orig = security;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    security.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    security.setName(text);
	    _name = text;
	} else if (tag == "rule" || tag == "ruleInsert") {
	    SecurityRule rule;
	    rule.allowView = iconvBoolean(e.attribute("view"));
	    rule.allowCreate = iconvBoolean(e.attribute("create"));
	    rule.allowUpdate = iconvBoolean(e.attribute("update"));
	    rule.allowDelete = iconvBoolean(e.attribute("delete"));
	    rule.screen = text;
	    security.rules().push_back(rule);
	} else if (tag == "ruleUpdate" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < security.rules().size(); ++i) {
		SecurityRule& rule = security.rules()[i];
		if (rule.screen == text) {
		    found = true;
		    if (!e.attribute("view").isEmpty())
			rule.allowView = iconvBoolean(e.attribute("view"));
		    if (!e.attribute("create").isEmpty())
			rule.allowCreate = iconvBoolean(e.attribute("create"));
		    if (!e.attribute("update").isEmpty())
			rule.allowUpdate = iconvBoolean(e.attribute("update"));
		    if (!e.attribute("delete").isEmpty())
			rule.allowDelete = iconvBoolean(e.attribute("delete"));
		    break;
		}
	    }
	    if (!found)
		logWarning(tr("Rule not found: %1").arg(text));
	} else if (tag == "ruleDelete" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < security.rules().size(); ++i) {
		if (security.rules()[i].screen == text) {
		    found = true;
		    security.rules().erase(security.rules().begin() + i);
		    break;
		}
	    }
	    if (!found)
		logWarning(tr("Rule not found: %1").arg(text));
	} else if (tag =="ruleClear") {
	    security.rules().clear();
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, security)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(security)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importSlip(QDomNodeList& nodes, bool isUpdate)
{
    Slip orig, slip;

    _type = tr("Slip");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findSlip(text, slip)) {
		orig = slip;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    slip.setExternalId(iconvFixed(text));
	} else if (tag == "number") {
	    slip.setNumber(text);
	    _name = text;
	} else if (tag == "vendor") {
	    Vendor vendor;
	    if (findVendor(text, vendor)) {
		slip.setVendorId(vendor.id());
	    } else {
		logWarning(tr("Unknown vendor: %1").arg(text));
	    }
	} else if (tag == "store") {
	    slip.setStoreId(iconvStore(text));
	} else if (tag == "waybill") {
	    slip.setWaybill(text);
	} else if (tag == "carrier") {
	    slip.setCarrier(text);
	} else if (tag == "shipDate") {
	    slip.setShipDate(iconvDate(text));
	} else if (tag == "orderNumber") {
	    // TODO: support insert, delete, and clear for orders
	    slip.orders() = QStringList::split(",", text);
	} else if (tag == "invoiceNumber") {
	    slip.setInvoiceNumber(text);
	} else if (tag == "numPieces") {
	    slip.setNumPieces(text.toInt());
	} else if (tag == "item" || tag == "itemInsert") {
	    Item item;
	    if (findItem(text, item)) {
		QString size = e.attribute("size");
		if (size.isEmpty()) size = item.numberSize(text);
		if (size.isEmpty()) size = item.purchaseSize();

		fixed qty = iconvFixed(e.attribute("qty", "1"));

		SlipItem line;
		line.item_id = item.id();
		line.number = text;
		line.size = size;
		line.size_qty = item.sizeQty(size);
		line.quantity = qty;
		slip.items().push_back(line);
	    } else {
		logWarning(tr("Unknown item: %1").arg(text));
	    }
	} else if (tag == "itemUpdate" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < slip.items().size(); ++i) {
		SlipItem& line = slip.items()[i];
		if (line.number != text) continue;

		Item item;
		findItem(text, item);

		if (!e.attribute("qty").isEmpty()) {
		    line.quantity = iconvFixed(e.attribute("qty"));
		}
		if (!e.attribute("size").isEmpty()) {
		    line.size = e.attribute("size");
		    line.size_qty = item.sizeQty(line.size);
		}

		found = true;
		break;
	    }
	    if (!found)
		logWarning(tr("Item not found: %1").arg(text));
	} else if (tag == "itemDelete" && isUpdate) {
	    bool found = false;
	    for (unsigned int i = 0; i < slip.items().size(); ++i) {
		if (slip.items()[i].number == text) {
		    found = true;
		    slip.items().erase(slip.items().begin() + i);
		    break;
		}
	    }
	    if (!found)
		logWarning(tr("Item not found: %1").arg(text));
	} else if (tag == "itemClear" && isUpdate) {
	    slip.items().clear();
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, slip)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(slip)) {
	    logError(tr("Create failed"));
	} else {
	    _name = slip.number();
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importSlipDelete(QDomNodeList& nodes)
{
    _type = tr("Slip");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "number" || tag == "externalId") {
	    _name = text;
	    Slip slip;
	    if (!findSlip(text, slip)) {
		logError(tr("Slip not found for delete"));
		continue;
	    }

	    if (!_db->remove(slip)) {
		logQuasarErrors(tr("Delete failed"));
	    } else {
		logInfo(tr("Data deleted"));
	    }
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    return true;
}

bool
DataImport::importStation(QDomNodeList& nodes, bool isUpdate)
{
    Station orig, station;
    QStringList links;

    _type = tr("Station");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findStation(text, station)) {
		orig = station;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    station.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    station.setName(text);
	    _name = text;
	} else if (tag == "number") {
	    station.setNumber(text);
	    _name = text;
	} else if (tag == "store") {
	    station.setStoreId(iconvStore(text));
	} else if (tag == "link") {
	    links.append(text);
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, station)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(station)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    _name = station.number();
	    logInfo(tr("Data created"));
	}
    }

    if (station.id() != INVALID_ID && links.count() > 0) {
	Company orig, company;
	_db->lookup(orig);
	company = orig;

	for (unsigned int i = 0; i < links.count(); ++i) {
	    if (links[i] == "safe")
		company.setSafeStation(station.id());
	    else {
		QString message = tr("Unknown station link: %1").arg(links[i]);
		logWarning(message);
	    }
	}

	_db->update(orig, company);
    }

    return true;
}

bool
DataImport::importStore(QDomNodeList& nodes, bool isUpdate)
{
    Store orig, store;
    QStringList links;

    _type = tr("Store");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findStore(text, store)) {
		orig = store;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    store.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    store.setName(text);
	    _name = text;
	} else if (tag == "number") {
	    store.setNumber(text);
	} else if (tag == "contact") {
	    store.setContact(text);
	} else if (tag == "street") {
	    store.setHasAddress(true);
	    store.setStreet(text);
	} else if (tag == "street2") {
	    store.setHasAddress(true);
	    store.setStreet2(text);
	} else if (tag == "city") {
	    store.setHasAddress(true);
	    store.setCity(text);
	} else if (tag == "province") {
	    store.setHasAddress(true);
	    store.setProvince(text);
	} else if (tag == "country") {
	    store.setHasAddress(true);
	    store.setCountry(text);
	} else if (tag == "postal") {
	    store.setHasAddress(true);
	    store.setPostal(text);
	} else if (tag == "phone") {
	    store.setHasAddress(true);
	    store.setPhoneNumber(text);
	} else if (tag == "fax") {
	    store.setHasAddress(true);
	    store.setFaxNumber(text);
	} else if (tag == "email") {
	    store.setHasAddress(true);
	    store.setEmail(text);
	} else if (tag == "webPage") {
	    store.setHasAddress(true);
	    store.setWebPage(text);
	} else if (tag == "canSell") {
	    store.setCanSell(iconvBoolean(text));
	} else if (tag == "company") {
	    store.setCompanyId(iconvCompany(text));
	} else if (tag == "link") {
	    links.append(text);
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, store)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(store)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    _name = store.number();
	    logInfo(tr("Data created"));
	}
    }

    if (store.id() != INVALID_ID && links.count() > 0) {
	Company orig, company;
	_db->lookup(orig);
	company = orig;

	for (unsigned int i = 0; i < links.count(); ++i) {
	    if (links[i] == "safe")
		company.setSafeStore(store.id());
	    else {
		QString message = tr("Unknown account link: %1").arg(links[i]);
		logWarning(message);
	    }
	}

	_db->update(orig, company);
    }

    return true;
}

bool
DataImport::importSubdept(QDomNodeList& nodes, bool isUpdate)
{
    Subdept orig, subdept;

    _type = tr("Subdept");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    _name = text;
	    if (findSubdept(text, subdept))
		orig = subdept;
	} else if (tag == "externalId") {
	    subdept.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    subdept.setName(text);
	    _name = text;
	} else if (tag == "number") {
	    subdept.setNumber(text);
	} else if (tag == "dept") {
	    subdept.setDeptId(iconvDept(text));
	} else if (tag == "purchased") {
	    subdept.setPurchased(iconvBoolean(text));
	} else if (tag == "sold") {
	    subdept.setSold(iconvBoolean(text));
	} else if (tag == "inventoried") {
	    subdept.setInventoried(iconvBoolean(text));
	} else if (tag == "expenseAccount") {
	    subdept.setExpenseAccount(iconvAccount(text));
	} else if (tag == "incomeAccount") {
	    subdept.setIncomeAccount(iconvAccount(text));
	} else if (tag == "assetAccount") {
	    subdept.setAssetAccount(iconvAccount(text));
	} else if (tag == "sellTax") {
	    subdept.setSellTax(iconvTax(text));
	} else if (tag == "purchaseTax") {
	    subdept.setPurchaseTax(iconvTax(text));
	} else if (tag == "targetGM") {
	    subdept.setTargetGM(iconvFixed(text));
	} else if (tag == "allowedVariance") {
	    subdept.setAllowedVariance(iconvFixed(text));
	} else if (tag == "discountable") {
	    subdept.setDiscountable(iconvBoolean(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, subdept)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(subdept)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importSubdeptDelete(QDomNodeList& nodes)
{
    _type = tr("Subdept");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "number" || tag == "name" || tag == "externalId") {
	    _name = text;
	    Subdept subdept;
	    if (!findSubdept(text, subdept)) {
		logError(tr("Subdept not found for delete"));
		continue;
	    }

	    if (!_db->remove(subdept)) {
		logQuasarErrors(tr("Delete failed"));
	    } else {
		logInfo(tr("Data deleted"));
	    }
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    return true;
}

bool
DataImport::importTax(QDomNodeList& nodes, bool isUpdate)
{
    Tax orig, tax;

    _type = tr("Tax");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findTax(text, tax)) {
		orig = tax;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    tax.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    tax.setName(text);
	    _name = text;
	} else if (tag == "description") {
	    tax.setDescription(text);
	} else if (tag == "number") {
	    tax.setNumber(text);
	} else if (tag == "rate") {
	    tax.setRate(iconvFixed(text));
	} else if (tag == "parent" || tag == "group") {
	    tax.setGroup(true);
	    Id group_id = iconvTax(text);
	    if (group_id != INVALID_ID)
		tax.group_ids().push_back(group_id);
	} else if (tag == "taxOnTax") {
	    Id tot_id = iconvTax(text);
	    if (tot_id != INVALID_ID)
		tax.tot_ids().push_back(tot_id);
	} else if (tag == "collected") {
	    tax.setCollectedAccount(iconvAccount(text));
	} else if (tag == "paid") {
	    tax.setPaidAccount(iconvAccount(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, tax)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(tax)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importTender(QDomNodeList& nodes, bool isUpdate)
{
    Tender orig, tender;

    _type = tr("Tender");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findTender(text, tender)) {
		orig = tender;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    tender.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    tender.setName(text);
	    _name = text;
	} else if (tag == "type") {
	    if (text == "cash")
		tender.setType(Tender::CASH);
	    else if (text == "cheque")
		tender.setType(Tender::CHEQUE);
	    else if (text == "card")
		tender.setType(Tender::CARD);
	    else
		logWarning(tr("Unknown type: %1").arg(text));
	} else if (tag == "limit") {
	    tender.setLimit(iconvFixed(text));
	} else if (tag == "rate") {
	    tender.setConvertRate(iconvFixed(text));
	} else if (tag == "overTender") {
	    tender.setOverTender(iconvBoolean(text));
	} else if (tag == "openDrawer") {
	    tender.setOpenDrawer(iconvBoolean(text));
	} else if (tag == "forceAmount") {
	    tender.setForceAmount(iconvBoolean(text));
	} else if (tag == "secondReceipt") {
	    tender.setSecondReceipt(iconvBoolean(text));
	} else if (tag == "account") {
	    tender.setAccountId(iconvAccount(text));
	} else if (tag == "safe") {
	    tender.setSafeId(iconvAccount(text));
	} else if (tag == "bank") {
	    tender.setBankId(iconvAccount(text));
	} else if (tag == "menuNumber") {
	    tender.setMenuNumber(iconvInt(text));
	} else if (tag == "denomination") {
	    TenderDenom denom;
	    denom.name = text;
	    denom.multiplier = iconvFixed(e.attribute("multiplier"));
	    tender.denominations().push_back(denom);
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, tender)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(tender)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importTerm(QDomNodeList& nodes, bool isUpdate)
{
    Term term, orig;

    _type = tr("Term");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "externalId") {
	    term.setExternalId(iconvFixed(text));
	} else if (tag == "cod") {
	    term.setCOD(iconvBoolean(text));
	} else if (tag == "discountDays") {
	    term.setDiscountDays(iconvInt(text));
	} else if (tag == "dueDays") {
	    term.setDueDays(iconvInt(text));
	} else if (tag == "discount") {
	    term.setDiscount(iconvFixed(text));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    _name = term.name();

    if (isUpdate) {
	if (!_db->update(orig, term)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(term)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importUser(QDomNodeList& nodes, bool isUpdate)
{
    User orig, user;

    _type = tr("User");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findUser(text, user)) {
		orig = user;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    user.setExternalId(iconvFixed(text));
	} else if (tag == "name") {
	    user.setName(text);
	    _name = text;
	} else if (tag == "password") {
	    user.setPassword(sha1Crypt(text));
	} else if (tag == "store") {
	    user.setDefaultStore(iconvStore(text));
	} else if (tag == "employee") {
	    user.setDefaultEmployee(iconvEmployee(text));
	} else if (tag == "securityType") {
	    user.setSecurityType(iconvSecurityType(text));
	} else if (tag == "screen") {
	    user.setScreen(text);
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, user)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(user)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importVendor(QDomNodeList& nodes, bool isUpdate)
{
    Vendor orig, vendor;

    _type = tr("Vendor");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag.left(3) == "old" && isUpdate) {
	    if (findVendor(text, vendor)) {
		orig = vendor;
		_name = text;
	    }
	} else if (tag == "externalId") {
	    vendor.setExternalId(iconvFixed(text));
	} else if (tag == "firstName") {
	    vendor.setFirstName(text.left(30));
	    _name = vendor.name();
	} else if (tag == "lastName") {
	    vendor.setLastName(text.left(30));
	    _name = vendor.name();
	} else if (tag == "name") {
	    vendor.setLastName(text.left(30));
	    _name = vendor.name();
	} else if (tag == "number") {
	    vendor.setNumber(text);
	} else if (tag == "company") {
	    vendor.setCompany(iconvBoolean(text));
	} else if (tag == "street") {
	    vendor.setStreet(text);
	} else if (tag == "street2") {
	    vendor.setStreet2(text);
	} else if (tag == "city") {
	    vendor.setCity(text);
	} else if (tag == "province") {
	    vendor.setProvince(text);
	} else if (tag == "country") {
	    vendor.setCountry(text);
	} else if (tag == "postal") {
	    vendor.setPostal(text);
	} else if (tag == "phone") {
	    vendor.setPhoneNumber(text);
	} else if (tag == "fax") {
	    vendor.setFaxNumber(text);
	} else if (tag == "email") {
	    vendor.setEmail(text);
	} else if (tag == "webPage") {
	    vendor.setWebPage(text);
	} else if (tag == "contact") {
	    vendor.setContact(text);
	} else if (tag == "comments") {
	    vendor.setComments(text);
	} else if (tag == "account") {
	    vendor.setAccountId(iconvAccount(text));
	} else if (tag == "terms") {
	    vendor.setTermsId(iconvTerms(text));
	} else if (tag == "backorders") {
	    vendor.setBackorders(iconvBoolean(text));
	} else if (tag == "group" || tag == "groupInsert") {
	    Id group_id = iconvGroup(text, Group::VENDOR);
	    if (group_id != INVALID_ID) {
		bool found = false;
		for (unsigned int i = 0; i < vendor.groups().size(); ++i) {
		    if (vendor.groups()[i] == group_id) {
			found = true;
			break;
		    }
		}
		if (!found)
		    vendor.groups().push_back(group_id);
	    } else {
		logWarning(tr("Unknown group: %1").arg(text));
	    }
	} else if (tag == "groupDelete" && isUpdate) {
	    Id group_id = iconvGroup(text, Group::VENDOR);
	    if (group_id != INVALID_ID) {
		bool found = false;
		for (unsigned int i = 0; i < vendor.groups().size(); ++i) {
		    if (vendor.groups()[i] == group_id) {
			found = true;
			vendor.groups().erase(vendor.groups().begin() + i);
			break;
		    }
		}
		if (!found)
		    logWarning(tr("Group not found: %1").arg(text));
	    } else {
		logWarning(tr("Unknown group: %1").arg(text));
	    }
	} else if (tag == "groupClear" && isUpdate) {
	    vendor.groups().clear();
	} else if (tag == "extra") {
	    Extra extra;
	    if (findExtra("Vendor", e.attribute("name"), extra))
		vendor.setValue(extra.id(), text);
	    else
		logWarning(tr("Unknown extra: %1,%21")
			   .arg("Vendor").arg(e.attribute("name")));
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (isUpdate) {
	if (!_db->update(orig, vendor)) {
	    logQuasarErrors(tr("Update failed"));
	} else {
	    logInfo(tr("Data updated"));
	}
    } else {
	if (!_db->create(vendor)) {
	    logQuasarErrors(tr("Create failed"));
	} else {
	    logInfo(tr("Data created"));
	}
    }

    return true;
}

bool
DataImport::importJournalEntry(QDomNodeList& nodes)
{
    General general;

    _type = tr("Journal Entry");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "number") {
	    general.setNumber(text);
	    _name = text;
	} else if (tag == "reference") {
	    general.setReference(text);
	    _name = text;
	} else if (tag == "date") {
	    general.setPostDate(iconvDate(text));
	} else if (tag == "time") {
	    general.setPostTime(iconvTime(text));
	} else if (tag == "memo") {
	    general.setMemo(text);
	} else if (tag == "station") {
	    Station station;
	    if (findStation(text, station))
		general.setStationId(station.id());
	    else
		logWarning(tr("Unknown station: %1").arg(text));
	} else if (tag == "employee") {
	    Employee employee;
	    if (findEmployee(text, employee))
		general.setEmployeeId(employee.id());
	    else
		logWarning(tr("Unknown employee: %1").arg(text));
	} else if (tag == "store") {
	    Store store;
	    if (findStore(text, store))
		general.setStoreId(store.id());
	    else
		logWarning(tr("Unknown store: %1").arg(text));
	} else if (tag == "account") {
	    Account account;
	    if (findAccount(text, account)) {
		AccountLine line;
		line.account_id = account.id();
		line.amount = iconvFixed(e.attribute("amount", "0.0"));
		general.accounts().push_back(line);
	    } else {
		logWarning(tr("Unknown account: %1").arg(text));
	    }
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    if (general.postDate().isNull())
	general.setPostDate(QDate::currentDate());
    if (general.postTime().isNull())
	general.setPostTime(QTime::currentTime());

    if (!_db->create(general)) {
	logQuasarErrors(tr("Create failed"));
    } else {
	_name = general.number();
	logInfo(tr("Data created"));
    }

    return true;
}

bool
DataImport::importCardAdjustment(QDomNodeList& nodes)
{
    CardAdjust adjust;
    fixed remain;
    bool isCustomer = false;
    bool isVendor = false;

    _type = tr("Card Adjustment");
    _name = tr("Unknown");

    // Start with blank account line since line with receivables or
    // payables account must be first in list.
    adjust.accounts().push_back(AccountLine());
    AccountLine& firstLine = adjust.accounts()[0];

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "number") {
	    adjust.setNumber(text);
	    _name = text;
	} else if (tag == "reference") {
	    adjust.setReference(text);
	    _name = text;
	} else if (tag == "date") {
	    adjust.setPostDate(iconvDate(text));
	} else if (tag == "time") {
	    adjust.setPostTime(iconvTime(text));
	} else if (tag == "memo") {
	    adjust.setMemo(text);
	} else if (tag == "station") {
	    Station station;
	    if (findStation(text, station))
		adjust.setStationId(station.id());
	    else
		logWarning(tr("Unknown station: %1").arg(text));
	} else if (tag == "employee") {
	    Employee employee;
	    if (findEmployee(text, employee))
		adjust.setEmployeeId(employee.id());
	    else
		logWarning(tr("Unknown employee: %1").arg(text));
	} else if (tag == "store") {
	    Store store;
	    if (findStore(text, store))
		adjust.setStoreId(store.id());
	    else
		logWarning(tr("Unknown store: %1").arg(text));
	} else if (tag == "customer") {
	    if (isCustomer)
		logWarning(tr("Customer can only be set once"));
	    if (isVendor)
		logWarning(tr("Only one of customer or vendor can be used"));
	    isCustomer = true;

	    Customer customer;
	    if (findCustomer(text, customer)) {
		adjust.setCardId(customer.id());
		firstLine.account_id = customer.accountId();
	    } else {
		logWarning(tr("Unknown customer: %1").arg(text));
	    }
	} else if (tag == "vendor") {
	    if (isVendor)
		logWarning(tr("Vendor can only be set once"));
	    if (isCustomer)
		logWarning(tr("Only one of customer or vendor can be used"));
	    isVendor = true;

	    Vendor vendor;
	    if (findVendor(text, vendor)) {
		adjust.setCardId(vendor.id());
		firstLine.account_id = vendor.accountId();
	    } else {
		logWarning(tr("Unknown vendor: %1").arg(text));
	    }
	} else if (tag == "amount") {
	    firstLine.amount = iconvFixed(text);
	    remain = firstLine.amount;
	} else if (tag == "account") {
	    Account account;
	    if (findAccount(text, account)) {
		fixed amount = remain;
		if (!e.attribute("amount").isEmpty())
		    amount = iconvFixed(e.attribute("amount"));
		remain -= amount;

		AccountLine line;
		line.account_id = account.id();
		line.amount = amount;
		adjust.accounts().push_back(line);
	    } else {
		logWarning(tr("Unknown account: %1").arg(text));
	    }
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    // Card balance change
    CardLine cline;
    cline.card_id = adjust.cardId();
    cline.amount = adjust.accounts()[0].amount;
    adjust.cards().push_back(cline);
    adjust.setAmount(cline.amount);

    // Fix up signs as needed
    if (isCustomer) {
	for (unsigned int i = 1; i < adjust.accounts().size(); ++i)
	    adjust.accounts()[i].amount *= -1;
    } else if (isVendor) {
	adjust.accounts()[0].amount *= -1;
    }

    if (adjust.postDate().isNull())
	adjust.setPostDate(QDate::currentDate());
    if (adjust.postTime().isNull())
	adjust.setPostTime(QTime::currentTime());

    if (!_db->create(adjust)) {
	logQuasarErrors(tr("Create failed"));
    } else {
	_name = adjust.number();
	logInfo(tr("Data created"));
    }

    return true;
}


static void
addTax(Invoice& invoice, fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	int found = -1;
	for (unsigned int j = 0; j < invoice.taxes().size(); ++j) {
	    TaxLine& line = invoice.taxes()[j];
	    if (line.tax_id == tax_id) {
		found = j;
		break;
	    }
	}

	if (found >= 0) {
	    invoice.taxes()[found].taxable += taxable;
	    invoice.taxes()[found].amount += tax_amt;
	} else {
	    TaxLine line(tax_id, taxable, tax_amt);
	    invoice.taxes().push_back(line);
	}
    }
}

static void
addTaxInc(Invoice& invoice,fixed taxable, vector<Id>& ids, vector<fixed>& amts)
{
    for (unsigned int i = 0; i < ids.size(); ++i) {
	Id tax_id = ids[i];
	fixed tax_amt = amts[i];

	int found = -1;
	for (unsigned int j = 0; j < invoice.taxes().size(); ++j) {
	    TaxLine& line = invoice.taxes()[j];
	    if (line.tax_id == tax_id) {
		found = j;
		break;
	    }
	}

	if (found >= 0) {
	    invoice.taxes()[found].inc_taxable += taxable;
	    invoice.taxes()[found].inc_amount += tax_amt;
	} else {
	    TaxLine line(tax_id, 0.0, 0.0, taxable, tax_amt);
	    invoice.taxes().push_back(line);
	}
    }
}

typedef QPair<Id, fixed> InfoPair;

static void
addInfo(vector<InfoPair>& info, Id id, fixed amount)
{
    if (id == INVALID_ID) return;
    for (unsigned int i = 0; i < info.size(); ++i) {
	if (info[i].first == id) {
	    info[i].second += amount;
	    return;
	}
    }
    info.push_back(InfoPair(id, amount));
}

bool
DataImport::importCustomerInvoice(QDomNodeList& nodes)
{
    Invoice invoice;

    _type = tr("Customer Invoice");
    _name = tr("Unknown");

    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "number") {
	    invoice.setNumber(text);
	    _name = text;
	} else if (tag == "reference") {
	    invoice.setReference(text);
	    _name = text;
	} else if (tag == "date") {
	    invoice.setPostDate(iconvDate(text));
	} else if (tag == "time") {
	    invoice.setPostTime(iconvTime(text));
	} else if (tag == "memo") {
	    invoice.setMemo(text);
	} else if (tag == "comment") {
	    invoice.setComment(text);
	} else if (tag == "shipVia") {
	    invoice.setShipVia(text);
	} else if (tag == "terms") {
	    invoice.setTermsId(iconvTerms(text));
	} else if (tag == "taxExempt") {
	    invoice.setTaxExemptId(iconvTax(text));
	} else if (tag == "station") {
	    Station station;
	    if (findStation(text, station))
		invoice.setStationId(station.id());
	    else
		logWarning(tr("Unknown station: %1").arg(text));
	} else if (tag == "employee") {
	    Employee employee;
	    if (findEmployee(text, employee))
		invoice.setEmployeeId(employee.id());
	    else
		logWarning(tr("Unknown employee: %1").arg(text));
	} else if (tag == "store") {
	    Store store;
	    if (findStore(text, store))
		invoice.setStoreId(store.id());
	    else
		logWarning(tr("Unknown store: %1").arg(text));
	} else if (tag == "customer") {
	    Customer customer;
	    if (findCustomer(text, customer)) {
		invoice.setCustomerId(customer.id());
		invoice.setCustomerAddress(customer.address());
		invoice.setTermsId(customer.termsId());
		invoice.setTaxExemptId(customer.taxExemptId());
	    } else {
		logWarning(tr("Unknown customer: %1").arg(text));
	    }
	} else if (tag == "shipTo") {
	    Customer customer;
	    if (findCustomer(text, customer)) {
		invoice.setShipId(customer.id());
		invoice.setShipAddress(customer.address());
	    } else {
		logWarning(tr("Unknown shipTo customer: %1").arg(text));
	    }
	} else if (tag == "item") {
	    Item item;
	    if (findItem(text, item)) {
		InvoiceItem line;
		line.item_id = item.id();
		line.number = text;
		line.description = item.description();
		line.quantity = iconvFixed(e.attribute("qty", "1"));
		line.tax_id = item.sellTax();
		line.item_tax_id = item.sellTax();
		line.include_tax = item.priceIncludesTax();
		line.include_deposit = item.priceIncludesDeposit();
		line.open_dept = item.isOpenDept();
		line.discountable = item.isDiscountable();

		// Get size for item
		line.size = e.attribute("size");
		if (line.size.isEmpty())
		    line.size = item.numberSize(line.number);
		if (line.size.isEmpty())
		    line.size = item.sellSize();
		line.size_qty = item.sizeQty(line.size);

		// Get the price and extended price
		if (!e.attribute("extPrice").isEmpty()) {
		    line.ext_price = iconvFixed(e.attribute("extPrice"));
		    if (line.quantity != 0)
			line.price.setPrice(line.ext_price / line.quantity);
		} else if (!e.attribute("price").isEmpty()) {
		    line.price.setPrice(iconvFixed(e.attribute("price")));
		    line.ext_price = line.price.calculate(line.quantity);
		} else {
		    Id store_id = invoice.storeId();
		    QString size = line.size;
		    line.price = item.price(store_id, size);
		    line.ext_price = line.price.calculate(line.quantity);
		}

		// Set the deposit
		line.ext_deposit = item.deposit() * line.quantity *
		    line.size_qty;

		// Calculate selling cost
		Id storeId = invoice.storeId();
		if (line.open_dept) {
		    fixed margin = item.targetGM(storeId, line.size) / 100.0;
		    line.inv_cost = line.ext_price - (margin * line.ext_price);
		} else {
		    fixed sign = line.quantity.sign();
		    fixed qty = line.quantity * sign;
		    fixed sell_price = line.ext_price * sign;
		    fixed sell_cost;
		    _db->itemSellingCost(item, line.size, storeId, qty,
					 sell_price, sell_cost);
		    line.inv_cost = sell_cost * sign;
		}

		// Tax over-ride
		QString tax = e.attribute("tax");
		if (tax.lower() == "none")
		    line.tax_id = INVALID_ID;
		else if (!tax.isEmpty())
		    line.tax_id = iconvTax(tax);

		invoice.items().push_back(line);
	    } else {
		logWarning(tr("Unknown item: %1").arg(text));
	    }
	} else if (tag == "tender") {
	    Tender tender;
	    if (findTender(text, tender)) {
		TenderLine line;
		line.tender_id = tender.id();
		line.amount = iconvFixed(e.attribute("amount", "0"));
		line.conv_rate = tender.convertRate();
		line.conv_amt = line.amount * line.conv_rate;
		invoice.tenders().push_back(line);
	    } else {
		logWarning(tr("Unknown tender: %1").arg(text));
	    }
	} else {
	    logWarning(tr("Unknown tag: %1").arg(tag));
	}
    }

    // Not handled yet:
    //    - discounts
    //    - customer references
    //    - customer return
    //    - account line type invoices
    //    - promised date

    if (invoice.postDate().isNull())
	invoice.setPostDate(QDate::currentDate());
    if (invoice.postTime().isNull())
	invoice.setPostTime(QTime::currentTime());

    // TODO: calculate item costs and other info
    // TODO: calculate totals and account postings

    ObjectCache cache(_db);
    _db->prepare(invoice, cache);

    if (!_db->create(invoice)) {
	logQuasarErrors(tr("Create failed"));
    } else {
	_name = invoice.number();
	logInfo(tr("Data created"));
    }

    return true;
}

bool
DataImport::iconvBoolean(const QString& text)
{
    return (text.lower() == "yes");
}

int
DataImport::iconvInt(const QString& text)
{
    bool ok;
    int value = text.toInt(&ok);
    if (!ok)
	logWarning(tr("Invalid integer: %1").arg(text));
    return value;
}

fixed
DataImport::iconvFixed(const QString& text)
{
    bool ok;
    double value = text.toDouble(&ok);
    if (!ok)
	logWarning(tr("Invalid number: %1").arg(text));
    return value;
}

Price
DataImport::iconvPrice(const QString& text)
{
    Price price;
    if (!price.fromString(text))
	logWarning(tr("Invalid price: %1").arg(text));
    return price;
}

QDate
DataImport::iconvDate(const QString& text)
{
    QDate date = QDate::fromString(text, Qt::ISODate);
    if (date.isNull())
	logWarning(tr("Invalid date: %1").arg(text));
    return date;
}

QTime
DataImport::iconvTime(const QString& text)
{
    QTime time = QTime::fromString(text);
    if (time.isNull())
	logWarning(tr("Invalid time: %1").arg(text));
    return time;
}

Id
DataImport::iconvAccount(const QString& text)
{
    Account account;
    if (findAccount(text, account))
	return account.id();

    return INVALID_ID;
}

Id
DataImport::iconvCompany(const QString& text)
{
    Company company;
    if (findCompany(text, company))
	return company.id();

    logWarning(tr("Unknown company: %1").arg(text));
    return INVALID_ID;
}

Id
DataImport::iconvCustomer(const QString& text)
{
    Customer customer;
    if (findCustomer(text, customer))
	return customer.id();

    logWarning(tr("Unknown customer: %1").arg(text));
    return INVALID_ID;
}

Id
DataImport::iconvDept(const QString& text)
{
    Dept dept;
    if (findDept(text, dept))
	return dept.id();

    logWarning(tr("Unknown department: %1").arg(text));
    return INVALID_ID;
}

Id
DataImport::iconvDiscount(const QString& text)
{
    Discount discount;
    if (findDiscount(text, discount))
	return discount.id();
    return INVALID_ID;
}

Id
DataImport::iconvEmployee(const QString& text)
{
    Employee employee;
    if (findEmployee(text, employee))
	return employee.id();

    logWarning(tr("Unknown employee: %1").arg(text));
    return INVALID_ID;
}

Id
DataImport::iconvGroup(const QString& text, int type)
{
    Group group;
    if (findGroup(text, type, group))
	return group.id();
    return INVALID_ID;
}

Id
DataImport::iconvPatGroup(const QString& text)
{
    PatGroup group;
    if (findPatGroup(text, group))
	return group.id();
    return INVALID_ID;
}

Id
DataImport::iconvSecurityType(const QString& text)
{
    SecurityType security;
    if (findSecurityType(text, security))
	return security.id();

    logWarning(tr("Unknown security type: %1").arg(text));
    return INVALID_ID;
}

Id
DataImport::iconvStation(const QString& text)
{
    Station station;
    if (findStation(text, station))
	return station.id();

    logWarning(tr("Unknown station: %1").arg(text));
    return INVALID_ID;
}

Id
DataImport::iconvStore(const QString& text)
{
    Store store;
    if (findStore(text, store))
	return store.id();

    logWarning(tr("Unknown store: %1").arg(text));
    return INVALID_ID;
}

Id
DataImport::iconvSubdept(const QString& text)
{
    Subdept subdept;
    if (findSubdept(text, subdept))
	return subdept.id();

    logWarning(tr("Unknown subdepartment: %1").arg(text));
    return INVALID_ID;
}

Id
DataImport::iconvTax(const QString& text)
{
    if (text.lower() != "none") {
	Tax tax;
	if (findTax(text, tax))
	    return tax.id();
    }
    return INVALID_ID;
}

Id
DataImport::iconvTerms(const QString& text)
{
    if (text.lower() != "none") {
	Term term;
	if (findTerm(text, term))
	    return term.id();
	logWarning(tr("Unknown terms: %1").arg(text));
    }
    return INVALID_ID;
}

Id
DataImport::iconvVendor(const QString& text)
{
    Vendor vendor;
    if (findVendor(text, vendor))
	return vendor.id();

    logWarning(tr("Unknown vendor: %1").arg(text));
    return INVALID_ID;
}

bool
DataImport::findAccount(const QString& name, Account& account)
{
    AccountSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Account> accounts;
    _db->select(accounts, select);

    if (accounts.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(accounts, select);
    }

    if (accounts.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(accounts, select);
    }

    if (accounts.size() != 1) {
	logWarning(tr("Unknown account: %1").arg(name));
	return false;
    }

    account = accounts[0];
    return true;
}

bool
DataImport::findAdjustReason(const QString& name, AdjustReason& reason)
{
    AdjustReasonSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<AdjustReason> reasons;
    _db->select(reasons, select);

    if (reasons.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(reasons, select);
    }

    if (reasons.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(reasons, select);
    }

    if (reasons.size() != 1) return false;
    reason = reasons[0];
    return true;
}

bool
DataImport::findCharge(const QString& name, Charge& charge)
{
    ChargeSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Charge> charges;
    if (!_db->select(charges, select)) return false;

    if (charges.size() == 0) {
	select.name = "";
	select.external_id = iconvFixed(name);
	_db->select(charges, select);
    }

    if (charges.size() != 1) return false;

    charge = charges[0];
    return true;
}

bool
DataImport::findCompany(const QString& name, Company& company)
{
    CompanySelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Company> companies;
    _db->select(companies, select);

    if (companies.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(companies, select);
    }

    if (companies.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(companies, select);
    }

    if (companies.size() != 1) return false;
    company = companies[0];
    return true;
}

bool
DataImport::findCustomer(const QString& name, Customer& customer)
{
    CustomerSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Customer> customers;
    _db->select(customers, select);

    if (customers.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(customers, select);
    }

    if (customers.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(customers, select);
    }

    if (customers.size() != 1) return false;
    customer = customers[0];
    return true;
}

bool
DataImport::findCustomerType(const QString& name, CustomerType& type)
{
    CustomerTypeSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<CustomerType> types;
    _db->select(types, select);

    if (types.size() == 0) {
	select.name = "";
	select.external_id = iconvFixed(name);
	_db->select(types, select);
    }

    if (types.size() != 1) return false;
    type = types[0];
    return true;
}

bool
DataImport::findDept(const QString& name, Dept& dept)
{
    DeptSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Dept> depts;
    _db->select(depts, select);

    if (depts.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(depts, select);
    }

    if (depts.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(depts, select);
    }

    if (depts.size() != 1) return false;
    dept = depts[0];
    return true;
}

bool
DataImport::findDiscount(const QString& name, Discount& discount)
{
    DiscountSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Discount> discounts;
    if (!_db->select(discounts, select)) return false;

    if (discounts.size() == 0) {
	select.name = "";
	select.external_id = iconvFixed(name);
	_db->select(discounts, select);
    }

    if (discounts.size() != 1) return false;
    discount = discounts[0];
    return true;
}

bool
DataImport::findEmployee(const QString& name, Employee& employee)
{
    EmployeeSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Employee> employees;
    _db->select(employees, select);

    if (employees.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(employees, select);
    }

    if (employees.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(employees, select);
    }

    if (employees.size() != 1) return false;
    employee = employees[0];
    return true;
}

bool
DataImport::findExpense(const QString& name, Expense& expense)
{
    ExpenseSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Expense> expenses;
    if (!_db->select(expenses, select)) return false;

    if (expenses.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(expenses, select);
    }

    if (expenses.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(expenses, select);
    }

    if (expenses.size() != 1) return false;
    expense = expenses[0];
    return true;
}

bool
DataImport::findExtra(const QString& table, const QString& name, Extra& extra)
{
    ExtraSelect select;
    select.activeOnly = true;
    select.table = table;
    select.name = name;

    vector<Extra> extras;
    _db->select(extras, select);
    if (extras.size() != 1) {
	return false;
    }

    extra = extras[0];
    return true;
}

bool
DataImport::findGroup(const QString& name, int type, Group& group)
{
    GroupSelect select;
    select.activeOnly = true;
    select.name = name;
    select.type = type;

    vector<Group> groups;
    _db->select(groups, select);
    if (groups.size() != 1) {
	logWarning(tr("Unknown group: %1").arg(name));
	return false;
    }

    group = groups[0];
    return true;
}

bool
DataImport::findItem(const QString& number, Item& item)
{
    if (number.isEmpty())
	return false;

    ItemSelect conditions;
    conditions.activeOnly = true;
    conditions.checkOrderNum = true;

    vector<Item> items;
    if (!_db->lookup(conditions, number, items)) {
	ItemSelect select;
	select.activeOnly = true;
	select.external_id = iconvFixed(number);
	_db->select(items, select);
	if (items.size() != 1) return false;
    }

    item = items[0];
    return true;
}

bool
DataImport::findLocation(const QString& name, Id store_id, Location& location)
{
    LocationSelect select;
    select.activeOnly = true;
    select.name = name;
    select.store_id = store_id;
    vector<Location> locations;
    if (!_db->select(locations, select)) return false;
    if (locations.size() != 1) return false;

    location = locations[0];
    return true;
}

bool
DataImport::findOrder(const QString& number, Order& order)
{
    OrderSelect select;
    select.activeOnly = true;
    select.number = number;
    vector<Order> orders;
    if (!_db->select(orders, select)) return false;
    if (orders.size() != 1) return false;

    order = orders[0];
    return true;
}

bool
DataImport::findPatGroup(const QString& name, PatGroup& group)
{
    PatGroupSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<PatGroup> groups;
    _db->select(groups, select);

    if (groups.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(groups, select);
    }

    if (groups.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(groups, select);
    }

    if (groups.size() != 1) return false;
    group = groups[0];
    return true;
}

bool
DataImport::findPersonal(const QString& name, Personal& personal)
{
    PersonalSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Personal> personals;
    _db->select(personals, select);

    if (personals.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(personals, select);
    }

    if (personals.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(personals, select);
    }

    if (personals.size() != 1) return false;
    personal = personals[0];
    return true;
}

bool
DataImport::findSecurityType(const QString& name, SecurityType& security)
{
    SecurityTypeSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<SecurityType> types;
    _db->select(types, select);

    if (types.size() != 1) return false;
    security = types[0];
    return true;
}

bool
DataImport::findSlip(const QString& number, Slip& slip)
{
    SlipSelect select;
    select.activeOnly = true;
    select.number = number;
    vector<Slip> slips;
    if (!_db->select(slips, select)) return false;
    if (slips.size() != 1) return false;

    slip = slips[0];
    return true;
}

bool
DataImport::findStation(const QString& name, Station& station)
{
    StationSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Station> stations;
    _db->select(stations, select);

    if (stations.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(stations, select);
    }

    if (stations.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(stations, select);
    }

    if (stations.size() != 1) return false;
    station = stations[0];
    return true;
}

bool
DataImport::findStore(const QString& name, Store& store)
{
    StoreSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Store> stores;
    _db->select(stores, select);

    if (stores.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(stores, select);
    }

    if (stores.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(stores, select);
    }

    if (stores.size() != 1) return false;
    store = stores[0];
    return true;
}

bool
DataImport::findSubdept(const QString& name, Subdept& subdept)
{
    SubdeptSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Subdept> subdepts;
    _db->select(subdepts, select);

    if (subdepts.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(subdepts, select);
    }

    if (subdepts.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(subdepts, select);
    }

    if (subdepts.size() != 1) return false;
    subdept = subdepts[0];
    return true;
}

bool
DataImport::findTax(const QString& name, Tax& tax)
{
    TaxSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Tax> taxes;
    _db->select(taxes, select);
    if (taxes.size() != 1) {
	logWarning(tr("Unknown tax: %1").arg(name));
	return false;
    }

    tax = taxes[0];
    return true;
}

bool
DataImport::findTemplate(const QString& name, OrderTemplate& templ)
{
    OrderTemplateSelect select;
    select.activeOnly = true;
    select.name = name;
    vector<OrderTemplate> templs;
    if (!_db->select(templs, select)) return false;
    if (templs.size() != 1) return false;

    templ = templs[0];
    return true;
}

bool
DataImport::findTender(const QString& name, Tender& tender)
{
    TenderSelect select;
    select.activeOnly = true;
    select.name = name;
    vector<Tender> tenders;
    if (!_db->select(tenders, select)) return false;
    if (tenders.size() != 1) return false;

    tender = tenders[0];
    return true;
}

bool
DataImport::findTerm(const QString& name, Term& term)
{
    TermSelect select;
    select.activeOnly = true;
    select.name = name;
    vector<Term> terms;
    if (!_db->select(terms, select)) return false;
    if (terms.size() != 1) return false;

    term = terms[0];
    return true;
}

bool
DataImport::findUser(const QString& name, User& user)
{
    return _db->lookup(name, user);
}

bool
DataImport::findVendor(const QString& name, Vendor& vendor)
{
    VendorSelect select;
    select.activeOnly = true;
    select.name = name;

    vector<Vendor> vendors;
    _db->select(vendors, select);

    if (vendors.size() == 0) {
	select.name = "";
	select.number = name;
	_db->select(vendors, select);
    }

    if (vendors.size() == 0) {
	select.number = "";
	select.external_id = iconvFixed(name);
	_db->select(vendors, select);
    }

    if (vendors.size() != 1) return false;
    vendor = vendors[0];
    return true;
}
