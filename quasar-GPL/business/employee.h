// $Id: employee.h,v 1.15 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef EMPLOYEE_H
#define EMPLOYEE_H

#include "card.h"

class Employee: public Card {
public:
    // Constructors and Destructor
    Employee();
    ~Employee();

    // Get methods
    QString loginName() const			{ return _login_name; }
    QString password() const			{ return _password; }
    Id defaultStore() const			{ return _store_id; }
    int posLevel() const			{ return _pos_level; }

    // Set methods
    void setLoginName(const QString& text)	{ _login_name = text; }
    void setPassword(const QString& text)	{ _password = text; }
    void setDefaultStore(Id store_id)		{ _store_id = store_id; }
    void setPosLevel(int level)			{ _pos_level = level; }

    // Operations
    bool operator==(const Employee& rhs) const;
    bool operator!=(const Employee& rhs) const;

protected:
    QString _login_name;	// Name for Quasar login
    QString _password;		// Password for login (Quasar or POS)
    Id _store_id;		// Default store user works in
    int _pos_level;		// Security level in POS
};

#endif // EMPLOYEE_H
