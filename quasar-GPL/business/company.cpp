// $Id: company.cpp,v 1.36 2005/03/13 22:12:54 bpepers Exp $
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

#include "company.h"
#include "money_valcon.h"

Company::Company()
    : _name(""), _number(""), _street(""), _street2(""), _city(""),
      _province(""), _country(""), _postal(""), _phone_num(""),
      _phone2_num(""), _fax_num(""), _email(""), _web_page(""),
      _shift_method(BY_STATION), _ly_closed(true)
{
    _data_type = COMPANY;
}

Company::~Company()
{
}

QString
Company::address(bool includeExtra) const
{
    QString text = _name;
    if (_street != "") {
	if (text != "") text += "\n";
	text += _street;
    }
    if (_street2 != "") {
	if (text != "") text += "\n";
	text += _street2;
    }
    if (_city != "" || _province != "") {
	if (text != "") text += "\n";
	text += _city;
	if (_city != "" && _province != "") text += ", ";
	text += _province;
    }
    if (_country != "" || _postal != "") {
	if (text != "") text += "\n";
	text += _country;
	if (_country != "" && _postal != "") text += "  ";
	text += _postal;
    }

    // Add on phone, fax, email, web if requested
    if (includeExtra) {
	if (_phone_num != "") {
	    if (text != "") text += "\n";
	    text += "Phone: " + _phone_num;
	}
	if (_fax_num != "") {
	    if (text != "") text += "\n";
	    text += "Fax: " + _fax_num;
	}
	if (_email != "") {
	    if (text != "") text += "\n";
	    text += "E-mail: " + _email;
	}
	if (_web_page != "") {
	    if (text != "") text += "\n";
	    text += "Web Page: " + _web_page;
	}
    }

    return text;
}

fixed
Company::priceRound(fixed price) const
{
    QString priceText = MoneyValcon().format(price);

    for (unsigned int i = 0; i < _rounding.size(); ++i) {
	QString endsIn = _rounding[i].endsIn;
	fixed addAmt = _rounding[i].addAmt;

	int len = endsIn.length();
	if (len == 0) continue;

	if (priceText.right(len) == endsIn)
	    return price + addAmt;
    }

    return price;
}

Price
Company::priceRound(Price price) const
{
    Price newPrice = price;
    newPrice.setUnitPrice(priceRound(price.unitPrice()));
    newPrice.setForPrice(priceRound(price.forPrice()));
    return newPrice;
}

bool
Company::operator==(const Company& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._number != _number) return false;
    if (rhs._street != _street) return false;
    if (rhs._street2 != _street2) return false;
    if (rhs._city != _city) return false;
    if (rhs._province != _province) return false;
    if (rhs._country != _country) return false;
    if (rhs._postal != _postal) return false;
    if (rhs._phone_num != _phone_num) return false;
    if (rhs._phone2_num != _phone2_num) return false;
    if (rhs._fax_num != _fax_num) return false;
    if (rhs._email != _email) return false;
    if (rhs._web_page != _web_page) return false;
    if (rhs._store_id != _store_id) return false;
    if (rhs._shift_method != _shift_method) return false;
    if (rhs._year_start != _year_start) return false;
    if (rhs._ly_closed != _ly_closed) return false;
    if (rhs._close_date != _close_date) return false;
    if (rhs._re_account != _re_account) return false;
    if (rhs._hb_account != _hb_account) return false;
    if (rhs._bank_account != _bank_account) return false;
    if (rhs._customer_acct != _customer_acct) return false;
    if (rhs._vendor_acct != _vendor_acct) return false;
    if (rhs._c_terms_acct != _c_terms_acct) return false;
    if (rhs._v_terms_acct != _v_terms_acct) return false;
    if (rhs._deposit_acct != _deposit_acct) return false;
    if (rhs._transfer_acct != _transfer_acct) return false;
    if (rhs._physical_acct != _physical_acct) return false;
    if (rhs._split_account != _split_account) return false;
    if (rhs._charge_acct != _charge_acct) return false;
    if (rhs._over_short_id != _over_short_id) return false;
    if (rhs._last_sc != _last_sc) return false;
    if (rhs._safe_store_id != _safe_store_id) return false;
    if (rhs._safe_station_id != _safe_station_id) return false;
    if (rhs._safe_employee_id != _safe_employee_id) return false;
    if (rhs._rounding != _rounding) return false;
    return true;
}

bool
Company::operator!=(const Company& rhs) const
{
    return !(*this == rhs);
}

RoundingRule::RoundingRule()
    : endsIn(""), addAmt(0.0)
{
}

bool
RoundingRule::operator==(const RoundingRule& rhs) const
{
    if (endsIn != rhs.endsIn) return false;
    if (addAmt != rhs.addAmt) return false;
    return true;
}

bool
RoundingRule::operator!=(const RoundingRule& rhs) const
{
    return !(*this == rhs);
}
