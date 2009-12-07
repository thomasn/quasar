// $Id: data_object.cpp,v 1.55 2005/01/08 10:52:41 bpepers Exp $
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

#include "data_object.h"
#include "data_model.h"

#include <qdir.h>
#include <qapplication.h>
#define tr(text) qApp->translate("DataObject",text)

DataObject::DataObject()
    : _id(INVALID_ID), _version(-1), _created_by(""), _updated_by(""),
      _inactive_by(""), _external_id(-1)
{
}

DataObject::~DataObject()
{
    _id = INVALID_ID;
    _version = -1;
}

QString
DataObject::value(Id extra_id) const
{
    for (unsigned int i = 0; i < _extra.size(); ++i)
	if (_extra[i].first == extra_id)
	    return _extra[i].second;
    return QString::null;
}

void
DataObject::setValue(Id extra_id, const QString& value)
{
    for (unsigned int i = 0; i < _extra.size(); ++i) {
	if (_extra[i].first == extra_id) {
	    _extra[i].second = value;
	    return;
	}
    }
    _extra.push_back(DataPair(extra_id, value));
}

bool
DataObject::isActive() const
{
    return _inactive_by.isEmpty();
}

bool
DataObject::isInactive() const
{
    return !isActive();
}

QString
DataObject::dataTypeName() const
{
    return dataTypeName(dataType());
}

QString
DataObject::dataTypeName(int type)
{
    switch (type) {
    case ACCOUNT:	return tr("Account");
    case ITEM_ADJUST:	return tr("Item Adjustment");
    case CHEQUE:	return tr("Cheque");
    case CUSTOMER:	return tr("Customer");
    case DEPT:		return tr("Dept");
    case EMPLOYEE:	return tr("Employee");
    case GENERAL:	return tr("Journal Entry");
    case INVOICE:	return tr("Customer Invoice");
    case ITEM:		return tr("Item");
    case ORDER:		return tr("Purchase Order");
    case PERSONAL:	return tr("Personal");
    case RECEIPT:	return tr("Customer Payment");
    case RECEIVE:	return tr("Vendor Invoice");
    case REPORT:	return tr("Report");
    case SUBDEPT:	return tr("Subdept");
    case TAX:		return tr("Tax");
    case TENDER:	return tr("Tender");
    case TODO:		return tr("Todo");
    case VENDOR:	return tr("Vendor");
    case EXPENSE:	return tr("Expense");
    case NOSALE:	return tr("Nosale");
    case PAYOUT:	return tr("Payout");
    case RETURN:	return tr("Customer Return");
    case SHIFT:		return tr("Shift");
    case TEND_ADJUST:	return tr("Tender Adjustment");
    case RECONCILE:	return tr("Reconcile");
    case ITEM_PRICE:	return tr("Price");
    case TERM:		return tr("Terms");
    case PLU:		return tr("Plu");
    case STORE:		return tr("Store");
    case DISCOUNT:	return tr("Discount");
    case SLIP:		return tr("Receiving");
    case CHARGE:	return tr("PR Charge");
    case COUNT:		return tr("Physical Count");
    case LOCATION:	return tr("Shelf Location");
    case QUOTE:		return tr("Customer Quote");
    case EXTRA:		return tr("Extra Data");
    case STATION:	return tr("Station");
    case CLAIM:		return tr("Vendor Claim");
    case RECURRING:	return tr("Recurring Trans.");
    case GROUP:		return tr("Group");
    case CARD_ADJUST:	return tr("Card Adjustment");
    case WITHDRAW:	return tr("Withdraw");
    case CUST_TYPE:	return tr("Customer Type");
    case TEND_COUNT:	return tr("Tender Count");
    case SECURITY_TYPE:	return tr("Security Type");
    case PO_TEMPLATE:	return tr("Order Template");
    case PAT_GROUP:	return tr("Pat Group");
    case PAT_WS:	return tr("Pat Worksheet");
    case LABEL_BATCH:	return tr("Label Batch");
    case PRICE_BATCH:	return tr("Price Batch");
    case PROMO_BATCH:	return tr("Promo Batch");
    case ADJUST_REASON:	return tr("Adjustment Reason");
    }
    return tr("Invalid Data Type");
}

