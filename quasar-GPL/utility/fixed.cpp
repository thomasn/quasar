// $Id: fixed.cpp,v 1.18 2005/04/19 07:11:33 bpepers Exp $
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

#include "fixed.h"
#include <assert.h>
#include <math.h>
#include <unicode/numfmt.h>

double
fsign(double value)
{
    if (value < 0.0) return -1.0;
    return 1.0;
}

fixed::fixed()
{
    set(4, 0);
}

fixed::fixed(double val)
{
    set(4, int64_t(val * pow(10.0, 4) + .5 * fsign(val)));
}

fixed::fixed(int32_t val)
{
    set(4, int64_t(val * pow(10.0, 4)));
}

fixed::fixed(int64_t val)
{
    set(4, int64_t(val * pow(10.0, 4)));
}

fixed::fixed(uint32_t val)
{
    set(4, int64_t(val * pow(10.0, 4)));
}

fixed::fixed(uint64_t val)
{
    set(4, int64_t(val * pow(10.0, 4)));
}

fixed::fixed(const fixed& value)
{
    _value = value._value;
}

fixed& fixed::operator=(const fixed& value)
{
    _value = value._value;
    return *this;
}

fixed& fixed::operator=(double value)
{
    set(4, int64_t(value * pow(10.0, 4) + .5 * fsign(value)));
    return *this;
}

fixed& fixed::operator=(int32_t value)
{
    set(4, int64_t(value * pow(10.0, 4)));
    return *this;
}

fixed& fixed::operator=(int64_t value)
{
    set(4, int64_t(value * pow(10.0, 4.0)));
    return *this;
}

int
fixed::sign() const
{
    if (toDouble() < 0.0) return -1;
    return 1;
}

int
fixed::scale() const
{
    return _value & 0xF;
}

void
fixed::setScale(int scale)
{
    assert(scale >= 0 && scale < 16);
#ifdef __GNUC__
    _value = (_value & 0xFFFFFFFFFFFFFFF0LL) | scale;
#else
    _value = (_value & 0xFFFFFFFFFFFFFFF0) | scale;
#endif
}

int64_t
fixed::value() const
{
    return _value >> 4;
}

void
fixed::setValue(int64_t value)
{
    _value = (_value & 0xF) | value << 4; 
}

void
fixed::set(int scale, int64_t value)
{
    assert(scale >= 0 && scale < 16);
#ifdef __GNUC__
    _value = ((value << 4) & 0xFFFFFFFFFFFFFFF0LL) | scale;
#else
    _value = ((value << 4) & 0xFFFFFFFFFFFFFFF0) | scale;
#endif
}

void
fixed::roundTo(int digits)
{
    double factor = pow(10.0, scale() - digits);
    if (value() < 0)
	setValue(int64_t(int64_t(value() / factor - .5) * factor));
    else
	setValue(int64_t(int64_t(value() / factor + .5) * factor));
}

void
fixed::moneyRound()
{
    UErrorCode status = U_ZERO_ERROR;
    NumberFormat* fmt = NumberFormat::createCurrencyInstance(status);
    if (U_FAILURE(status)) {
	roundTo(2);
	return;
    }
    roundTo(fmt->getMaximumFractionDigits());
}

fixed
fixed::neg(void) const
{
    fixed result(*this);
    result.setValue(-result.value());
    return result;
}

double
fixed::toDouble() const
{
    return value() / pow(10.0, scale());
}

int
fixed::toInt() const
{
    if (value() >= 0)
	return int(value() / pow(10.0, scale()) + .000001);
    return int(value() / pow(10.0, scale()) - .000001);
}

unsigned int
fixed::toUInt() const
{
    if (value() >= 0)
	return int(value() / pow(10.0, scale()) + .000001);
    return (unsigned int)(value() / pow(10.0, scale()) - .000001);
}

int64_t
fixed::toInt64() const
{
    if (value() >= 0)
	return int64_t(value() / pow(10.0, scale()) + .000001);
    return int64_t(value() / pow(10.0, scale()) - .000001);
}

fixed
fixed::abs() const
{
    return fabs(toDouble());
}

bool
operator==(fixed lhs, fixed rhs)
{
    int ls = lhs.scale();
    int rs = rhs.scale();

    if (ls == rs)
	return lhs.value() == rhs.value();
    if (ls > rs)
	return lhs.value() == rhs.value() * pow(10.0, ls - rs);
    return lhs.value() * pow(10.0, ls - rs) == rhs.value();
}

bool
operator!=(fixed lhs, fixed rhs)
{
    int ls = lhs.scale();
    int rs = rhs.scale();

    if (ls == rs)
	return lhs.value() != rhs.value();
    if (ls > rs)
	return lhs.value() != rhs.value() * pow(10.0, ls - rs);
    return lhs.value() * pow(10.0, ls - rs) != rhs.value();
}

bool
operator<(fixed lhs, fixed rhs)
{
    int ls = lhs.scale();
    int rs = rhs.scale();

    if (ls == rs)
	return lhs.value() < rhs.value();
    if (ls > rs)
	return lhs.value() < rhs.value() * pow(10.0, ls - rs);
    return lhs.value() * pow(10.0, ls - rs) < rhs.value();
}

bool
operator<=(fixed lhs, fixed rhs)
{
    int ls = lhs.scale();
    int rs = rhs.scale();

    if (ls == rs)
	return lhs.value() <= rhs.value();
    if (ls > rs)
	return lhs.value() <= rhs.value() * pow(10.0, ls - rs);
    return lhs.value() * pow(10.0, ls - rs) <= rhs.value();
}

bool
operator>(fixed lhs, fixed rhs)
{
    int ls = lhs.scale();
    int rs = rhs.scale();

    if (ls == rs)
	return lhs.value() > rhs.value();
    if (ls > rs)
	return lhs.value() > rhs.value() * pow(10.0, ls - rs);
    return lhs.value() * pow(10.0, ls - rs) > rhs.value();
}

bool
operator>=(fixed lhs, fixed rhs)
{
    int ls = lhs.scale();
    int rs = rhs.scale();

    if (ls == rs)
	return lhs.value() >= rhs.value();
    if (ls > rs)
	return lhs.value() >= rhs.value() * pow(10.0, ls - rs);
    return lhs.value() * pow(10.0, ls - rs) >= rhs.value();
}

QString
fixed::toString() const
{
    QString text = QString::number(toDouble(), 'f');
    while (text.right(1) == "0")
	text = text.left(text.length() - 1);
    if (text.right(1) == ".")
	text = text.left(text.length() - 1);
    return text;
}

fixed
operator+(fixed lhs, fixed rhs)
{
    return lhs.toDouble() + rhs.toDouble();
}

fixed
operator-(fixed lhs, fixed rhs)
{
    return lhs.toDouble() - rhs.toDouble();
}

fixed
operator*(fixed lhs, fixed rhs)
{
    return lhs.toDouble() * rhs.toDouble();
}

fixed
operator/(fixed lhs, fixed rhs)
{
    return lhs.toDouble() / rhs.toDouble();
}
