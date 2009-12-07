// $Id: store.h,v 1.11 2005/03/15 15:33:59 bpepers Exp $
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

#ifndef STORE_H
#define STORE_H

#include "data_object.h"

class Store: public DataObject {
public:
    // Constructors and Destructor
    Store();
    ~Store();

    // Get methods
    const QString& name()		const { return _name; }
    const QString& number()		const { return _number; }
    const QString& contact()		const { return _contact; }
    bool hasAddress()			const { return _has_addr; }
    const QString& street()		const { return _street; }
    const QString& street2()		const { return _street2; }
    const QString& city()		const { return _city; }
    const QString& province()		const { return _province; }
    const QString& country()		const { return _country; }
    const QString& postal()		const { return _postal; }
    const QString& phoneNumber()	const { return _phone_num; }
    const QString& phone2Number()	const { return _phone2_num; }
    const QString& faxNumber()		const { return _fax_num; }
    const QString& email()		const { return _email; }
    const QString& webPage()		const { return _web_page; }
    bool canSell()			const { return _can_sell; }
    Id yearEndTransferId()		const { return _year_end_id; }
    Id companyId()			const { return _company_id; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setNumber(const QString& number)	{ _number = number; }
    void setContact(const QString& contact)	{ _contact = contact; }
    void setHasAddress(bool flag)		{ _has_addr = flag; }
    void setStreet(const QString& street)	{ _street = street; }
    void setStreet2(const QString& street)	{ _street2 = street; }
    void setCity(const QString& city)		{ _city = city; }
    void setProvince(const QString& prov)	{ _province = prov; }
    void setCountry(const QString& country)	{ _country = country; }
    void setPostal(const QString& postal)	{ _postal = postal; }
    void setPhoneNumber(const QString& phone)	{ _phone_num = phone; }
    void setPhone2Number(const QString& phone)	{ _phone2_num = phone; }
    void setFaxNumber(const QString& fax)	{ _fax_num = fax; }
    void setEmail(const QString& email)		{ _email = email; }
    void setWebPage(const QString& webPage)	{ _web_page = webPage; }
    void setCanSell(bool flag)			{ _can_sell = flag; }
    void setYearEndTransferId(Id id)		{ _year_end_id = id; }
    void setCompanyId(Id id)			{ _company_id = id; }

    // Address string
    QString address(bool includeExtra=false) const;

    // Operations
    bool operator==(const Store& rhs) const;
    bool operator!=(const Store& rhs) const;

protected:
    QString _name;		// Name of store
    QString _number;		// Unique store number
    QString _contact;		// Contact person
    bool _has_addr;		// Has its own address?
    QString _street;
    QString _street2;
    QString _city;
    QString _province;
    QString _country;
    QString _postal;
    QString _phone_num;
    QString _phone2_num;
    QString _fax_num;
    QString _email;
    QString _web_page;
    bool _can_sell;		// Can sell from this store?
    Id _year_end_id;		// Year end income->earnings transfer tx id
    Id _company_id;		// Company store is in
};

#endif // STORE_H
