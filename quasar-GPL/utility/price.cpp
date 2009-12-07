// $Id: price.cpp,v 1.6 2005/03/13 23:14:19 bpepers Exp $
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

#include "price.h"
#include "money_valcon.h"
#include <qstringlist.h>

Price::Price()
{
    setNull();
}

Price::Price(fixed price, fixed for_qty, fixed for_price)
    : _unit_price(price), _for_qty(for_qty), _for_price(for_price)
{
}

Price::~Price()
{
}

fixed
Price::price() const
{
    if (_for_qty != 0.0 && _unit_price == 0.0)
	return _for_price / _for_qty;
    return _unit_price;
}

void
Price::setPrice(fixed price)
{
    _unit_price = price;
    _for_qty = 0.0;
    _for_price = 0.0;
}

bool
Price::isNull() const
{
    return _unit_price == 0.0 && _for_qty == 0.0 && _for_price != 0.0;
}

void
Price::setNull()
{
    _unit_price = 0.0;
    _for_qty = 0.0;
    _for_price = 1.0;
}

bool
Price::operator==(const Price& rhs) const
{
    if (rhs._unit_price != _unit_price) return false;
    if (rhs._for_qty != _for_qty) return false;
    if (_for_qty != 0.0 && rhs._for_price != _for_price) return false;
    return true;
}

bool
Price::operator!=(const Price& rhs) const
{
    return !(*this == rhs);
}

bool
Price::operator>(const Price& rhs) const
{
    return price() > rhs.price();
}

bool
Price::operator>=(const Price& rhs) const
{
    return price() >= rhs.price();
}

bool
Price::operator<(const Price& rhs) const
{
    return price() < rhs.price();
}

bool
Price::operator<=(const Price& rhs) const
{
    return price() <= rhs.price();
}

fixed
Price::calculate(fixed qty) const
{
    fixed ext_price = 0.0;
    fixed sign = 1.0;
    if (qty < 0) {
	sign = -1.0;
	qty = -qty;
    }

    // Use full at/for price groups
    if (_for_qty != 0.0) {
	while (qty >= _for_qty) {
	    ext_price += _for_price;
	    qty -= _for_qty;
	}
    }

    // Use unit price or for qty/price for rest
    if (qty > 0.0) {
	if (_unit_price != 0.0) {
	    ext_price += _unit_price * qty;
	} else if (_for_qty != 0.0) {
	    fixed singlePrice = _for_price / _for_qty;
	    singlePrice.moneyRound();
	    ext_price += singlePrice * qty;
	}
    }

    ext_price.moneyRound();
    return ext_price * sign;
}

Price
Price::markup(fixed percent) const
{
    Price newPrice;
    newPrice._unit_price = _unit_price + _unit_price * percent / 100;
    newPrice._for_qty = _for_qty;
    newPrice._for_price = _for_price + _for_price * percent / 100;

    newPrice._unit_price.moneyRound();
    newPrice._for_price.moneyRound();

    return newPrice;
}

Price
Price::margin(fixed percent) const
{
    Price newPrice;
    newPrice._unit_price = _unit_price / (100.0 - percent) * 100;
    newPrice._for_qty = _for_qty;
    newPrice._for_price = _for_price / (100.0 - percent) * 100;

    newPrice._unit_price.moneyRound();
    newPrice._for_price.moneyRound();

    return newPrice;
}

QString
Price::toString() const
{
    if (isNull()) return "";
    if (_for_qty == 0.0) return MoneyValcon().format(_unit_price);

    QString text;
    if (_unit_price != 0.0)
	text = MoneyValcon().format(_unit_price) + " ";
    text += _for_qty.toString() + "/" + MoneyValcon().format(_for_price);

    return text;
}

bool
Price::fromString(const QString& text)
{
    QString simpleText = text.simplifyWhiteSpace();
    NumberValcon numberValcon;
    MoneyValcon moneyValcon;

    setNull();
    if (simpleText.isEmpty()) return true;

    QString money, atFor;
    int spaceCount = simpleText.contains(' ');
    if (spaceCount == 0) {
	// Trivial parsing - just check for slash
	if (simpleText.find('/') == -1)
	    money = simpleText;
	else
	    atFor = simpleText;
    } else if (spaceCount == 1) {
	// Little more complicated since space may be from money format
	if (moneyValcon.parse(simpleText)) {
	    money = simpleText;
	} else {
	    QStringList list = QStringList::split(" ", simpleText);
	    money = list[0];
	    atFor = list[1];
	}
    } else {
	// Full blown complicated with multiple spaces
	int pos = -1;
	bool found = false;
	while (true) {
	    pos = simpleText.find(' ', pos + 1);
	    if (pos == -1) break;

	    money = simpleText.left(pos);
	    atFor = simpleText.mid(pos + 1);

	    if (!moneyValcon.parse(money)) continue;
	    QStringList list = QStringList::split("/", atFor);
	    if (list.count() != 2) continue;
	    if (!numberValcon.parse(list[0])) continue;
	    if (!moneyValcon.parse(list[1])) continue;

	    found = true;
	    break;
	}
	if (!found) return false;
    }

    if (!money.isEmpty()) {
	if (!moneyValcon.parse(money)) return false;
	_unit_price = moneyValcon.getValue();
    }

    if (!atFor.isEmpty()) {
	QStringList list = QStringList::split("/", atFor);
	if (list.count() != 2) return false;
	if (!numberValcon.parse(list[0])) return false;
	if (!moneyValcon.parse(list[1])) return false;
	_for_qty = numberValcon.getValue();
	_for_price = moneyValcon.getValue();
    }

    return true;
}
