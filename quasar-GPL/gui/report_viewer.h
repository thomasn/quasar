// $Id: report_viewer.h,v 1.16 2005/02/05 11:54:24 bpepers Exp $
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
//

#ifndef REPORT_VIEWER_H
#define REPORT_VIEWER_H

#include <qmainwindow.h>
#include <qstringlist.h>

#include "report_defn.h"

class ReportOutput;
class ReportPageView;
class LineEdit;
class QLabel;

class ReportViewer: public QMainWindow {
    Q_OBJECT
public:
    ReportViewer(ReportDefn& report, ParamMap& params, ReportOutput* output);
    ~ReportViewer();

public slots:
    void firstPage();
    void prevPage();
    void nextPage();
    void lastPage();
    void pickPage();
    void pageUp();
    void pageDown();
    void lineUp();
    void lineDown();
    void zoomIn();
    void zoomOut();
    void regenerate();
    void print();

protected:
    void keyPressEvent(QKeyEvent* e);
    void setPageLabel();

    ReportDefn _report;
    ParamMap _params;
    ReportOutput* _output;

    LineEdit* _name;
    ReportPageView* _page;
    QLabel* _pageLabel;
};

#endif // REPORT_VIEWER_H
