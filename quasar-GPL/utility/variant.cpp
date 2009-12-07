// $Id: variant.cpp,v 1.27 2005/04/12 07:42:57 bpepers Exp $
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

#include "variant.h"
#include "date_valcon.h"
#include "time_valcon.h"
#include "price_valcon.h"

Variant Variant::null;

Variant::Variant()
    : _type(T_NULL)
{
}

Variant::~Variant()
{
    clear();
}

Variant::Variant(const Variant& value)
    : _type(value._type), _value(value._value)
{
    switch (_type) {
    case STRING:
	_value.stringV = new QString(*value._value.stringV);
	break;
    case DATE:
	_value.dateV = new QDate(*value._value.dateV);
	break;
    case TIME:
	_value.timeV = new QTime(*value._value.timeV);
	break;
    case PRICE:
	_value.priceV = new Price(*value._value.priceV);
	break;
    case ID:
	_value.idV = new Id(*value._value.idV);
	break;
    case PLU:
	_value.pluV = new Plu(*value._value.pluV);
	break;
    default:
	break;
    }
}

Variant::Variant(const QString& value)
{
    if (value.isNull()) {
	_type = T_NULL;
    } else {
	_type = STRING;
	_value.stringV = new QString(value);
    }
}

Variant::Variant(const char* value)
{
    if (value == NULL) {
	_type = T_NULL;
    } else {
	_type = STRING;
	_value.stringV = new QString(value);
    }
}

Variant::Variant(int32_t value)
{
    _type = FIXED;
    _value.fixedV = fixed(value).raw();
}

Variant::Variant(int64_t value)
{
    _type = FIXED;
    _value.fixedV = fixed(value).raw();
}

Variant::Variant(uint32_t value)
{
    _type = FIXED;
    _value.fixedV = fixed(value).raw();
}

Variant::Variant(uint64_t value)
{
    _type = FIXED;
    _value.fixedV = fixed(value).raw();
}

Variant::Variant(bool value)
{
    _type = BOOL;
    _value.boolV = value;
}

Variant::Variant(double value)
{
    _type = FIXED;
    _value.fixedV = fixed(value).raw();
}

Variant::Variant(fixed value)
{
    _type = FIXED;
    _value.fixedV = value.raw();
}

Variant::Variant(QDate value)
{
    if (value.isNull()) {
	_type = T_NULL;
    } else {
	_type = DATE;
	_value.dateV = new QDate(value);
    }
}

Variant::Variant(QTime value)
{
    _type = TIME;
    _value.timeV = new QTime(value);
}

Variant::Variant(const Price& value)
{
    _type = PRICE;
    _value.priceV = new Price(value);
}

Variant::Variant(const Id& value)
{
    if (value.isNull()) {
	_type = T_NULL;
    } else {
	_type = ID;
	_value.idV = new Id(value);
    }
}

Variant::Variant(const Plu& value)
{
    _type = PLU;
    _value.pluV = new Plu(value);
}

Variant&
Variant::operator=(const Variant& rhs)
{
    clear();
    _type = rhs._type;
    switch (_type) {
    case T_NULL:
	break;
    case STRING:
	_value.stringV = new QString(*rhs._value.stringV);
	break;
    case FIXED:
	_value.fixedV = rhs._value.fixedV;
	break;
    case BOOL:
	_value.boolV = rhs._value.boolV;
	break;
    case DATE:
	_value.dateV = new QDate(*rhs._value.dateV);
	break;
    case TIME:
	_value.timeV = new QTime(*rhs._value.timeV);
	break;
    case PRICE:
	_value.priceV = new Price(*rhs._value.priceV);
	break;
    case ID:
	_value.idV = new Id(*rhs._value.idV);
	break;
    case PLU:
	_value.pluV = new Plu(*rhs._value.pluV);
	break;
    }
    return *this;
}

