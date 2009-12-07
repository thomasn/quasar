// $Id: report_interp.h,v 1.6 2004/08/09 23:44:53 bpepers Exp $
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

#ifndef REPORT_INTERP_H
#define REPORT_INTERP_H

#include <qvariant.h>
#include <qpair.h>
#include <qvaluevector.h>

// Types for callbacks
class ReportInterp;
typedef void (*InitCallback)(ReportInterp*,void*);
struct InterpCallback {
    QString type;
    InitCallback func;
    void* arg;
};

class ReportInterp {
public:
    ReportInterp();
    virtual ~ReportInterp();

    // Interpreter type
    virtual QString type() const=0;

    // Initialize
    virtual void initialize()=0;

    // Variables
    virtual void setVar(const QString& name, const QVariant& value)=0;
    virtual void setVar(const QString& type, const QString& name,
			const QVariant& value)=0;
    virtual QVariant getVar(const QString& name)=0;
    virtual QVariant getVar(const QString& type, const QString& name)=0;

    // Evaluate expression
    virtual QVariant evaluate(const QString& expr)=0;

    // Run code
    virtual QVariant execute(const QString& code)=0;

    // Shortcuts to returning evaluated value as a type
    bool evalBoolean(const QString& expr, bool def=false);

    // Callback on initialization
    static void addCallback(const QString& type, InitCallback func, void* arg);

protected:
    static QValueVector<InterpCallback> _callbacks;
    void runCallbacks(const QString& type);
};

#endif // REPORT_INTERP_H
