// $Id: security_type.h,v 1.7 2004/12/30 00:07:58 bpepers Exp $
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

#ifndef SECURITY_TYPE_H
#define SECURITY_TYPE_H

#include "data_object.h"

struct SecurityRule {
    SecurityRule();

    QString screen;
    bool allowView;
    bool allowCreate;
    bool allowUpdate;
    bool allowDelete;

    bool operator==(const SecurityRule& rhs) const;
    bool operator!=(const SecurityRule& rhs) const;
};

class SecurityType: public DataObject {
public:
    // Constructors and Destructor
    SecurityType();
    ~SecurityType();

    // Get methods
    const QString& name()		const { return _name; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }

    // Security rules
    vector<SecurityRule>& rules()		{ return _rules; }
    const vector<SecurityRule>& rules() const	{ return _rules; }

    // Special operations
    bool securityCheck(const QString& screen, const QString& op) const;

    // Operations
    bool operator==(const SecurityType& rhs) const;
    bool operator!=(const SecurityType& rhs) const;

protected:
    QString _name;		 // Short name (30 characters)
    vector<SecurityRule> _rules; // List of whats not allowed
};

#endif // SECURITY_TYPE_H