int
Variant::compare(const Variant& rhs) const
{
    if (*this == rhs) return 0;
    if (*this < rhs) return -1;
    return 1;
}

bool
Variant::operator==(const Variant& rhs) const
{
    // Null isn't equal to anything but itself
    if (_type == T_NULL || rhs._type == T_NULL)
	return _type == rhs._type;

    // Handle the valid cases
    switch (_type) {
    case STRING:
	if (rhs._type == STRING) return (toString() == rhs.toString());
	if (rhs._type == FIXED) return (toFixed() == rhs.toFixed());
	if (rhs._type == BOOL) return (toBool() == rhs.toBool());
	if (rhs._type == DATE) return (toDate() == rhs.toDate());
	if (rhs._type == TIME) return (toTime() == rhs.toTime());
	if (rhs._type == PRICE) return (toPrice() == rhs.toPrice());
	if (rhs._type == ID) return (toId() == rhs.toId());
	if (rhs._type == PLU) return (toPlu() == rhs.toPlu());
	return false;
    case FIXED:
	if (rhs._type == STRING) return (toFixed() == rhs.toFixed());
	if (rhs._type == FIXED) return (toFixed() == rhs.toFixed());
	if (rhs._type == BOOL) return (toFixed() == rhs.toFixed());
	return false;
    case BOOL:
	if (rhs._type == STRING) return (toBool() == rhs.toBool());
	if (rhs._type == FIXED) return (toBool() == rhs.toBool());
	if (rhs._type == BOOL) return (toBool() == rhs.toBool());
	return false;
    case DATE:
	if (rhs._type == STRING) return (toDate() == rhs.toDate());
	if (rhs._type == DATE) return (toDate() == rhs.toDate());
	return false;
    case TIME:
	if (rhs._type == STRING) return (toTime() == rhs.toTime());
	if (rhs._type == TIME) return (toTime() == rhs.toTime());
	return false;
    case PRICE:
	if (rhs._type == STRING) return (toPrice() == rhs.toPrice());
	if (rhs._type == PRICE) return (toPrice() == rhs.toPrice());
	return false;
    case ID:
	if (rhs._type == STRING) return (toId() == rhs.toId());
	if (rhs._type == ID) return (toId() == rhs.toId());
	return false;
    case PLU:
	if (rhs._type == STRING) return (toPlu() == rhs.toPlu());
	if (rhs._type == PLU) return (toPlu() == rhs.toPlu());
	return false;
    default:
	break;
    }
    return false;
}

bool
Variant::operator!=(const Variant& rhs) const
{
    return !(*this == rhs);
}

bool
Variant::operator>(const Variant& rhs) const
{
    // Null isn't greater than to anything
    if (_type == T_NULL)
	return false;
    if (rhs._type == T_NULL)
	return true;

    // Handle the valid cases
    switch (_type) {
    case STRING:
	if (rhs._type == STRING) return (toString() > rhs.toString());
	if (rhs._type == FIXED) return (toFixed() > rhs.toFixed());
	if (rhs._type == BOOL) return (toBool() > rhs.toBool());
	if (rhs._type == DATE) return (toDate() > rhs.toDate());
	if (rhs._type == TIME) return (toTime() > rhs.toTime());
	if (rhs._type == PRICE) return (toPrice() > rhs.toPrice());
	return false;
    case FIXED:
	if (rhs._type == STRING) return (toFixed() > rhs.toFixed());
	if (rhs._type == FIXED) return (toFixed() > rhs.toFixed());
	if (rhs._type == BOOL) return (toFixed() > rhs.toFixed());
	return false;
    case BOOL:
	if (rhs._type == STRING) return (toBool() > rhs.toBool());
	if (rhs._type == FIXED) return (toBool() > rhs.toBool());
	if (rhs._type == BOOL) return (toBool() > rhs.toBool());
	return false;
    case DATE:
	if (rhs._type == STRING) return (toDate() > rhs.toDate());
	if (rhs._type == DATE) return (toDate() > rhs.toDate());
	return false;
    case TIME:
	if (rhs._type == STRING) return (toTime() > rhs.toTime());
	if (rhs._type == TIME) return (toTime() > rhs.toTime());
	return false;
    case PRICE:
	if (rhs._type == STRING) return (toPrice() > rhs.toPrice());
	if (rhs._type == PRICE) return (toPrice() > rhs.toPrice());
	return false;
    default:
	break;
    }
    return false;
}

