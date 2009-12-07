// $Id: location.h,v 1.9 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef LOCATION_H
#define LOCATION_H

#include "data_object.h"

class Location: public DataObject {
public:
    // Constructors and Destructor
    Location();
    ~Location();

    // Get methods
    const QString& name()		const { return _name; }
    Id storeId()			const { return _store_id; }
    const QString& section()		const { return _section; }
    const QString& fixture()		const { return _fixture; }
    const QString& bin()		const { return _bin; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setStoreId(Id store_id)		{ _store_id = store_id; }
    void setSection(const QString& section)	{ _section = section; }
    void setFixture(const QString& fixture)	{ _fixture = fixture; }
    void setBin(const QString& bin)		{ _bin = bin; }

    // Special methods
    QString canonicalName() const;

    // Operations
    bool operator==(const Location& rhs) const;
    bool operator!=(const Location& rhs) const;

private:
    QString _name;		// Name of location
    Id _store_id;		// Store for location
    QString _section;		// Highest level (ie: Sporting Goods)
    QString _fixture;		// Middle level (ie: Aisle 3)
    QString _bin;		// Lowest level (ie: A1)
};

#endif // LOCATION_H
