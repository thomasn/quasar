// $Id: pat_group.h,v 1.5 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef PAT_GROUP_H
#define PAT_GROUP_H

#include "data_object.h"

class PatGroup: public DataObject {
public:
    // Constructors and Destructor
    PatGroup();
    ~PatGroup();

    // Get methods
    const QString& name()		const { return _name; }
    const QString& number()		const { return _number; }
    Id equityId()			const { return _equity_id; }
    Id creditId()			const { return _credit_id; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setNumber(const QString& number)	{ _number = number; }
    void setEquityId(Id card_id)		{ _equity_id = card_id; }
    void setCreditId(Id card_id)		{ _credit_id = card_id; }

    // Cards in group
    vector<Id>& card_ids()			{ return _card_ids; }
    const vector<Id>& card_ids() const		{ return _card_ids; }

    // Operations
    bool operator==(const PatGroup& rhs) const;
    bool operator!=(const PatGroup& rhs) const;

protected:
    QString _name;		// Group name
    QString _number;		// Group number (unique)
    Id _equity_id;		// Card for equity allocation
    Id _credit_id;		// Card for credit allocation
    vector<Id> _card_ids;	// Cards in group (Customer or Vendor)
};

#endif // PAT_GROUP_H
