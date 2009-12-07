// $Id: report_band.h,v 1.8 2005/01/30 04:16:25 bpepers Exp $
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

#ifndef REPORT_BAND_H
#define REPORT_BAND_H

#include "report_element.h"
#include <qvaluevector.h>
#include <qdom.h>

class ReportInterp;

class ReportBand {
public:
    ReportBand();
    ReportBand(QDomElement e);
    ReportBand(const ReportBand& band);
    ~ReportBand();

    // Assignment
    ReportBand& operator=(const ReportBand& rhs);

    int height;				   // Height of the band
    QString printWhen;			   // Expression controlling printing
    QString beforeCode;			   // Code to run before printing
    QString afterCode;			   // Code to run after printing
    QString onNewPage;			   // Start on a new page?
    QString newPageAfter;		   // New page after this band?
    QString onBottom;			   // Print at bottom of page?
    QValueVector<ReportElement*> elements; // Elements to show in the band

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    bool toXML(QDomElement& e) const;

    // Evaluate printWhen and return result
    bool shouldPrint(ReportInterp* interp) const;
};

#endif // REPORT_BAND_H
