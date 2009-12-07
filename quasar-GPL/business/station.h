// $Id: station.h,v 1.6 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef STATION_H
#define STATION_H

#include "data_object.h"

class Station: public DataObject {
public:
    // Constructors and Destructor
    Station();
    ~Station();

    // General
    const QString& name()		const { return _name; }
    const QString& number()		const { return _number; }
    Id storeId()			const { return _store_id; }
    fixed offlineNumber()		const { return _offline_num; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setNumber(const QString& number)	{ _number = number; }
    void setStoreId(Id store_id)		{ _store_id = store_id; }
    void setOfflineNumber(fixed number)		{ _offline_num = number; }

    // Operations
    bool operator==(const Station& rhs) const;
    bool operator!=(const Station& rhs) const;

protected:
    QString _name;		// Name
    QString _number;		// Unique station number
    Id _store_id;		// Store station works in
    fixed _offline_num;		// Next offline number for POS
};

#endif // STATION_H
