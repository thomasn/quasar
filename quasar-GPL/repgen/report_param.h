// $Id: report_param.h,v 1.11 2005/01/24 10:01:22 bpepers Exp $
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

#ifndef REPORT_PARAM_H
#define REPORT_PARAM_H

#include <qdom.h>
#include <qmap.h>
#include <qvaluevector.h>

class ReportParam {
public:
    ReportParam();
    ReportParam(QDomElement e);
    ~ReportParam();

    QString name;			// Name to identify param
    QString description;		// Description (for prompting)
    QString type;			// Type of data
    QString defaultValue;		// Default value if not specified
    bool manditory;			// Blank value allowed?
    QMap<QString,QString> attributes;	// Type specific attributes

    // Get/set attributes
    QString getAttribute(const QString& key) const;
    void setAttribute(const QString& key, const QString& value);

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    bool toXML(QDomElement& e) const;
};

typedef QValueVector<ReportParam> ReportParamVector;

#endif // REPORT_PARAM_H
