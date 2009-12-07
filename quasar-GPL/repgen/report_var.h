// $Id: report_var.h,v 1.10 2005/01/30 04:16:25 bpepers Exp $
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

#ifndef REPORT_VAR_H
#define REPORT_VAR_H

#include <qdom.h>

class ReportInterp;

class ReportVar {
public:
    ReportVar();
    ReportVar(QDomElement e);
    ~ReportVar();

    // Reset types
    enum { RESET_NONE, RESET_REPORT, RESET_PAGE, RESET_GROUP };

    QString name;		// Name to identify variable
    QString calcExpr;		// Expression to calculate
    int resetType;		// Automatic reset type
    QString resetGroup;		// Group to reset on changes
    QString resetExpr;		// Expression to reset

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    bool toXML(QDomElement& e) const;

    // Initialize and calculate variable
    void initialize(ReportInterp* interp);
    void calculate(ReportInterp* interp);
};

#endif // REPORT_VAR_H
