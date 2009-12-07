// $Id: variant.h,v 1.17 2005/04/12 07:42:57 bpepers Exp $
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

#ifndef VARIANT_H
#define VARIANT_H

#include <qstring.h>
#include "date_time.h"
#include "fixed.h"
#include "price.h"
#include "id.h"
#include "plu.h"

class Variant
{
public:
    enum Type { T_NULL, STRING, FIXED, BOOL, DATE, TIME, PRICE, ID, PLU };

    Variant();
    ~Variant();

    Variant(const Variant& value);
    Variant(const QString& value);
    Variant(const char* value);
    Variant(int32_t value);
    Variant(int64_t value);
    Variant(uint32_t value);
    Variant(uint64_t value);
    Variant(bool value);
    Variant(double value);
    Variant(fixed value);
    Variant(QDate value);
    Variant(QTime value);
    Variant(const Price& price);
    Variant(const Id& value);
    Variant(const Plu& plu);

    Variant& operator=(const Variant& rhs);

    int compare(const Variant& rhs) const;
    bool operator==(const Variant& rhs) const;
    bool operator!=(const Variant& rhs) const;
    bool operator>(const Variant& rhs) const;
    bool operator>=(const Variant& rhs) const;
    bool operator<(const Variant& rhs) const;
    bool operator<=(const Variant& rhs) const;

    Type type() const;
    const char* typeName() const;

    bool isNull() const;
    void clear();
    void cast(Type);

    QString toString() const;
    fixed toFixed() const;
    bool toBool() const;
    QDate toDate() const;
    QTime toTime() const;
    int toInt() const { return toFixed().toInt(); }
    int toUInt() const { return toFixed().toUInt(); }
    double toDouble() const { return toFixed().toDouble(); }
    Price toPrice() const;
    Id toId() const;
    Plu toPlu() const;

    QString& asString();
    bool& asBool();
    QDate& asDate();
    QTime& asTime();
    Price& asPrice();
    Id& asId();
    Plu& asPlu();

    static const char* typeToName(Type type);
    static Type nameToType(const char* name);
    static bool isNumeric(Type type);
    static Variant null;

private:
    Type _type;
    union {
	QString* stringV;
	int64_t fixedV;
	bool boolV;
	QDate* dateV;
	QTime* timeV;
	Price* priceV;
	Id* idV;
	Plu* pluV;
    } _value;
};

inline Variant::Type Variant::type() const
{
    return _type;
}

inline bool Variant::isNull() const
{
    return _type == T_NULL;
}

#endif
