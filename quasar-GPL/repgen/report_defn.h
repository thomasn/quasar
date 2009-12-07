// $Id: report_defn.h,v 1.22 2005/01/30 04:16:25 bpepers Exp $
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

#ifndef REPORT_DEFN_H
#define REPORT_DEFN_H

#include <qmap.h>
#include <qpair.h>

#include "data_source.h"
#include "report_param.h"
#include "report_var.h"
#include "report_group.h"
#include "report_interp.h"
#include "report_output.h"

// Used for passing parameter values to generate
typedef QMap<QString,QVariant> ParamMap;

// Used for indexing a page and an element in the page
typedef QPair<int,int> ElementBinding;

class ReportDefn {
public:
    ReportDefn();
    ReportDefn(const ReportDefn& report);
    ~ReportDefn();

    // Print order
    enum { PRINT_VERTICAL, PRINT_HORIZONTAL };

    // When no data
    enum { NO_PAGES, BLANK_PAGE, NO_DETAIL };

    QString name;		// Name of report
    QString module;		// Module report belongs to
    int pageWidth;		// Width of page
    int pageHeight;		// Height of page
    bool portrait;		// Portrait or landscape orientation
    int whenNoData;		// What to do when no data
    int leftMargin;		// Left margin of page
    int rightMargin;		// Right margin of page
    int topMargin;		// Top margin of page
    int bottomMargin;		// Bottom margin of page
    QString initCode;		// Code to initialize environment
    DataSource* source;		// Method if getting data rows

    // Parameters (run-time entered values), variables (report defined
    // calculated values), and groups (grouping of rows)
    ReportParamVector parameters;
    QValueVector<ReportVar> variables;
    QValueVector<ReportGroup> groups;

    // Sections in report
    QValueVector<ReportBand> backgrounds;
    QValueVector<ReportBand> titles;
    QValueVector<ReportBand> pageHeaders;
    QValueVector<ReportBand> details;
    QValueVector<ReportBand> pageFooters;
    QValueVector<ReportBand> summaries;

    // Load and save in XML format
    bool load(const QString& filePath);
    bool save(const QString& filePath) const;

    // Assignment
    ReportDefn& operator=(const ReportDefn& report);

    // Prompt for params
    bool getParams(QWidget* parent, ParamMap& params);

    // Fill in the output using the definition, data source, and params
    bool generate(const ParamMap& params, ReportOutput& output);

private:
    ReportOutput* output;		// Current output being built
    ReportInterp* interp;		// Interpreter for code/expr/...
    bool newPage;			// At top of new page?
    int offsetX;			// Position to place next band
    int offsetY;			// Position to place next band
    int pageNumber;			// Current page number
    int rowNumber;			// Current row number in data source
    int rowCount;			// Running count of rows processed
    ReportPage* currentPage;		// Current page to put bands on

    // These are text elements that need to be calculated at the
    // given point in report generation.
    QValueVector<ElementBinding> reportBoundTexts;
    QValueVector<ElementBinding> pageBoundTexts;
    QValueVector<ElementBinding> groupBoundTexts;

    // Get the next row from the database and set the old_column and
    // column values in the interpreter with the current row values
    bool next();

    // Handle user variable definitions
    void initializeVars(int resetType, const QString& group="");
    void calculateVars();

    // Resolve bound texts
    PageElement& boundElement(ElementBinding& binding);
    void resolveReportBoundTexts();
    void resolvePageBoundTexts();
    void resolveGroupBoundTexts();
    void resolveText(PageElement& element);

    // Processing for the report
    void reportStart();
    void reportRow();
    void reportEnd();

    // Add sections to the page
    void fillTitles();
    void fillSummaries();
    void fillPageHeaders();
    void fillPageFooters();
    void fillGroupHeaders(ReportGroup& group);
    void fillGroupFooters(ReportGroup& group);

    // Check for space on the page for a band
    void checkSpace(const ReportBand& band);

    // Get the height of a section taking into account printWhen exprs
    int sectionHeight(QValueVector<ReportBand>& section);

    // Add a band to a page moving down the page
    void addBand(const ReportBand& band);

    // Add a new page to the report and make it the current page
    void addPage(bool resetPageNumber=false);
};

#endif // REPORT_DEFN_H
