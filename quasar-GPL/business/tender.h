// $Id: tender.h,v 1.21 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef TENDER_H
#define TENDER_H

#include "data_object.h"

struct TenderRange {
    QString range_start;
    QString range_end;

    bool operator==(const TenderRange& rhs) const;
    bool operator!=(const TenderRange& rhs) const;
};

struct TenderDenom {
    QString name;
    fixed multiplier;

    bool operator==(const TenderDenom& rhs) const;
    bool operator!=(const TenderDenom& rhs) const;
};

struct TenderRound {
    QString ends_in;
    fixed change;

    bool operator==(const TenderRound& rhs) const;
    bool operator!=(const TenderRound& rhs) const;
};

class Tender: public DataObject {
public:
    // Constructors and Destructor
    Tender();
    ~Tender();

    enum Type { CASH, CHEQUE, CARD };

    // Get methods
    const QString& name()		const { return _name; }
    Type type()				const { return _type; }
    fixed limit()			const { return _limit; }
    fixed convertRate()			const { return _convert_rate; }
    bool overTender()			const { return _over_tender; }
    bool openDrawer()			const { return _open_drawer; }
    bool forceAmount()			const { return _force_amt; }
    bool secondReceipt()		const { return _second_rcpt; }
    Id accountId()			const { return _account_id; }
    Id safeId()				const { return _safe_id; }
    Id bankId()				const { return _bank_id; }
    int menuNumber()			const { return _menu_num; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setType(Type type)			{ _type = type; }
    void setLimit(fixed limit)			{ _limit = limit; }
    void setConvertRate(fixed rate)		{ _convert_rate = rate; }
    void setOverTender(bool flag)		{ _over_tender = flag; }
    void setOpenDrawer(bool flag)		{ _open_drawer = flag; }
    void setForceAmount(bool flag)		{ _force_amt = flag; }
    void setSecondReceipt(bool flag)		{ _second_rcpt = flag; }
    void setAccountId(Id account_id)		{ _account_id = account_id; }
    void setSafeId(Id safe_id)			{ _safe_id = safe_id; }
    void setBankId(Id bank_id)			{ _bank_id = bank_id; }
    void setMenuNumber(int num)			{ _menu_num = num; }

    // Credit card ranges
    vector<TenderRange>& ranges()		{ return _ranges; }
    const vector<TenderRange>& ranges() const	{ return _ranges; }

    // Denominations for counting
    vector<TenderDenom>& denominations()	{ return _denoms; }
    const vector<TenderDenom>& denominations() const { return _denoms; }

    // Tender rounding
    vector<TenderRound>& rounding()		{ return _rounding; }
    const vector<TenderRound>& rounding() const { return _rounding; }

    // Special methods
    QString typeName() const;
    static QString typeName(Type type);

    // Operations
    bool operator==(const Tender& rhs) const;
    bool operator!=(const Tender& rhs) const;

protected:
    QString _name;		// Name of tender
    Type _type;			// Type
    fixed _limit;		// Maximum allowed in a sale
    fixed _convert_rate;	// Conversion rate
    bool _over_tender;		// Allowed to over tender?
    bool _open_drawer;		// Open drawer?
    bool _force_amt;		// Force cashier to enter amount?
    bool _second_rcpt;		// Print second receipt?
    Id _account_id;		// Transaction account
    Id _safe_id;		// Safe account
    Id _bank_id;		// Bank deposit account
    int _menu_num;		// Menu order in POS
    vector<TenderRange> _ranges;   // Credit card ranges
    vector<TenderDenom> _denoms;   // Denominations (pennies, nickles, ...)
    vector<TenderRound> _rounding; // Tendering rounding (for AUS)
};

#endif // TENDER_H
