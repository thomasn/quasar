// $Id: base64.cpp,v 1.4 2004/07/01 19:26:00 bpepers Exp $
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

#include "base64.h"

static char
base64Encode(unsigned char ch)
{
    if (ch < 26) return 'A' + ch;
    if (ch < 52) return 'a' + (ch - 26);
    if (ch < 62) return '0' + (ch - 52);
    if (ch == 62) return '+';
    return '/';
}

static unsigned char
base64Decode(char ch)
{
    if (ch >= 'A' && ch <= 'Z') return ch - 'A';
    if (ch >= 'a' && ch <= 'z') return ch - 'a' + 26;
    if (ch >= '0' && ch <= '9') return ch - '0' + 52;
    if (ch == '+') return 62;
    return 63;
}

static bool
isBase64(char ch)
{
    if (ch >= 'A' && ch <= 'Z') return true;
    if (ch >= 'a' && ch <= 'z') return true;
    if (ch >= '0' && ch <= '9') return true;
    if (ch == '+' || ch == '/') return true;
    if (ch == '=') return true;
    return false;
}

QString
base64Encode(QByteArray& data)
{
    if (data.size() == 0) return "";

    QString result;
    for (unsigned int i = 0; i < data.size(); i += 3) {
	unsigned char b1 = 0, b2 = 0, b3 = 0;
	b1 = data.at(i);
	if (i + 1 < data.size()) b2 = data.at(i + 1);
	if (i + 2 < data.size()) b3 = data.at(i + 2);

	unsigned char v1 = b1 >> 2;
	unsigned char v2 = ((b1 & 0x3) << 4) | (b2 >> 4);
	unsigned char v3 = ((b2 & 0xf) << 2) | (b3 >> 6);
	unsigned char v4 = b3 & 0x3f;

	if ((i % 54) == 0 && i != 0) result += "\r\n";

	result += base64Encode(v1);
	result += base64Encode(v2);
	if (i + 1 < data.size())
	    result += base64Encode(v3);
	else
	    result += '=';
	if (i + 2 < data.size())
	    result += base64Encode(v4);
	else
	    result += '=';
    }

    return result;
}

bool
base64Decode(const QString& text, QByteArray& data)
{
    QString valid;
    for (unsigned int i = 0; i < text.length(); ++i)
	if (isBase64(text.at(i))) valid += text.at(i);

    data.resize(valid.length() * 3 / 4);
    if (valid.isEmpty()) return true;

    unsigned int pos = 0;
    for (unsigned int i = 0; i < valid.length(); i += 4) {
	char c1 = valid.at(i).latin1();
	char c2 = (i + 1 < valid.length()) ? valid.at(i + 1).latin1() : 'A';
	char c3 = (i + 2 < valid.length()) ? valid.at(i + 2).latin1() : 'A';
	char c4 = (i + 3 < valid.length()) ? valid.at(i + 3).latin1() : 'A';

	unsigned char b1 = base64Decode(c1);
	unsigned char b2 = base64Decode(c2);
	unsigned char b3 = base64Decode(c3);
	unsigned char b4 = base64Decode(c4);

	data.at(pos++) = (b1 << 2) | (b2 >> 4);
	if (c3 != '=')
	    data.at(pos++) = ((b2 & 0xf) << 4) | (b3 >> 2);
	if (c4 != '=')
	    data.at(pos++) = ((b3 & 0x3) << 6) | b4;
    }

    if (pos != data.size())
	data.resize(pos);

    return true;
}
