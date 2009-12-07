// $Id: user.h,v 1.23 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef USER_H
#define USER_H

#include "data_object.h"

class User: public DataObject {
public:
    // Constructors and Destructor
    User();
    ~User();

    // Get methods
    QString name() const			{ return _name; }
    QString password() const			{ return _password; }
    Id defaultStore() const			{ return _store_id; }
    Id defaultEmployee() const			{ return _employee_id; }
    Id securityType() const			{ return _security_id; }
    QString screen() const			{ return _screen; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setPassword(const QString& password)	{ _password = password; }
    void setDefaultStore(Id id)			{ _store_id = id; }
    void setDefaultEmployee(Id id)		{ _employee_id = id; }
    void setSecurityType(Id id)			{ _security_id = id; }
    void setScreen(const QString& screen)	{ _screen = screen; }

    // Operations
    bool operator==(const User& rhs) const;
    bool operator!=(const User& rhs) const;

protected:
    QString _name;		// Login name
    QString _password;		// Password
    Id _store_id;		// Default store
    Id _employee_id;		// Default employee
    Id _security_id;		// Security role
    QString _screen;		// Starting screen
};

#endif // USER_H
