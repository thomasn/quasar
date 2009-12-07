// $Id: charge.h,v 1.7 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef CHARGE_H
#define CHARGE_H

#include "data_object.h"

class Charge: public DataObject {
public:
    // Constructors and Destructor
    Charge();
    ~Charge();

    enum Method { MANUAL, NONE=0, COST, WEIGHT };

    // Get methods
    const QString& name()		const { return _name; }
    Id accountId()			const { return _account_id; }
    Id taxId()				const { return _tax_id; }
    Method calculateMethod()		const { return _calc_method; }
    fixed amount()			const { return _amount; }
    Method allocateMethod()		const { return _alloc_method; }
    bool includeTax()			const { return _inc_tax; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setAccountId(Id account_id)		{ _account_id = account_id; }
    void setTaxId(Id tax_id)			{ _tax_id = tax_id; }
    void setCalculateMethod(Method method)	{ _calc_method = method; }
    void setAmount(fixed amount)		{ _amount = amount; }
    void setAllocateMethod(Method method)	{ _alloc_method = method; }
    void setIncludeTax(bool flag)		{ _inc_tax = flag; }

    // Operations
    bool operator==(const Charge& rhs) const;
    bool operator!=(const Charge& rhs) const;

protected:
    QString _name;		// Name of charge
    Id _account_id;		// GL account
    Id _tax_id;			// Tax to charge
    Method _calc_method;	// Calculation method
    fixed _amount;		// Percent for Cost, dollar for Weight
    Method _alloc_method;	// Allocation method
    bool _inc_tax;		// Amount includes the tax?
};

#endif // CHARGE_H
