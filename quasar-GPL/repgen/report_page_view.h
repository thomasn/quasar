// $Id: report_page_view.h,v 1.3 2005/01/06 00:02:54 bpepers Exp $
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

#ifndef REPORT_PAGE_VIEW_H
#define REPORT_PAGE_VIEW_H

#include <qscrollview.h>
#include "report_output.h"

class ReportPageView: public QScrollView {
    Q_OBJECT
public:
    ReportPageView(QWidget* parent);
    ~ReportPageView();

    int pageNumber() { return _pageNumber; }
    double zoom() { return _zoom; }

public slots:
    void setOutput(ReportOutput* output);
    void setPageNumber(int pageNumber);
    void setZoom(double zoom);
    void nextPage();
    void prevPage();
    void firstPage();
    void lastPage();

protected:
    QSize sizeHint() const;
    QSize minimumSizeHint() const;
    void drawContents(QPainter* p, int cx, int cy, int cw, int ch);

    ReportOutput* _output;
    int _pageNumber;
    double _zoom;
};

#endif // REPORT_PAGE_VIEW_H
