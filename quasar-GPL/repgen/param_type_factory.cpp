// $Id: param_type_factory.cpp,v 1.6 2005/06/22 23:17:51 bpepers Exp $
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

#include "param_type_factory.h"
#include "string_param_type.h"
#include "integer_param_type.h"
#include "double_param_type.h"
#include "date_param_type.h"
#include "time_param_type.h"
#include "bool_param_type.h"
#include "choice_param_type.h"

template
QValueVector<ParamTypeFactory::TypePair> ParamTypeFactory::_types;

static ReportParamType*
createString() { return new StringParamType(); }

static ReportParamType*
createInteger() { return new IntegerParamType(); }

static ReportParamType*
createDouble() { return new DoubleParamType(); }

static ReportParamType*
createDate() { return new DateParamType(); }

static ReportParamType*
createTime() { return new TimeParamType(); }

static ReportParamType*
createBool() { return new BoolParamType(); }

static ReportParamType*
createChoice() { return new ChoiceParamType(); }

template <>
void
ParamTypeFactory::addBuiltinTypes()
{
    _types.push_back(TypePair("string", createString));
    _types.push_back(TypePair("integer", createInteger));
    _types.push_back(TypePair("double", createDouble));
    _types.push_back(TypePair("date", createDate));
    _types.push_back(TypePair("time", createTime));
    _types.push_back(TypePair("bool", createBool));
    _types.push_back(TypePair("choice", createChoice));
}
