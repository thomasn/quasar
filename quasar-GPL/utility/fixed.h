// $Id: fixed.h,v 1.14 2005/04/19 07:11:33 bpepers Exp $
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

#ifndef FIXED_H
#define FIXED_H

#include <qstring.h>
#include "quasar_int.h"

class fixed
{
public:
    // Constructors
    fixed();
    fixed(double value);
    fixed(int32_t value);
    fixed(int64_t value);
    fixed(uint32_t value);
    fixed(uint64_t value);
    fixed(const fixed& value);

    // Assignment
    fixed& operator=(const fixed& value);
    fixed& operator=(double value);
    fixed& operator=(int32_t value);
    fixed& operator=(int64_t value);
    fixed& operator=(const QString& value);

    // Assignment with arithmetic
    fixed& operator+=(fixed value) { *this = *this + value; return *this; }
    fixed& operator-=(fixed value) { *this = *this - value; return *this; }
    fixed& operator*=(fixed value) { *this = *this * value; return *this; }
    fixed& operator/=(fixed value) { *this = *this / value; return *this; }

    // Basic arithmetic
    friend fixed operator+(fixed lhs, fixed rhs);
    friend fixed operator-(fixed lhs, fixed rhs);
    friend fixed operator*(fixed lhs, fixed rhs);
    friend fixed operator/(fixed lhs, fixed rhs);

    // Prefix and postfix increment/decrement
    fixed& operator++();
    fixed& operator--();

    // Simple information
    int sign() const;
    int scale() const;
    bool isInteger() const;

    // Conversions
    QString toString() const;
    double toDouble() const;
    int toInt() const;
    unsigned int toUInt() const;
    int64_t toInt64() const;

    // Functions
    fixed abs() const;
    fixed neg() const;
    fixed round(int digits) const;
    fixed operator-(void) const { return neg(); }

    // Comparisons
    friend bool operator==(fixed lhs, fixed rhs);
    friend bool operator!=(fixed lhs, fixed rhs);
    friend bool operator< (fixed lhs, fixed rhs);
    friend bool operator<=(fixed lhs, fixed rhs);
    friend bool operator> (fixed lhs, fixed rhs);
    friend bool operator>=(fixed lhs, fixed rhs);

    void setScale(int scale);
    void roundTo(int digits);

    // Round to proper number of decimals for locale's currency
    void moneyRound();

    int64_t raw() { return _value; }
    void setRaw(int64_t rawValue) { _value = rawValue; }

protected:
    int64_t _value;

    int64_t value() const;
    void setValue(int64_t value);
    void set(int scale, int64_t value);
};

inline fixed fabs(fixed value) { return value.abs(); }

#if 0
// Disambiguate double
bool operator==(fixed lhs, double rhs) { return lhs == fixed(rhs); }
bool operator!=(fixed lhs, double rhs) { return lhs != fixed(rhs); }
bool operator< (fixed lhs, double rhs) { return lhs <  fixed(rhs); }
bool operator<=(fixed lhs, double rhs) { return lhs <= fixed(rhs); }
bool operator> (fixed lhs, double rhs) { return lhs >  fixed(rhs); }
bool operator>=(fixed lhs, double rhs) { return lhs >= fixed(rhs); }
fixed operator+(fixed lhs, double rhs) { return lhs + fixed(rhs); }
fixed operator-(fixed lhs, double rhs) { return lhs - fixed(rhs); }
fixed operator*(fixed lhs, double rhs) { return lhs * fixed(rhs); }
fixed operator/(fixed lhs, double rhs) { return lhs / fixed(rhs); }
#endif

#endif // FIXED_H
