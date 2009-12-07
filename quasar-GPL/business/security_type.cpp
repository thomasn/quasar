// $Id: security_type.cpp,v 1.9 2004/12/30 00:07:58 bpepers Exp $
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

#include "security_type.h"

SecurityType::SecurityType()
    : _name("")
{
    _data_type = SECURITY_TYPE;
}

SecurityType::~SecurityType()
{
}

bool
SecurityType::operator==(const SecurityType& rhs) const
{
    if ((const DataObject&)rhs != *this) return false;
    if (rhs._name != _name) return false;
    if (rhs._rules != _rules) return false;
    return true;
}

bool
SecurityType::operator!=(const SecurityType& rhs) const
{
    return !(*this == rhs);
}

bool
SecurityType::securityCheck(const QString& screen, const QString& op) const
{
    bool allow = true;
    for (unsigned int i = 0; i < _rules.size(); ++i) {
	const SecurityRule& rule = _rules[i];
	if (rule.screen != screen && rule.screen != "All") continue;
	if (op == "View") allow = rule.allowView;
	if (op == "Create") allow = rule.allowCreate;
	if (op == "Update") allow = rule.allowUpdate;
	if (op == "Delete") allow = rule.allowDelete;
    }
    return allow;
}

SecurityRule::SecurityRule()
    : screen(""), allowView(true), allowCreate(true), allowUpdate(true),
      allowDelete(true)
{
}

bool
SecurityRule::operator==(const SecurityRule& rhs) const
{
    if (rhs.screen != screen) return false;
    if (rhs.allowView != allowView) return false;
    if (rhs.allowCreate != allowCreate) return false;
    if (rhs.allowUpdate != allowUpdate) return false;
    if (rhs.allowDelete != allowDelete) return false;
    return true;
}

bool
SecurityRule::operator!=(const SecurityRule& rhs) const
{
    return !(*this == rhs);
}