bool
Variant::operator>=(const Variant& rhs) const
{
    // Null isn't greater than to anything
    if (_type == T_NULL)
	return false;
    if (rhs._type == T_NULL)
	return true;

    // Handle the valid cases
    switch (_type) {
    case STRING:
	if (rhs._type == STRING) return (toString() >= rhs.toString());
	if (rhs._type == FIXED) return (toFixed() >= rhs.toFixed());
	if (rhs._type == BOOL) return (toBool() >= rhs.toBool());
	return false;
    case FIXED:
	if (rhs._type == STRING) return (toFixed() >= rhs.toFixed());
	if (rhs._type == FIXED) return (toFixed() >= rhs.toFixed());
	if (rhs._type == BOOL) return (toFixed() >= rhs.toFixed());
	return false;
    case BOOL:
	if (rhs._type == STRING) return (toBool() >= rhs.toBool());
	if (rhs._type == FIXED) return (toBool() >= rhs.toBool());
	if (rhs._type == BOOL) return (toBool() >= rhs.toBool());
	return false;
    case DATE:
	if (rhs._type == STRING) return (toDate() >= rhs.toDate());
	if (rhs._type == DATE) return (toDate() >= rhs.toDate());
	return false;
    case TIME:
	if (rhs._type == STRING) return (toTime() >= rhs.toTime());
	if (rhs._type == TIME) return (toTime() >= rhs.toTime());
	return false;
    case PRICE:
	if (rhs._type == STRING) return (toPrice() >= rhs.toPrice());
	if (rhs._type == PRICE) return (toPrice() >= rhs.toPrice());
	return false;
    default:
	break;
    }
    return false;
}

bool
Variant::operator<(const Variant& rhs) const
{
    // Null isn't greater than to anything
    if (_type == T_NULL)
	return true;
    if (rhs._type == T_NULL)
	return false;

    // Handle the valid cases
    switch (_type) {
    case STRING:
	if (rhs._type == STRING) return (toString() < rhs.toString());
	if (rhs._type == FIXED) return (toFixed() < rhs.toFixed());
	if (rhs._type == BOOL) return (toBool() < rhs.toBool());
	return false;
    case FIXED:
	if (rhs._type == STRING) return (toFixed() < rhs.toFixed());
	if (rhs._type == FIXED) return (toFixed() < rhs.toFixed());
	if (rhs._type == BOOL) return (toFixed() < rhs.toFixed());
	return false;
    case BOOL:
	if (rhs._type == STRING) return (toBool() < rhs.toBool());
	if (rhs._type == FIXED) return (toBool() < rhs.toBool());
	if (rhs._type == BOOL) return (toBool() < rhs.toBool());
	return false;
    case DATE:
	if (rhs._type == STRING) return (toDate() < rhs.toDate());
	if (rhs._type == DATE) return (toDate() < rhs.toDate());
	return false;
    case TIME:
	if (rhs._type == STRING) return (toTime() < rhs.toTime());
	if (rhs._type == TIME) return (toTime() < rhs.toTime());
	return false;
    case PRICE:
	if (rhs._type == STRING) return (toPrice() < rhs.toPrice());
	if (rhs._type == PRICE) return (toPrice() < rhs.toPrice());
	return false;
    default:
	break;
    }
    return false;
}

