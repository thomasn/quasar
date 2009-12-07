// $Id: shift.h,v 1.8 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef SHIFT_H
#define SHIFT_H

#include "gltx.h"

class Shift: public Gltx {
public:
    // Constructors and Destructor
    Shift();
    virtual ~Shift();

    // Get methods
    Id adjustmentId() const			{ return _adjust_id; }
    Id transferId() const			{ return _transfer_id; }

    // Set methods
    void setAdjustmentId(Id id)			{ _adjust_id = id; }
    void setTransferId(Id id)			{ _transfer_id = id; }

    // Special methods
    fixed total() const;

    // Operations
    bool operator==(const Shift& rhs) const;
    bool operator!=(const Shift& rhs) const;

protected:
    Id _adjust_id;		// Over/short adjustment
    Id _transfer_id;		// Transfer to safe
};

#endif // SHIFT_H
