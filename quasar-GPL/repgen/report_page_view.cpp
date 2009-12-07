// $Id: report_page_view.cpp,v 1.8 2005/01/06 00:02:54 bpepers Exp $
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

#include "report_page_view.h"

#include <qapplication.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>

#define DPI_X(x) int((x) * metrics.logicalDpiX() * _zoom / 600.0 + .5)
#define DPI_Y(y) int((y) * metrics.logicalDpiY() * _zoom / 600.0 + .5)

ReportPageView::ReportPageView(QWidget* parent)
    : QScrollView(parent, "MainWindow"), _output(NULL), _pageNumber(1),
      _zoom(1.0)
{
    QPaintDeviceMetrics metrics(this);
    resizeContents(DPI_X(5100), DPI_Y(6600));
    setVScrollBarMode(AlwaysOn);
    setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
}

ReportPageView::~ReportPageView()
{
}

QSize
ReportPageView::sizeHint() const
{
    constPolish();
    QSize result = QSize(frameWidth()*2, frameWidth()*2);
    result += QSize(contentsWidth(),contentsHeight());

    // Limit width/height to 90% of screen width, 80% of screen height
    int screenWidth = qApp->desktop()->width();
    int screenHeight = qApp->desktop()->height();
    if (result.width() > screenWidth * .90)
	result.setWidth(int(screenWidth * .90));
    if (result.height() > screenHeight * .80)
	result.setHeight(int(screenHeight * .80));

    // Add on extra for scroll bar
    result.setWidth(result.width() + 20);

    return result;
}

QSize
ReportPageView::minimumSizeHint() const
{
    return sizeHint();
}

void
ReportPageView::setOutput(ReportOutput* output)
{
    _output = output;
    _pageNumber = 1;

    if (output != NULL) {
	QPaintDeviceMetrics metrics(this);
	resizeContents(DPI_X(output->pageWidth), DPI_Y(output->pageHeight));
    }
    viewport()->update();
}

void
ReportPageView::setPageNumber(int pageNumber)
{
    _pageNumber = pageNumber;
    viewport()->update();
}

void
ReportPageView::setZoom(double zoom)
{
    _zoom = zoom;
    if (_zoom < .2) _zoom = .2;

    if (_output != NULL) {
	QPaintDeviceMetrics metrics(this);
	resizeContents(DPI_X(_output->pageWidth), DPI_Y(_output->pageHeight));
    }
    viewport()->update();
}

void
ReportPageView::nextPage()
{
    if (_output == NULL) return;
    if (_pageNumber < int(_output->pages.size())) {
	++_pageNumber;
	viewport()->update();
    }
}

void
ReportPageView::prevPage()
{
    if (_output == NULL) return;
    if (_pageNumber > 1) {
	--_pageNumber;
	viewport()->update();
    }
}

void
ReportPageView::firstPage()
{
    if (_output == NULL) return;
    _pageNumber = 1;
    viewport()->update();
}

void
ReportPageView::lastPage()
{
    if (_output == NULL) return;
    _pageNumber = _output->pages.size();
    viewport()->update();
}

void
ReportPageView::drawContents(QPainter* p, int, int, int, int)
{
    // Clear the page
    p->fillRect(0, 0, contentsWidth(), contentsHeight(), QColor("white"));
    if (_output == NULL) return;

    // If not a valid page then done
    if (_pageNumber < 1 || _pageNumber > int(_output->pages.size())) {
	return;
    }

    // Paint the page
    ReportPage& page = _output->pages[_pageNumber - 1];
    page.paint(p, _zoom);
}