bool
Variant::operator<=(const Variant& rhs) const
{
    // Null isn't greater than to anything
    if (_type == T_NULL)
	return true;
    if (rhs._type == T_NULL)
	return false;

    // Handle the valid cases
    switch (_type) {
    case STRING:
	if (rhs._type == STRING) return (toString() <= rhs.toString());
	if (rhs._type == FIXED) return (toFixed() <= rhs.toFixed());
	if (rhs._type == BOOL) return (toBool() <= rhs.toBool());
	return false;
    case FIXED:
	if (rhs._type == STRING) return (toFixed() <= rhs.toFixed());
	if (rhs._type == FIXED) return (toFixed() <= rhs.toFixed());
	if (rhs._type == BOOL) return (toFixed() <= rhs.toFixed());
	return false;
    case BOOL:
	if (rhs._type == STRING) return (toBool() <= rhs.toBool());
	if (rhs._type == FIXED) return (toBool() <= rhs.toBool());
	if (rhs._type == BOOL) return (toBool() <= rhs.toBool());
	return false;
    case DATE:
	if (rhs._type == STRING) return (toDate() <= rhs.toDate());
	if (rhs._type == DATE) return (toDate() <= rhs.toDate());
	return false;
    case TIME:
	if (rhs._type == STRING) return (toTime() <= rhs.toTime());
	if (rhs._type == TIME) return (toTime() <= rhs.toTime());
	return false;
    case PRICE:
	if (rhs._type == STRING) return (toPrice() <= rhs.toPrice());
	if (rhs._type == PRICE) return (toPrice() <= rhs.toPrice());
	return false;
    default:
	break;
    }
    return false;
}

const char*
Variant::typeName() const
{
    return typeToName(_type);
}

void
Variant::clear()
{
    cast(T_NULL);
}

void
Variant::cast(Type type)
{
    if (_type == type) return;

    // Save pointers so can free at end
    QString* stringPtr = (_type == STRING) ? _value.stringV : NULL;
    QDate* datePtr = (_type == DATE) ? _value.dateV : NULL;
    QTime* timePtr = (_type == TIME) ? _value.timeV : NULL;
    Price* pricePtr = (_type == PRICE) ? _value.priceV : NULL;
    Id* idPtr = (_type == ID) ? _value.idV : NULL;
    Plu* pluPtr = (_type == PLU) ? _value.pluV : NULL;

    switch (type) {
    case T_NULL:	break;
    case STRING:	_value.stringV = new QString(toString()); break;
    case FIXED:		_value.fixedV = toFixed().raw(); break;
    case BOOL:		_value.boolV = toBool(); break;
    case DATE:		_value.dateV = new QDate(toDate()); break;
    case TIME:		_value.timeV = new QTime(toTime()); break;
    case PRICE:		_value.priceV = new Price(toPrice()); break;
    case ID:		_value.idV = new Id(toId()); break;
    case PLU:		_value.pluV = new Plu(toPlu()); break;
    }

    // Free old pointers
    delete stringPtr;
    delete datePtr;
    delete timePtr;
    delete pricePtr;
    delete idPtr;
    delete pluPtr;

    _type = type;
}

QString
Variant::toString() const
{
    fixed value;
    switch (_type) {
    case T_NULL:	return QString::null;
    case STRING:	return *_value.stringV;
    case FIXED:		value.setRaw(_value.fixedV); return value.toString();
    case BOOL:		return QString::number(int(_value.boolV));
    case DATE:		return _value.dateV->toString(Qt::ISODate);
    case TIME:		return _value.timeV->toString();
    case PRICE:		return PriceValcon().format(*_value.priceV);
    case ID:		return _value.idV->toString();
    case PLU:		return _value.pluV->toString();
    }
    return QString::null;
}

