// $Id: report_interp.cpp,v 1.9 2004/08/11 06:05:17 bpepers Exp $
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

#include "report_interp.h"

QValueVector<InterpCallback> ReportInterp::_callbacks;

ReportInterp::ReportInterp()
{
}

ReportInterp::~ReportInterp()
{
}

bool
ReportInterp::evalBoolean(const QString& expr, bool defaultValue)
{
    if (expr.isEmpty()) return defaultValue;
    QVariant value = evaluate(expr);
    QString text = value.toString().lower();
    if (text == "1" || text == "true" || text == "yes" || text == "on")
	return true;
    return false;
}

void
ReportInterp::addCallback(const QString& type, InitCallback func, void* arg)
{
    InterpCallback callback;
    callback.type = type;
    callback.func = func;
    callback.arg = arg;
    _callbacks.push_back(callback);
}

void
ReportInterp::runCallbacks(const QString& type)
{
    for (unsigned int i = 0; i < _callbacks.size(); ++i) {
	if (_callbacks[i].type.lower() == type.lower())
	    _callbacks[i].func(this, _callbacks[i].arg);
    }
}
