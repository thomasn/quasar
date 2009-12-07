// $Id: slip.h,v 1.16 2005/03/13 22:16:34 bpepers Exp $
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

#ifndef SLIP_H
#define SLIP_H

#include "data_object.h"

struct SlipItem {
    SlipItem();

    Id item_id;			// Item id of item
    QString number;		// Number of the item
    QString size;		// Size of item
    fixed size_qty;		// Quantity of size
    fixed quantity;		// Quantity of item
    fixed ext_cost;		// Extended cost for cost-on slips

    bool operator==(const SlipItem& rhs) const;
    bool operator!=(const SlipItem& rhs) const;
};

class Slip: public DataObject {
public:
    // Constructors and Destructor
    Slip();
    virtual ~Slip();

    // Get methods
    Id vendorId()			const { return _vendor_id; }
    QString number()			const { return _number; }
    QString waybill()			const { return _waybill; }
    QString carrier()			const { return _carrier; }
    QDate shipDate()			const { return _ship_date; }
    Id storeId()			const { return _store_id; }
    QString invoiceNumber()		const { return _inv_num; }
    int numPieces()			const { return _num_pieces; }
    QString status()			const { return _status; }
    QDate postDate()			const { return _post_date; }

    // Set methods
    void setVendorId(Id id)			{ _vendor_id = id; }
    void setNumber(const QString& number)	{ _number = number; }
    void setWaybill(const QString& waybill)	{ _waybill = waybill; }
    void setCarrier(const QString& carrier)	{ _carrier = carrier; }
    void setShipDate(QDate date)		{ _ship_date = date; }
    void setStoreId(Id id)			{ _store_id = id; }
    void setInvoiceNumber(const QString& num)	{ _inv_num = num; }
    void setNumPieces(int count)		{ _num_pieces = count; }
    void setStatus(const QString& status)	{ _status = status; }
    void setPostDate(QDate date)		{ _post_date = date; }

    // Item lines
    vector<SlipItem>& items()			{ return _items; }
    const vector<SlipItem>& items() const	{ return _items; }

    // Purchase orders
    QStringList& orders()			{ return _order_nums; }
    const QStringList& orders() const		{ return _order_nums; }

    // Operations
    bool operator==(const Slip& rhs) const;
    bool operator!=(const Slip& rhs) const;

protected:
    Id _vendor_id;			// Vendor card id
    QString _number;			// Slip number
    QString _waybill;			// Waybill number
    QString _carrier;			// Carrier
    QDate _ship_date;			// Date was shipped
    Id _store_id;			// Store
    QStringList _order_nums;		// Purchase order numbers
    QString _inv_num;			// Vendor invoice number
    int _num_pieces;			// Number of boxes
    QString _status;			// Current status
    QDate _post_date;			// Date slip was posted
    vector<SlipItem> _items;		// Items in slip
};

#endif // SLIP_H
