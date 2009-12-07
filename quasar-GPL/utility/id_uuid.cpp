// $Id: id_uuid.cpp,v 1.1 2004/05/06 04:21:44 bpepers Exp $
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

#include "id.h"
#include <ctype.h>
#include <qdatetime.h>

Id::Id()
{
    setNull();
}

Id::~Id()
{
}

bool
Id::isNull() const
{
    for (int i = 0; i < 16; ++i)
	if (_id[i] != 0) return false;
    return true;
}

void
Id::setNull()
{
    memset(_id, 0, 16);
}

QString
Id::toString() const
{
    char buffer[38];
    sprintf(buffer, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-"
	    "%02x%02x%02x%02x%02x%02x", _id[0], _id[1], _id[2], _id[3],
	    _id[4], _id[5], _id[6], _id[7], _id[8], _id[9], _id[10],
	    _id[11], _id[12], _id[13], _id[14], _id[15]);
    return QString(buffer);
}

bool
Id::fromString(const QString& text)
{
    if (text.length() != 36) return false;

    for (int i = 0; i < 36; ++i) {
	char c = text[i].latin1();
	if (c == '-' && (i == 8 || i == 13 || i == 18 || i == 23))
	    continue;
	if (!isxdigit(c)) return false;
    }

    _id[0] = text.mid(0, 2).toUShort(NULL, 16);
    _id[1] = text.mid(2, 2).toUShort(NULL, 16);
    _id[2] = text.mid(4, 2).toUShort(NULL, 16);
    _id[3] = text.mid(6, 2).toUShort(NULL, 16);

    _id[4] = text.mid(9, 2).toUShort(NULL, 16);
    _id[5] = text.mid(11, 2).toUShort(NULL, 16);

    _id[6] = text.mid(14, 2).toUShort(NULL, 16);
    _id[7] = text.mid(16, 2).toUShort(NULL, 16);

    _id[8] = text.mid(19, 2).toUShort(NULL, 16);
    _id[9] = text.mid(21, 2).toUShort(NULL, 16);

    for (int i = 6; i--;)
	_id[10 + i] = text.mid(i*2+24, 2).toUShort(NULL, 16);

    return true;
}

Id
Id::fromStringStatic(const QString& text)
{
    Id id;
    if (!id.fromString(text))
	qWarning("Invalid id: " + text);
    return id;
}

bool
Id::operator==(const Id& rhs) const
{
    return memcmp(_id, rhs._id, 16) == 0;
}

bool
Id::operator!=(const Id& rhs) const
{
    return memcmp(_id, rhs._id, 16) != 0;
}

bool
Id::operator<(const Id& rhs) const
{
    return memcmp(_id, rhs._id, 16) < 0;
}

bool
Id::operator<=(const Id& rhs) const
{
    return memcmp(_id, rhs._id, 16) <= 0;
}

bool
Id::operator>(const Id& rhs) const
{
    return memcmp(_id, rhs._id, 16) > 0;
}

bool
Id::operator>=(const Id& rhs) const
{
    return memcmp(_id, rhs._id, 16) >= 0;
}

void
Id::newValue()
{
    static const int intbits = sizeof(int)*8;
    static int randbits = 0;
    if (!randbits) {
	int max = RAND_MAX;
	do { ++randbits; } while ((max = max >> 1));
	srand((uint)QDateTime::currentDateTime().toTime_t());
	rand();
    }

    uint* data = (uint*)&_id;
    int chunks = 16 / sizeof(uint);
    while (chunks--) {
	uint randNumber = 0;
	for (int filled = 0; filled < intbits; filled += randbits)
	    randNumber |= rand() << filled;
	data[chunks] = randNumber;
    }

    _id[8] = (_id[8] & 0x3F) | 0x80; // DCE
    _id[6] = (_id[6] & 0x0F) | 0x40; // Random
}

QString
Id::sqlDataType()
{
    return "char(36)";
}
