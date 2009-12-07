// $Id: report_page.h,v 1.7 2005/01/06 00:02:54 bpepers Exp $
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

#ifndef REPORT_PAGE_H
#define REPORT_PAGE_H

#include "page_element.h"

class QPainter;

class ReportPage {
public:
    ReportPage();
    ReportPage(QDomElement e);
    ~ReportPage();

    // Elements on the page
    QValueVector<PageElement> elements;

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    bool toXML(QDomElement& e) const;

    // Page this page to a painter
    void paint(QPainter* p, double zoom=1.0) const;
};

#endif // REPORT_PAGE_H
