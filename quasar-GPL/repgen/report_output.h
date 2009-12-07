// $Id: report_output.h,v 1.6 2004/10/12 05:48:02 bpepers Exp $
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

#ifndef REPORT_OUTPUT_H
#define REPORT_OUTPUT_H

#include "report_page.h"
class ReportDefn;

class ReportOutput {
public:
    ReportOutput();
    ~ReportOutput();

    // Initialize from a report definition
    void initialize(ReportDefn& report);

    // Clear out all current data
    void clear();

    // Print output
    void print(QWidget* parent, const QString& printer="", bool doSetup=true);

    // Add a page returning a pointer to it
    ReportPage* addPage();

    QString name;		// Name of report
    int pageWidth;		// Width of page
    int pageHeight;		// Height of page
    bool portrait;		// Portrait or landscape orientation

    // Pages of output
    QValueVector<ReportPage> pages;

    // Load and save in XML format
    bool load(const QString& filePath);
    bool save(const QString& filePath) const;
};

#endif // REPORT_OUTPUT_H
