// $Id: order_template.h,v 1.8 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef ORDER_TEMPLATE_H
#define ORDER_TEMPLATE_H

#include "data_object.h"

struct TemplateItem {
    TemplateItem();

    Id item_id;
    QString number;
    QString size;

    bool operator==(const TemplateItem& rhs) const;
    bool operator!=(const TemplateItem& rhs) const;
};

struct TemplateCharge {
    TemplateCharge(Id charge_id=INVALID_ID, Id tax_id=INVALID_ID,
		   fixed amount=0.0, bool internal=true);

    Id charge_id;
    Id tax_id;
    fixed amount;
    bool internal;

    bool operator==(const TemplateCharge& rhs) const;
    bool operator!=(const TemplateCharge& rhs) const;
};

class OrderTemplate: public DataObject {
public:
    // Constructors and Destructor
    OrderTemplate();
    ~OrderTemplate();

    // Get methods
    const QString& name()		const { return _name; }
    Id vendorId()			const { return _vendor_id; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setVendorId(Id vendor_id)		{ _vendor_id = vendor_id; }

    // Order items
    vector<TemplateItem>& items()		{ return _items; }
    const vector<TemplateItem>& items() const	{ return _items; }

    // Order charges
    vector<TemplateCharge>& charges()			{ return _charges; }
    const vector<TemplateCharge>& charges() const	{ return _charges; }

    // Operations
    bool operator==(const OrderTemplate& rhs) const;
    bool operator!=(const OrderTemplate& rhs) const;

protected:
    QString _name;			// Name of template
    Id _vendor_id;			// Vendor to order from
    vector<TemplateItem> _items;	// List of items to order
    vector<TemplateCharge> _charges;	// List of charges
};

#endif // ORDER_TEMPLATE_H
