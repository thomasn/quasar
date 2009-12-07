// $Id: plu.h,v 1.2 2004/02/03 00:13:26 arandell Exp $
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

#ifndef PLU_H
#define PLU_H

#include "id.h"

class Plu {
public:
    // Constructors and Destructor
    Plu();
    Plu(Id item_id, const QString& number);
    ~Plu();

    // Get methods
    Id itemId()				const { return _item_id; }
    QString number()			const { return _number; }

    // Set methods
    void setItemId(Id item_id)			{ _item_id = item_id; }
    void setNumber(const QString& number)	{ _number = number; }

    // Operations
    bool operator==(const Plu& rhs) const;
    bool operator!=(const Plu& rhs) const;

    // Convert to/from string
    QString toString() const;
    bool fromString(const QString& text);

protected:
    Id _item_id;		// Link to item table
    QString _number;		// Item number to use
};

#endif // PLU_H
