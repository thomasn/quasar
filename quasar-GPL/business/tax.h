// $Id: tax.h,v 1.15 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef TAX_H
#define TAX_H

#include "data_object.h"

class Tax: public DataObject {
public:
    // Constructors and Destructor
    Tax();
    ~Tax();

    // Get methods
    const QString& name()		const { return _name; }
    const QString& description()	const { return _description; }
    bool isGroup()			const { return _group; }
    const QString& number()		const { return _number; }
    fixed rate()			const { return _rate; }
    Id collectedAccount()		const { return _collected_id; }
    Id paidAccount()			const { return _paid_id; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setDescription(const QString& desc)	{ _description = desc; }
    void setGroup(bool flag)			{ _group = flag; }
    void setNumber(const QString& number)	{ _number = number; }
    void setRate(fixed rate)			{ _rate = rate; }
    void setCollectedAccount(Id id)		{ _collected_id = id; }
    void setPaidAccount(Id id)			{ _paid_id = id; }

    // Tax ids for tax on tax
    vector<Id>& tot_ids()			{ return _tot_ids; }
    const vector<Id>& tot_ids() const		{ return _tot_ids; }

    // Tax ids for group taxes
    vector<Id>& group_ids()			{ return _group_ids; }
    const vector<Id>& group_ids() const		{ return _group_ids; }

    // Operations
    bool operator==(const Tax& rhs) const;
    bool operator!=(const Tax& rhs) const;

protected:
    QString _name;		// Short name (3 letters)
    QString _description;	// Description
    bool _group;		// Group of taxes?
    QString _number;		// Number (shown on invoice)
    fixed _rate;		// Rate to charge
    Id _collected_id;		// Account for collected taxes
    Id _paid_id;		// Account for paid taxes
    vector<Id> _tot_ids;	// Taxes in tax-on-tax list
    vector<Id> _group_ids;	// Taxes in group (if _group = true)
};

#endif // TAX_H
