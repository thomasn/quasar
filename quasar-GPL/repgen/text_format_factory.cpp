// $Id: text_format_factory.cpp,v 1.5 2005/06/22 23:17:51 bpepers Exp $
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

#include "text_format_factory.h"
#include "date_text_format.h"
#include "time_text_format.h"
#include "number_text_format.h"
#include "money_text_format.h"
#include "percent_text_format.h"

template
QValueVector<TextFormatFactory::TypePair> TextFormatFactory::_types;

static ReportTextFormat*
createDate() { return new DateTextFormat(); }

static ReportTextFormat*
createTime() { return new TimeTextFormat(); }

static ReportTextFormat*
createNumber() { return new NumberTextFormat(); }

static ReportTextFormat*
createMoney() { return new MoneyTextFormat(); }

static ReportTextFormat*
createPercent() { return new PercentTextFormat(); }

template <>
void
TextFormatFactory::addBuiltinTypes()
{
    _types.push_back(TypePair("date", createDate));
    _types.push_back(TypePair("time", createTime));
    _types.push_back(TypePair("number", createNumber));
    _types.push_back(TypePair("money", createMoney));
    _types.push_back(TypePair("percent", createPercent));
}