fixed
Variant::toFixed() const
{
    fixed value;
    switch (_type) {
    case T_NULL:	value = 0.0; break;
    case STRING:	value = _value.stringV->toDouble(); break;
    case FIXED:		value.setRaw(_value.fixedV); break;
    case BOOL:		value = int(_value.boolV); break;
    case DATE:		value = int(makeTimeT(*_value.dateV)); break;
    case TIME:		value = int(makeTimeT(*_value.timeV)); break;
    default:		return value;
    }
    return value;
}

bool
Variant::toBool() const
{
    fixed value;

    switch (_type) {
    case T_NULL:	return false;
    case STRING:	return bool(_value.stringV->toInt());
    case FIXED:		value.setRaw(_value.fixedV); return value.toInt();
    case BOOL:		return _value.boolV;
    default:		return false;
    }

    return false;
}

QDate
Variant::toDate() const
{
    if (_type == STRING) {
	DateValcon valcon;
	if (valcon.parse(*_value.stringV))
	    return valcon.getDate();
    }

    if (_type == DATE)
	return *_value.dateV;

    return QDate();
}

QTime
Variant::toTime() const
{
    if (_type == STRING) {
	TimeValcon valcon;
	if (valcon.parse(*_value.stringV))
	    return valcon.getTime();
    }

    if (_type == TIME)
	return *_value.timeV;

    return QTime();
}

Price
Variant::toPrice() const
{
    Price price;
    PriceValcon valcon;

    switch (_type) {
    case STRING:
	if (valcon.parse(*_value.stringV))
	    return valcon.getPrice();
	break;
    case PRICE:
	return *_value.priceV;
    default:
	return price;
    }

    return price;
}

Id
Variant::toId() const
{
    Id id;

    switch (_type) {
    case STRING:	id.fromString(*_value.stringV); return id;
    case ID:		return *_value.idV;
    case PLU:		return _value.pluV->itemId();
    default:		return id;
    }

    return id;
}

Plu
Variant::toPlu() const
{
    Plu plu;

    switch (_type) {
    case STRING:	plu.fromString(*_value.stringV); return plu;
    case PLU:		return *_value.pluV;
    default:		return plu;
    }

    return plu;
}

QString&
Variant::asString()
{
    cast(STRING);
    return *_value.stringV;
}

bool&
Variant::asBool()
{
    cast(BOOL);
    return _value.boolV;
}

QDate&
Variant::asDate()
{
    cast(DATE);
    return *_value.dateV;
}

QTime&
Variant::asTime()
{
    cast(TIME);
    return *_value.timeV;
}

Price&
Variant::asPrice()
{
    cast(PRICE);
    return *_value.priceV;
}

Id&
Variant::asId()
{
    cast(ID);
    return *_value.idV;
}

Plu&
Variant::asPlu()
{
    cast(PLU);
    return *_value.pluV;
}

const char*
Variant::typeToName(Type type)
{
    switch (type) {
    case T_NULL:	return "Null";
    case STRING:	return "String";
    case FIXED:		return "Fixed";
    case BOOL:		return "Bool";
    case DATE:		return "Date";
    case TIME:		return "Time";
    case PRICE:		return "Price";
    case ID:		return "Id";
    case PLU:		return "Plu";
    }
    return "";
}

Variant::Type
Variant::nameToType(const char* name)
{
    if (strcmp(name, "Null") == 0) return T_NULL;
    if (strcmp(name, "String") == 0) return STRING;
    if (strcmp(name, "Fixed") == 0) return FIXED;
    if (strcmp(name, "Bool") == 0) return BOOL;
    if (strcmp(name, "Date") == 0) return DATE;
    if (strcmp(name, "Time") == 0) return TIME;
    if (strcmp(name, "Price") == 0) return PRICE;
    if (strcmp(name, "Id") == 0) return ID;
    if (strcmp(name, "Plu") == 0) return PLU;
    return T_NULL;
}

bool
Variant::isNumeric(Type type)
{
    switch (type) {
    case FIXED:		return true;
    case BOOL:		return true;
    default:		return false;
    }
    return false;
}
