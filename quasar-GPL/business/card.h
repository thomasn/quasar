// $Id: card.h,v 1.18 2005/02/03 08:18:03 bpepers Exp $
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

#ifndef CARD_H
#define CARD_H

#include "data_object.h"

class Card: public DataObject {
public:
    // Contructors and Destructor
    Card();
    ~Card();

    // Get methods
    bool company()			const { return _company; }
    const QString& firstName()		const { return _first_name; }
    const QString& lastName()		const { return _last_name; }
    const QString& number()		const { return _number; }
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
    const QString& contact()		const { return _contact; }
    const QString& comments()		const { return _comments; }
    Id patGroupId()			const { return _patgroup_id; }

    // Set methods
    void setCompany(bool flag)			{ _company = flag; }
    void setFirstName(const QString& name)	{ _first_name = name; }
    void setLastName(const QString& name)	{ _last_name = name; }
    void setNumber(const QString& number)	{ _number = number; }
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
    void setContact(const QString& contact)	{ _contact = contact; }
    void setComments(const QString& comments)	{ _comments = comments; }
    void setPatGroupId(Id patgroup_id)		{ _patgroup_id = patgroup_id; }

    // Card groups
    const vector<Id>& groups()		const { return _groups; }
    vector<Id>& groups()		      { return _groups; }

    // Card discounts
    const vector<Id>& discounts()	const { return _discounts; }
    vector<Id>& discounts()		      { return _discounts; }

    // Special methods
    QString name() const;
    QString nameFL() const;
    QString address(bool includeExtra=false) const;

    // Operations
    bool operator==(const Card& rhs) const;
    bool operator!=(const Card& rhs) const;

protected:
    bool _company;
    QString _first_name;
    QString _last_name;
    QString _number;
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
    QString _contact;
    QString _comments;
    Id _patgroup_id;
    vector<Id> _groups;
    vector<Id> _discounts;
};

#endif // CARD_H
