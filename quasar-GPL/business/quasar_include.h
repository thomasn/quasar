// $Id: quasar_include.h,v 1.10 2004/03/20 01:33:45 bpepers Exp $
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

#ifndef QUASAR_INCLUDE_H
#define QUASAR_INCLUDE_H

// These are common includes that pretty much everything in Quasar uses
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qpair.h>
#include <qvaluevector.h>
#include <qvaluelist.h>

// Work around bogus QValueVector operators
template <class T>
bool operator==(const QValueVector<T>& lhs, const QValueVector<T>& rhs)
{
    if (lhs.size() != rhs.size()) return false;
    return qEqual(lhs.begin(), lhs.end(), rhs.begin());
}
template <class T>
bool operator!=(const QValueVector<T>& lhs, const QValueVector<T>& rhs)
{
    if (lhs.size() != rhs.size()) return true;
    return !qEqual(lhs.begin(), lhs.end(), rhs.begin());
}

// Another workaround since QPair doesn't define !=
template <class T1, class T2>
bool operator!=(const QPair<T1, T2>& lhs, const QPair<T1, T2>& rhs)
{
    return lhs.first != rhs.first || lhs.second != rhs.second;
}

// Fix until all code is changed to use QValueVector directly
//#define vector QValueVector
#include <vector>
using std::vector;

#endif // QUASAR_INCLUDE_H
