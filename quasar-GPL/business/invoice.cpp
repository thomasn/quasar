// $Id: invoice.cpp,v 1.43 2005/01/11 08:23:25 bpepers Exp $
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

#include "invoice.h"

Invoice::Invoice()
    : _customer_addr(""), _ship_id(INVALID_ID), _ship_addr(""),
      _ship_via(""), _term_id(INVALID_ID), _comment(""),
      _line_type(Item)
{
    _data_type = INVOICE;
}

Invoice::~Invoice()
{
}

bool
Invoice::operator==(const Invoice& rhs) const
{
    if ((const Gltx&)rhs != *this) return false;
    if (rhs._customer_addr != _customer_addr) return false;
    if (rhs._ship_id != _ship_id) return false;
    if (rhs._ship_addr != _ship_addr) return false;
    if (rhs._ship_via != _ship_via) return false;
    if (rhs._term_id != _term_id) return false;
    if (rhs._comment != _comment) return false;
    if (rhs._promised != _promised) return false;
    if (rhs._line_type != _line_type) return false;
    if (rhs._tax_exempt_id != _tax_exempt_id) return false;
    if (rhs._items != _items) return false;
    if (rhs._discounts != _discounts) return false;
    return true;
}

bool
Invoice::operator!=(const Invoice& rhs) const
{
    return !(*this == rhs);
}

bool
Invoice::isReturn() const
{
    return _data_type == RETURN;
}

void
Invoice::setReturn(bool flag)
{
    if (flag)
	_data_type = RETURN;
    else
	_data_type = INVOICE;
}

QString
Invoice::lineTypeName() const
{
    return lineTypeName(_line_type);
}

QString
Invoice::lineTypeName(LineType type)
{
    switch (type) {
    case Item:		return "item";
    case Account:	return "account";
    }
    return "Invalid Line Type";
}

fixed
Invoice::priceTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _items.size(); ++i) {
	if (_items[i].voided) continue;
	total += _items[i].ext_base;
    }
    return total;
}

fixed
Invoice::lineDiscountTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _items.size(); ++i) {
	if (_items[i].voided) continue;
	total += _items[i].line_disc.total_amt;
    }
    return total;
}

fixed
Invoice::transDiscountTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _discounts.size(); ++i) {
	if (_discounts[i].voided) continue;
	total += _discounts[i].total_amt;
    }
    return total;
}

fixed
Invoice::itemDiscountTotal() const
{
    return lineDiscountTotal() + transDiscountTotal();
}

fixed
Invoice::tenderTotal() const
{
    fixed total = Gltx::tenderTotal();
    return total;
}

fixed
Invoice::chargeTotal() const
{
    fixed total = Gltx::chargeTotal();
    return total;
}

fixed
Invoice::total() const
{
    fixed total = priceTotal() - itemDiscountTotal() + depositTotal() +
	taxTotal();
    if (isReturn()) return -total;
    return total;
}

fixed
Invoice::paidTotal() const
{
    return paymentTotal() + tenderTotal();
}

fixed
Invoice::dueTotal() const
{
    return total() - paidTotal();
}

InvoiceItem::InvoiceItem()
    : price_id(INVALID_ID), price_disc(0.0), ext_price(0.0), ext_base(0.0),
      ext_tax(0.0), tax_id(INVALID_ID), item_tax_id(INVALID_ID),
      include_tax(false), include_deposit(false), discountable(true),
      subdept_id(INVALID_ID), scale(false), you_save(0.0), open_dept(false)
{
}

bool
InvoiceItem::operator==(const InvoiceItem& rhs) const
{
    if ((const ItemLine&)rhs != *this) return false;
    if (price_id != rhs.price_id) return false;
    if (price_disc != rhs.price_disc) return false;
    if (price != rhs.price) return false;
    if (ext_price != rhs.ext_price) return false;
    if (ext_base != rhs.ext_base) return false;
    if (ext_tax != rhs.ext_tax) return false;
    if (tax_id != rhs.tax_id) return false;
    if (item_tax_id != rhs.item_tax_id) return false;
    if (include_tax != rhs.include_tax) return false;
    if (include_deposit != rhs.include_deposit) return false;
    if (discountable != rhs.discountable) return false;
    if (subdept_id != rhs.subdept_id) return false;
    if (scale != rhs.scale) return false;
    if (you_save != rhs.you_save) return false;
    if (open_dept != rhs.open_dept) return false;
    return true;
}

bool
InvoiceItem::operator!=(const InvoiceItem& rhs) const
{
    return !(*this == rhs);
}

InvoiceDisc::InvoiceDisc()
    : discount_id(INVALID_ID), method(0), amount(0.0),
      account_id(INVALID_ID), total_amt(0.0), voided(false)
{
}

bool
InvoiceDisc::operator==(const InvoiceDisc& rhs) const
{
    if (discount_id != rhs.discount_id) return false;
    if (method != rhs.method) return false;
    if (amount != rhs.amount) return false;
    if (account_id != rhs.account_id) return false;
    if (total_amt != rhs.total_amt) return false;
    if (voided != rhs.voided) return false;
    return true;
}

bool
InvoiceDisc::operator!=(const InvoiceDisc& rhs) const
{
    return !(*this == rhs);
}