int
DataObject::dataType(const QString& name)
{
    if (name == tr("Account")) return ACCOUNT;
    if (name == tr("Item Adjustment")) return ITEM_ADJUST;
    if (name == tr("Cheque")) return CHEQUE;
    if (name == tr("Customer")) return CUSTOMER;
    if (name == tr("Dept")) return DEPT;
    if (name == tr("Employee")) return EMPLOYEE;
    if (name == tr("Journal Entry")) return GENERAL;
    if (name == tr("Customer Invoice")) return INVOICE;
    if (name == tr("Item")) return ITEM;
    if (name == tr("Purchase Order")) return ORDER;
    if (name == tr("Personal")) return PERSONAL;
    if (name == tr("Customer Payment")) return RECEIPT;
    if (name == tr("Vendor Invoice")) return RECEIVE;
    if (name == tr("Report")) return REPORT;
    if (name == tr("Subdept")) return SUBDEPT;
    if (name == tr("Tax")) return TAX;
    if (name == tr("Tender")) return TENDER;
    if (name == tr("Todo")) return TODO;
    if (name == tr("Vendor")) return VENDOR;
    if (name == tr("Expense")) return EXPENSE;
    if (name == tr("Nosale")) return NOSALE;
    if (name == tr("Payout")) return PAYOUT;
    if (name == tr("Customer Return")) return RETURN;
    if (name == tr("Shift")) return SHIFT;
    if (name == tr("Tender Adjustment")) return TEND_ADJUST;
    if (name == tr("Reconcile")) return RECONCILE;
    if (name == tr("Price")) return ITEM_PRICE;
    if (name == tr("Terms")) return TERM;
    if (name == tr("Plu")) return PLU;
    if (name == tr("Store")) return STORE;
    if (name == tr("Discount")) return DISCOUNT;
    if (name == tr("Receiving")) return SLIP;
    if (name == tr("PR Charge")) return CHARGE;
    if (name == tr("Physical Count")) return COUNT;
    if (name == tr("Shelf Location")) return LOCATION;
    if (name == tr("Customer Quote")) return QUOTE;
    if (name == tr("Extra Data")) return EXTRA;
    if (name == tr("Station")) return STATION;
    if (name == tr("Vendor Claim")) return CLAIM;
    if (name == tr("Recurring Trans.")) return RECURRING;
    if (name == tr("Group")) return GROUP;
    if (name == tr("Card Adjustment")) return CARD_ADJUST;
    if (name == tr("Withdraw")) return WITHDRAW;
    if (name == tr("Customer Type")) return CUST_TYPE;
    if (name == tr("Tender Count")) return TEND_COUNT;
    if (name == tr("Security Type")) return SECURITY_TYPE;
    if (name == tr("Order Template")) return PO_TEMPLATE;
    if (name == tr("Pat Group")) return PAT_GROUP;
    if (name == tr("Pat Worksheet")) return PAT_WS;
    if (name == tr("Label Batch")) return LABEL_BATCH;
    if (name == tr("Price Batch")) return PRICE_BATCH;
    if (name == tr("Promo Batch")) return PROMO_BATCH;
    if (name == tr("Adjustment Reason")) return ADJUST_REASON;

    qFatal(tr("Unknown type name: ") + name);
    return ACCOUNT;
}

bool
DataObject::error(const QString& message)
{
    qWarning("Error: " + message);
    _last_error = message;
    return false;
}

bool
DataObject::operator==(const DataObject& rhs) const
{
    // NOTE: don't compare id or version in here
    if (rhs._data_type != _data_type) return false;
    if (rhs._created_by != _created_by) return false;
    if (rhs._updated_by != _updated_by) return false;
    if (rhs._inactive_by != _inactive_by) return false;
    if (rhs._created_on != _created_on) return false;
    if (rhs._updated_on != _updated_on) return false;
    if (rhs._inactive_on != _inactive_on) return false;
    if (rhs._external_id != _external_id) return false;
    if (rhs._extra != _extra) return false;
    return true;
}

bool
DataObject::operator!=(const DataObject& rhs) const
{
    return !(*this == rhs);
}
