// $Id: tcl_interp.h,v 1.4 2004/08/09 23:44:53 bpepers Exp $
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

#ifndef TCL_INTERP_H
#define TCL_INTERP_H

#include "report_interp.h"
struct Tcl_Interp;

class TclInterp: public ReportInterp {
public:
    TclInterp();
    ~TclInterp();

    // Interpreter type
    QString type() const { return "tcl"; }

    // Initialize and cleanup
    void initialize();

    // Variables
    void setVar(const QString& name, const QVariant& value);
    void setVar(const QString& type, const QString& name, const QVariant& val);
    QVariant getVar(const QString& name);
    QVariant getVar(const QString& type, const QString& name);

    // Evaluate expression
    QVariant evaluate(const QString& expr);

    // Run code
    QVariant execute(const QString& code);

    // Tcl interpreter
    Tcl_Interp* interp;
};

#endif // TCL_REPORT_INTERP_H
