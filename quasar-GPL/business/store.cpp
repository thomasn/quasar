// $Id: store.cpp,v 1.13 2005/03/15 15:33:59 bpepers Exp $
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

#include "store.h"

Store::Store()
    : _name(""), _number(""), _contact(""), _has_addr(false), _street(""),
      _street2(""), _city(""), _province(""), _country(""), _postal(""),
      _phone_num(""), _phone2_num(""), _fax_num(""), _email(""),
      _web_page(""), _can_sell(true)
{
    _data_type = STORE;
}

Store::~Store()
{
}

QString
Store::address(bool includeExtra) const
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

bool
Store::operator==(const Store& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._number != _number) return false;
    if (rhs._contact != _contact) return false;
    if (rhs._has_addr != _has_addr) return false;
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
    if (rhs._can_sell != _can_sell) return false;
    if (rhs._year_end_id != _year_end_id) return false;
    if (rhs._company_id != _company_id) return false;
    return true;
}

bool
Store::operator!=(const Store& rhs) const
{
    return !(*this == rhs);
}
