// $Id: report_output.cpp,v 1.9 2005/02/17 09:55:03 bpepers Exp $
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

#include "report_output.h"
#include "report_defn.h"
#include "user_config.h"

#include <qfile.h>
#include <qprinter.h>
#include <qpainter.h>

struct PaperSize {
    int width, height;
    QPrinter::PageSize size;
};

PaperSize paperSizes[] = {
    {  4961,  7016, QPrinter::A4 },
    {  4157,  5906, QPrinter::B5 },
    {  5100,  6600, QPrinter::Letter },
    {  5100,  8400, QPrinter::Legal },
    {  4500,  6000, QPrinter::Executive },
    { 19866, 28086, QPrinter::A0 },
    { 14031, 19866, QPrinter::A1 },
    {  9921, 14031, QPrinter::A2 },
    {  7016,  9921, QPrinter::A3 },
    {  3496,  7016, QPrinter::A5 },
    {  2480,  3496, QPrinter::A6 },
    {  1748,  2480, QPrinter::A7 },
    {  1228,  1748, QPrinter::A8 },
    {   874,  1228, QPrinter::A9 },
    { 23622, 33402, QPrinter::B0 },
    { 16701, 23622, QPrinter::B1 },
    { 11811, 16701, QPrinter::B2 },
    {  8339, 11811, QPrinter::B3 },
    {  5906,  8339, QPrinter::B4 },
    {  2953,  5906, QPrinter::B6 },
    {  2079,  2953, QPrinter::B7 },
    {  1465,  2079, QPrinter::B8 },
    {  1039,  1465, QPrinter::B9 },
    {   732,  1039, QPrinter::B10 },
    {  3827,  5409, QPrinter::C5E },
    {  2480,  5700, QPrinter::Comm10E },
    {  2598,  5197, QPrinter::DLE },
    {  4961,  7795, QPrinter::Folio },
    { 10200,  6600, QPrinter::Ledger },
    {  6600, 10200, QPrinter::Tabloid },
    { -1, -1, QPrinter::Letter }
};

// Convert width/height in 600 DPI to QPrinter::PageSize
QPrinter::PageSize
getSize(int width, int height)
{
    int index = 0;
    while (true) {
	PaperSize& paperSize = paperSizes[index++];
	if (paperSize.width == -1)
	    return paperSize.size;

	if (paperSize.width != width) continue;
	if (paperSize.height != height) continue;

	return paperSize.size;
    }
    return QPrinter::Letter;
}

// Convert QPrinter::PageSize to width/height in 600 DPI
bool
getSize(QPrinter::PageSize size, int& width, int& height)
{
    int index = 0;
    while (true) {
	PaperSize& paperSize = paperSizes[index++];
	if (paperSize.width == -1)
	    return false;

	if (paperSize.size == size) {
	    width = paperSize.width;
	    height = paperSize.height;
	    return true;
	}
    }
    return false;
}

ReportOutput::ReportOutput()
    : name(""), pageWidth(0), pageHeight(0), portrait(true)
{
}

ReportOutput::~ReportOutput()
{
    clear();
}

void
ReportOutput::initialize(ReportDefn& report)
{
    clear();
    name = report.name;
    pageWidth = report.pageWidth;
    pageHeight = report.pageHeight;
    portrait = report.portrait;
}

void
ReportOutput::clear()
{
    name = "";
    pageWidth = 0;
    pageHeight = 0;
    portrait = true;
    pages.clear();
}

