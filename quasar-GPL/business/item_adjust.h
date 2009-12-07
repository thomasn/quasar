// $Id: item_adjust.h,v 1.15 2005/03/13 22:14:54 bpepers Exp $
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

#ifndef ITEM_ADJUST_H
#define ITEM_ADJUST_H

#include "gltx.h"

class ItemAdjust: public Gltx {
public:
    // Constructors and Destructor
    ItemAdjust();
    virtual ~ItemAdjust();

    // Get methods
    Id reasonId()			const { return _reason_id; }
    Id accountId()			const { return _account_id; }

    // Set methods
    void setReasonId(Id id)			{ _reason_id = id; }
    void setAccountId(Id id)			{ _account_id = id; }

    // Item lines
    vector<ItemLine>& items()			{ return _items; }
    const vector<ItemLine>& items() const	{ return _items; }
    unsigned int itemCnt() const		{ return _items.size(); }
    const ItemLine& item(unsigned int i) const	{ return _items[i]; }

    // Operations
    bool operator==(const ItemAdjust& rhs) const;
    bool operator!=(const ItemAdjust& rhs) const;

protected:
    Id _reason_id;		// Reason for adjustment
    Id _account_id;		// Account to expense to
    vector<ItemLine> _items;	// Items being adjusted
};

#endif // ITEM_ADJUST_H
