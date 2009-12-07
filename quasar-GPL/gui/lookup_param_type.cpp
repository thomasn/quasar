// $Id: lookup_param_type.cpp,v 1.8 2005/03/13 22:18:59 bpepers Exp $
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

#include "lookup_param_type.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "lookup_edit.h"

#include "account.h"
#include "company.h"
#include "customer.h"
#include "dept.h"
#include "employee.h"
#include "group.h"
#include "item.h"
#include "item_select.h"
#include "station.h"
#include "store.h"
#include "subdept.h"
#include "tax.h"
#include "tender.h"
#include "vendor.h"

#include "account_lookup.h"
#include "adjust_reason_lookup.h"
#include "charge_lookup.h"
#include "count_lookup.h"
#include "customer_lookup.h"
#include "customer_type_lookup.h"
#include "dept_lookup.h"
#include "discount_lookup.h"
#include "employee_lookup.h"
#include "expense_lookup.h"
#include "group_lookup.h"
#include "invoice_lookup.h"
#include "item_lookup.h"
#include "gltx_lookup.h"
#include "location_lookup.h"
#include "order_lookup.h"
#include "order_template_lookup.h"
#include "pat_group_lookup.h"
#include "personal_lookup.h"
#include "quote_lookup.h"
#include "receive_lookup.h"
#include "security_type_lookup.h"
#include "slip_lookup.h"
#include "station_lookup.h"
#include "store_lookup.h"
#include "subdept_lookup.h"
#include "tax_lookup.h"
#include "tender_lookup.h"
#include "term_lookup.h"
#include "vendor_lookup.h"

QuasarClient* LookupParamType::quasar;

LookupParamType::LookupParamType()
{
}

LookupParamType::~LookupParamType()
{
}

LookupParamType*
LookupParamType::clone() const
{
    LookupParamType* type = new LookupParamType();
    *type = *this;
    return type;
}

QWidget*
LookupParamType::getWidget(QWidget* parent)
{
    LookupEdit* widget = new LookupEdit(parent);
    widget->setLength(30);

    QString dataType = _param.getAttribute("dataType").lower();
    if (dataType == "account")
	widget->setLookupWindow(new AccountLookup(quasar, parent));
    else if (dataType == "adjust_reason")
	widget->setLookupWindow(new AdjustReasonLookup(quasar, parent));
    else if (dataType == "charge")
	widget->setLookupWindow(new ChargeLookup(quasar, parent));
    else if (dataType == "count")
	widget->setLookupWindow(new CountLookup(quasar, parent));
    else if (dataType == "customer")
	widget->setLookupWindow(new CustomerLookup(quasar, parent));
    else if (dataType == "customer invoice")
	widget->setLookupWindow(new InvoiceLookup(quasar, parent));
    else if (dataType == "customer type")
	widget->setLookupWindow(new CustomerTypeLookup(quasar, parent));
    else if (dataType == "dept")
	widget->setLookupWindow(new DeptLookup(quasar, parent));
    else if (dataType == "discount")
	widget->setLookupWindow(new DiscountLookup(quasar, parent));
    else if (dataType == "employee")
	widget->setLookupWindow(new EmployeeLookup(quasar, parent));
    else if (dataType == "expense")
	widget->setLookupWindow(new ExpenseLookup(quasar, parent));
    else if (dataType == "group")
	widget->setLookupWindow(new GroupLookup(quasar, parent));
    else if (dataType == "item")
	widget->setLookupWindow(new ItemLookup(quasar, parent));
    else if (dataType == "item adjustment")
	widget->setLookupWindow(new GltxLookup(quasar, parent,
					       DataObject::ITEM_ADJUST));
    else if (dataType == "location")
	widget->setLookupWindow(new LocationLookup(quasar, parent));
    else if (dataType == "purchase order")
	widget->setLookupWindow(new OrderLookup(quasar, parent));
    else if (dataType == "order template")
	widget->setLookupWindow(new OrderTemplateLookup(quasar, parent));
    else if (dataType == "pat group")
	widget->setLookupWindow(new PatGroupLookup(quasar, parent));
    else if (dataType == "personal")
	widget->setLookupWindow(new PersonalLookup(quasar, parent));
    else if (dataType == "quote")
	widget->setLookupWindow(new QuoteLookup(quasar, parent));
    else if (dataType == "security type")
	widget->setLookupWindow(new SecurityTypeLookup(quasar, parent));
    else if (dataType == "slip")
	widget->setLookupWindow(new SlipLookup(quasar, parent));
    else if (dataType == "station")
	widget->setLookupWindow(new StationLookup(quasar, parent));
    else if (dataType == "store")
	widget->setLookupWindow(new StoreLookup(quasar, parent));
    else if (dataType == "subdept")
	widget->setLookupWindow(new SubdeptLookup(quasar, parent));
    else if (dataType == "tax")
	widget->setLookupWindow(new TaxLookup(quasar, parent));
    else if (dataType == "tender")
	widget->setLookupWindow(new TenderLookup(quasar, parent));
    else if (dataType == "terms")
	widget->setLookupWindow(new TermLookup(quasar, parent));
    else if (dataType == "vendor")
	widget->setLookupWindow(new VendorLookup(quasar, parent));
    else if (dataType == "vendor invoice")
	widget->setLookupWindow(new ReceiveLookup(quasar, parent));
    else
	qWarning("Unknown lookup param data type: " + dataType);

    return widget;
}

