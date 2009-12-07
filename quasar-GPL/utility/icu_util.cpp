// $Id: icu_util.cpp,v 1.2 2005/03/05 11:10:35 bpepers Exp $
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

#include "icu_util.h"
#include <stdlib.h>

UnicodeString
convertToICU(const QString& string)
{
    return UnicodeString(string.utf8(), "UTF8");
}

QString
convertToQt(const UnicodeString& string)
{
    int size = string.length() * 4 + 1;
    if (size < 8192) {
	char data[8192];
	int32_t used = string.extract(0, string.length(), data, size, "UTF8");
	data[used] = 0;
	return QString::fromUtf8(data);
    }

    char* data = (char*)malloc(size);
    int32_t used = string.extract(0, string.length(), data, size, "UTF8");
    data[used] = 0;
    QString text = QString::fromUtf8(data);

    free(data);
    return text;
}
