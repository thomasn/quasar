// $Id: tender.cpp,v 1.30 2004/12/30 00:07:58 bpepers Exp $
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

#include "tender.h"

#include <qapplication.h>
#define tr(text) qApp->translate("Tender",text)

Tender::Tender()
    : _name(""), _type(CASH), _limit(0.0), _convert_rate(1.0),
      _over_tender(true), _open_drawer(true), _force_amt(false),
      _second_rcpt(false), _menu_num(-1)
{
    _data_type = TENDER;
}

Tender::~Tender()
{
}

bool
Tender::operator==(const Tender& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._type != _type) return false;
    if (rhs._limit != _limit) return false;
    if (rhs._convert_rate != _convert_rate) return false;
    if (rhs._over_tender != _over_tender) return false;
    if (rhs._open_drawer != _open_drawer) return false;
    if (rhs._force_amt != _force_amt) return false;
    if (rhs._second_rcpt != _second_rcpt) return false;
    if (rhs._account_id != _account_id) return false;
    if (rhs._safe_id != _safe_id) return false;
    if (rhs._bank_id != _bank_id) return false;
    if (rhs._menu_num != _menu_num) return false;
    if (rhs._ranges != _ranges) return false;
    if (rhs._denoms != _denoms) return false;
    if (rhs._rounding != _rounding) return false;
    return true;
}

bool
Tender::operator!=(const Tender& rhs) const
{
    return !(*this == rhs);
}

QString
Tender::typeName() const
{
    return typeName(_type);
}

QString
Tender::typeName(Type type)
{
    switch (type) {
    case CASH:		return tr("Cash");
    case CHEQUE:	return tr("Cheque");
    case CARD:		return tr("Card");
    }
    return tr("Invalid tender type");
}

bool
TenderRange::operator==(const TenderRange& rhs) const
{
    if (rhs.range_start != range_start) return false;
    if (rhs.range_end != range_end) return false;
    return true;
}

bool
TenderRange::operator!=(const TenderRange& rhs) const
{
    return !(*this == rhs);
}

bool
TenderDenom::operator==(const TenderDenom& rhs) const
{
    if (rhs.name != name) return false;
    if (rhs.multiplier != multiplier) return false;
    return true;
}

bool
TenderDenom::operator!=(const TenderDenom& rhs) const
{
    return !(*this == rhs);
}

bool
TenderRound::operator==(const TenderRound& rhs) const
{
    if (rhs.ends_in != ends_in) return false;
    if (rhs.change != change) return false;
    return true;
}

bool
TenderRound::operator!=(const TenderRound& rhs) const
{
    return !(*this == rhs);
}