QVariant
LookupParamType::getValue(QWidget* w)
{
    LookupEdit* widget = (LookupEdit*)w;
    if (widget->getId() == INVALID_ID)
	return QString::null;
    return widget->getId().toString();
}

void
LookupParamType::setValue(QWidget* w, const QVariant& value)
{
    LookupEdit* widget = (LookupEdit*)w;
    if (value.toString().isEmpty())
	widget->setId(INVALID_ID);
    else
	widget->setId(Id::fromStringStatic(value.toString()));
}

void
LookupParamType::setText(QWidget* w, const QString& value)
{
    LookupEdit* widget = (LookupEdit*)w;
    widget->setText(value);
}

bool
LookupParamType::validate(QWidget* w)
{
    LookupEdit* widget = (LookupEdit*)w;
    return widget->valid();
}

bool
LookupParamType::convert(const QString& text, QVariant& value)
{
    if (ReportParamType::convert(text, value))
	return true;

    QString dataType = _param.getAttribute("dataType").lower();

    if (dataType == "account") {
	Account account;
	if (quasar->db()->lookup(text, account)) {
	    value = account.id().toString();
	    return true;
	}

	qWarning("Account not found: " + text);
	return false;
    }

    if (dataType == "company") {
	Company company;
	if (quasar->db()->lookup(text, company)) {
	    value = company.id().toString();
	    return true;
	}

	qWarning("Company not found: " + text);
	return false;
    }

    if (dataType == "customer") {
	Customer customer;
	if (quasar->db()->lookup(text, customer)) {
	    value = customer.id().toString();
	    return true;
	}

	qWarning("Customer not found: " + text);
	return false;
    }

    if (dataType == "dept") {
	Dept dept;
	if (quasar->db()->lookup(text, dept)) {
	    value = dept.id().toString();
	    return true;
	}

	qWarning("Department not found: " + text);
	return false;
    }

    if (dataType == "employee") {
	Employee employee;
	if (quasar->db()->lookup(text, employee)) {
	    value = employee.id().toString();
	    return true;
	}

	qWarning("Company not found: " + text);
	return false;
    }

    if (dataType == "group") {
	Group group;
	if (quasar->db()->lookup(text, group)) {
	    value = group.id().toString();
	    return true;
	}

	qWarning("Group not found: " + text);
	return false;
    }

    if (dataType == "item") {
	ItemSelect conditions;
	vector<Id> item_ids;
	if (quasar->db()->lookup(conditions, text, item_ids)) {
	    if (item_ids.size() > 0) {
		value = item_ids[0].toString();
		return true;
	    }
	}

	qWarning("Item not found: " + text);
	return false;
    }

    if (dataType == "station") {
	Station station;
	if (quasar->db()->lookup(text, station)) {
	    value = station.id().toString();
	    return true;
	}

	qWarning("Station not found: " + text);
	return false;
    }

    if (dataType == "store") {
	Store store;
	if (quasar->db()->lookup(text, store)) {
	    value = store.id().toString();
	    return true;
	}

	qWarning("Store not found: " + text);
	return false;
    }

    if (dataType == "subdept") {
	Subdept subdept;
	if (quasar->db()->lookup(text, subdept)) {
	    value = subdept.id().toString();
	    return true;
	}

	qWarning("Subdepartment not found: " + text);
	return false;
    }

    if (dataType == "tax") {
	Tax tax;
	if (quasar->db()->lookup(text, tax)) {
	    value = tax.id().toString();
	    return true;
	}

	qWarning("Tax not found: " + text);
	return false;
    }

    if (dataType == "tender") {
	Tender tender;
	if (quasar->db()->lookup(text, tender)) {
	    value = tender.id().toString();
	    return true;
	}

	qWarning("Tender not found: " + text);
	return false;
    }

    if (dataType == "vendor") {
	Vendor vendor;
	if (quasar->db()->lookup(text, vendor)) {
	    value = vendor.id().toString();
	    return true;
	}

	qWarning("Vendor not found: " + text);
	return false;
    }

    qWarning("Invalid lookup param type: " + dataType);
    return false;
}

ReportParamType*
createLookupParamType()
{
    return new LookupParamType();
}