void
ReportOutput::print(QWidget* parent, const QString& printerName, bool doSetup)
{
    if (pages.size() == 0)
	return;

    UserConfig config;
    config.load();

    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);
    printer.setMinMax(1, pages.size());
    printer.setColorMode(QPrinter::Color);

    // Handle Letter to A4 conversion
    QPrinter::PageSize size = getSize(pageWidth, pageHeight);
    if (size == QPrinter::Letter && config.preferA4)
	size = QPrinter::A4;
    printer.setPageSize(size);

    // Set orientation
    if (portrait)
	printer.setOrientation(QPrinter::Portrait);
    else
	printer.setOrientation(QPrinter::Landscape);

    if (!printerName.isEmpty())
	printer.setPrinterName(printerName);
    if (doSetup)
	if (!printer.setup(parent)) return;

    // Check if prefer A4
    if (size == QPrinter::Letter && printer.pageSize() == QPrinter::A4) {
	config.preferA4 = true;
	config.save(true);
    }

    // Set zoom based on report page size vrs printer page size
    double zoom = 1.0;
    int printerWidth, printerHeight;
    getSize(printer.pageSize(), printerWidth, printerHeight);
    if (printerWidth < pageWidth || printerHeight < pageHeight) {
	double zoomWidth = printerWidth / double(pageWidth);
	double zoomHeight = printerHeight / double(pageHeight);
	zoom = (zoomWidth < zoomHeight) ? zoomWidth : zoomHeight;
    }

    QPainter p;
    p.begin(&printer);
    if (printer.pageOrder() == QPrinter::FirstPageFirst) {
	// Page range as for look values
	int start = printer.fromPage() - 1;
	int end = printer.toPage() - 1;

	// Sanity check on the values
	if (start < 0)
	    start = 0;
	if (end < 0 || end >= int(pages.size()))
	    end = int(pages.size() - 1);

	for (int i = start; i <= end; ++i) {
	    ReportPage& page = pages[i];
	    page.paint(&p, zoom);
	    if (i < end) printer.newPage();
	}
    } else {
	// Page range as for look values
	int start = printer.fromPage() - 1;
	int end = printer.toPage() - 1;

	// Sanity check on the values
	if (start < 0)
	    start = 0;
	if (end >= int(pages.size()))
	    end = int(pages.size() - 1);

	for (int i = end; i >= start; --i) {
	    ReportPage& page = pages[i];
	    page.paint(&p, zoom);
	    if (i > start) printer.newPage();
	}
    }
    p.end();
}

ReportPage*
ReportOutput::addPage()
{
    ReportPage page;
    pages.push_back(page);
    return &pages.back();
}

bool
ReportOutput::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) {
	qWarning("Failed to open \"" + filePath + "\" for reading");
	return false;
    }

    QDomDocument doc("ReportOutput");
    QString errorMsg;
    int errorLine, errorCol;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorCol)) {
	qWarning("Error processing the report file on line %d, "
		 "column %d:\n    " + errorMsg, errorLine, errorCol);
	return false;
    }

    QDomElement root = doc.documentElement();

    name = root.attribute("name");
    pageWidth = root.attribute("pageWidth", "5100").toInt();
    pageHeight = root.attribute("pageHeight", "6600").toInt();

    QString orient = root.attribute("orientation", "portrait").lower();
    if (orient == "portrait") portrait = true;
    else if (orient == "landscape") portrait = false;
    else qWarning("Invalid orientation: " + orient);

    QDomNodeList nodes = root.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	QString tag = e.tagName();
	if (tag == "page")
	    pages.push_back(ReportPage(e));
	else
	    qWarning("Unknown report output tag: " + tag);
    }

    return true;
}

bool
ReportOutput::save(const QString& filePath) const
{
    QDomDocument doc("ReportOutput");
    QDomElement root = doc.createElement("ReportOutput");

    QString xmlInstruction = "version=\"1.0\" encoding=\"UTF-8\"";
    doc.appendChild(doc.createProcessingInstruction("xml", xmlInstruction));
    doc.appendChild(root);

    root.setAttribute("name", name);
    root.setAttribute("pageWidth", QString::number(pageWidth));
    root.setAttribute("pageHeight", QString::number(pageHeight));
    root.setAttribute("orientation", portrait ? "portrait" : "landscape");

    for (unsigned int i = 0; i < pages.size(); ++i) {
	QDomElement e = doc.createElement("page");
	pages[i].toXML(e);
	root.appendChild(e);
    }

    QFile file(filePath);
    if (!file.open(IO_WriteOnly)) {
	qWarning("Failed to open \"" + filePath + "\" for writing");
	return false;
    }

    QTextStream stream(&file);
    stream << doc.toString();

    file.close();
    return true;
}
