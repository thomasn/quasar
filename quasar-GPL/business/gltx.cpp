// $Id: gltx.cpp,v 1.70 2005/03/04 18:00:56 bpepers Exp $
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

#include "gltx.h"
#include <assert.h>

Gltx::Gltx()
    : _number("#"), _reference(""), _memo(""), _printed(false),
      _paid(false), _amount(0.0)
{
}

Gltx::~Gltx()
{
}

bool
Gltx::operator==(const Gltx& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._number != _number) return false;
    if (rhs._reference != _reference) return false;
    if (rhs._post_date != _post_date) return false;
    if (rhs._post_time != _post_time) return false;
    if (rhs._memo != _memo) return false;
    if (rhs._station_id != _station_id) return false;
    if (rhs._employee_id != _employee_id) return false;
    if (rhs._card_id != _card_id) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._shift_id != _shift_id) return false;
    if (rhs._link_id != _link_id) return false;
    if (rhs._printed != _printed) return false;
    // NOTE: paid and amount aren't compared here since they are internal
    if (rhs._accounts != _accounts) return false;
    if (rhs._cards != _cards) return false;
    if (rhs._taxes != _taxes) return false;
    if (rhs._payments != _payments) return false;
    if (rhs._tenders != _tenders) return false;
    if (rhs._refName != _refName) return false;
    if (rhs._refData != _refData) return false;
    return true;
}

bool
Gltx::operator!=(const Gltx& rhs) const
{
    return !(*this == rhs);
}

fixed
Gltx::cardTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _cards.size(); ++i)
	total += _cards[i].amount;
    return total;
}

fixed
Gltx::taxTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _taxes.size(); ++i)
	total += _taxes[i].amount + _taxes[i].inc_amount;
    return total;
}

fixed
Gltx::paymentTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _payments.size(); ++i)
	total += _payments[i].amount + _payments[i].discount;
    return total;
}

fixed
Gltx::termsDiscountTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _payments.size(); ++i)
	total += _payments[i].discount;
    return total;
}

// NOTE: the implementation of Gltx::item routines are provided but all
// will assert since no meaningful implementation can occur here and they
// shouldn't be called

unsigned int
Gltx::itemCnt() const
{
    return 0;
}

const ItemLine&
Gltx::item(unsigned int) const
{
    assert(false);
    return *((ItemLine*)NULL);
}

fixed
Gltx::costTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < itemCnt(); ++i) {
	if (item(i).voided) continue;
	total += item(i).inv_cost;
    }
    return total;
}

fixed
Gltx::depositTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < itemCnt(); ++i) {
	if (item(i).voided) continue;
	total += item(i).ext_deposit;
    }
    return total;
}

// This method is virtual and should always be subclassed.
fixed
Gltx::total() const
{
    return costTotal() + depositTotal() + taxTotal();
}

AccountLine::AccountLine()
    : memo("")
{
}

AccountLine::AccountLine(Id _account_id, fixed _amount)
    : account_id(_account_id), amount(_amount), memo("")
{
}

bool
AccountLine::operator==(const AccountLine& rhs) const
{
    if (account_id != rhs.account_id) return false;
    if (amount != rhs.amount) return false;
    if (memo != rhs.memo) return false;
    if (cleared != rhs.cleared) return false;
    return true;
}

bool
AccountLine::operator!=(const AccountLine& rhs) const
{
    return !(*this == rhs);
}

CardLine::CardLine()
{
}

CardLine::CardLine(Id _card_id, fixed _amount)
    : card_id(_card_id), amount(_amount)
{
}

bool
CardLine::operator==(const CardLine& rhs) const
{
    if (card_id != rhs.card_id) return false;
    if (amount != rhs.amount) return false;
    return true;
}

bool
CardLine::operator!=(const CardLine& rhs) const
{
    return !(*this == rhs);
}

ItemLine::ItemLine()
    : description(""), size(""), voided(false)
{
}

bool
ItemLine::operator==(const ItemLine& rhs) const
{
    if (item_id != rhs.item_id) return false;
    if (account_id != rhs.account_id) return false;
    if (number != rhs.number) return false;
    if (description != rhs.description) return false;
    if (size != rhs.size) return false;
    if (size_qty != rhs.size_qty) return false;
    if (quantity != rhs.quantity) return false;
    if (inv_cost != rhs.inv_cost) return false;
    if (sale_price != rhs.sale_price) return false;
    if (ext_deposit != rhs.ext_deposit) return false;
    if (voided != rhs.voided) return false;
    return true;
}

bool
ItemLine::operator!=(const ItemLine& rhs) const
{
    return !(*this == rhs);
}

TaxLine::TaxLine(Id id, fixed able, fixed amt, fixed i_able, fixed i_amt)
    : tax_id(id), taxable(able), amount(amt), inc_taxable(i_able),
      inc_amount(i_amt)
{
}

bool
TaxLine::operator==(const TaxLine& rhs) const
{
    if (tax_id != rhs.tax_id) return false;
    if (taxable != rhs.taxable) return false;
    if (amount != rhs.amount) return false;
    if (inc_taxable != rhs.inc_taxable) return false;
    if (inc_amount != rhs.inc_amount) return false;
    return true;
}

bool
TaxLine::operator!=(const TaxLine& rhs) const
{
    return !(*this == rhs);
}

PaymentLine::PaymentLine()
    : gltx_id(INVALID_ID), amount(0.0), discount(0.0)
{
}

PaymentLine::PaymentLine(Id _gltx_id, fixed _amount, fixed _discount)
    : gltx_id(_gltx_id), amount(_amount), discount(_discount)
{
}

bool
PaymentLine::operator==(const PaymentLine& rhs) const
{
    if (gltx_id != rhs.gltx_id) return false;
    if (amount != rhs.amount) return false;
    if (discount != rhs.discount) return false;
    return true;
}

bool
PaymentLine::operator!=(const PaymentLine& rhs) const
{
    return !(*this == rhs);
}

// Used to sort in date and number order
bool operator<(const Gltx& lhs, const Gltx& rhs) {
    if (lhs.postDate() != rhs.postDate())
	return lhs.postDate() < rhs.postDate();

    bool ok;
    double ln = lhs.number().toDouble(&ok);
    if (ok) {
	double rn = rhs.number().toDouble(&ok);
	if (ok) return ln < rn;
    }

    return lhs.number() < rhs.number();
}

fixed
Gltx::tenderTotal() const
{
    fixed total = 0.0;
    for (unsigned int i = 0; i < _tenders.size(); ++i) {
	if (_tenders[i].voided) continue;
	total += _tenders[i].amount;
    }
    return total;
}

fixed
Gltx::chargeTotal() const
{
    return total() - tenderTotal();
}

TenderLine::TenderLine()
    : tender_id(INVALID_ID), amount(0.0), conv_rate(0.0), conv_amt(0.0),
      card_num(""), auth_num(""), is_change(false), voided(false)
{
}

bool
TenderLine::operator==(const TenderLine& rhs) const
{
    if (tender_id != rhs.tender_id) return false;
    if (amount != rhs.amount) return false;
    if (conv_rate != rhs.conv_rate) return false;
    if (conv_amt != rhs.conv_amt) return false;
    if (card_num != rhs.card_num) return false;
    if (expiry_date != rhs.expiry_date) return false;
    if (auth_num != rhs.auth_num) return false;
    if (is_change != rhs.is_change) return false;
    if (voided != rhs.voided) return false;
    return true;
}

bool
TenderLine::operator!=(const TenderLine& rhs) const
{
    return !(*this == rhs);
}
