// $Id: report_defn.cpp,v 1.36 2005/03/04 18:37:19 bpepers Exp $
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

#include "report_defn.h"
#include "report_output.h"
#include "text_element.h"
#include "data_source_factory.h"
#include "interp_factory.h"
#include "param_dialog.h"

#include <qfile.h>
#include <assert.h>

ReportDefn::ReportDefn()
    : pageWidth(5100), pageHeight(6600), portrait(true),
      whenNoData(NO_DETAIL), leftMargin(150), rightMargin(150),
      topMargin(300), bottomMargin(300), initCode(""), source(NULL),
      output(NULL), interp(NULL), currentPage(NULL)
{
}

ReportDefn::ReportDefn(const ReportDefn& report)
    : name(report.name), module(report.module), pageWidth(report.pageWidth),
      pageHeight(report.pageHeight), portrait(report.portrait),
      whenNoData(report.whenNoData), leftMargin(report.leftMargin),
      rightMargin(report.rightMargin), topMargin(report.topMargin),
      bottomMargin(report.bottomMargin), initCode(report.initCode),
      source(NULL), parameters(report.parameters), variables(report.variables),
      groups(report.groups), backgrounds(report.backgrounds),
      titles(report.titles), pageHeaders(report.pageHeaders),
      details(report.details), pageFooters(report.pageFooters),
      summaries(report.summaries), output(NULL), interp(NULL),
      currentPage(NULL)
{
    if (report.source != NULL)
	source = report.source->clone();
}

ReportDefn::~ReportDefn()
{
    delete source;
}

ReportDefn&
ReportDefn::operator=(const ReportDefn& report)
{
    if (&report != this) {
	delete source;

	name = report.name;
	module = report.module;
	pageWidth = report.pageWidth;
	pageHeight = report.pageHeight;
	portrait = report.portrait;
	whenNoData = report.whenNoData;
	leftMargin = report.leftMargin;
	rightMargin = report.rightMargin;
	topMargin = report.topMargin;
	bottomMargin = report.bottomMargin;
	initCode = report.initCode;
	source = NULL;
	parameters = report.parameters;
	variables = report.variables;
	groups = report.groups;
	backgrounds = report.backgrounds;
	titles = report.titles;
	pageHeaders = report.pageHeaders;
	details = report.details;
	pageFooters = report.pageFooters;
	summaries = report.summaries;
	output = NULL;
	interp = NULL;
	currentPage = NULL;

	if (report.source != NULL)
	    source = report.source->clone();
    }
    return *this;
}

bool
ReportDefn::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) {
	qWarning("Failed to open \"" + filePath + "\" for reading");
	return false;
    }

    QDomDocument doc("ReportDefn");
    QString errorMsg;
    int errorLine, errorCol;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorCol)) {
	qWarning("Error processing the report file on line %d, "
		 "column %d:\n    " + errorMsg, errorLine, errorCol);
	return false;
    }

    QDomElement root = doc.documentElement();

    name = root.attribute("name");
    module = root.attribute("module");
    pageWidth = root.attribute("pageWidth", "5100").toInt();
    pageHeight = root.attribute("pageHeight", "6600").toInt();
    leftMargin = root.attribute("leftMargin", "150").toInt();
    rightMargin = root.attribute("rightMargin", "150").toInt();
    topMargin = root.attribute("topMargin", "300").toInt();
    bottomMargin = root.attribute("bottomMargin", "300").toInt();

    QString orient = root.attribute("orientation", "portrait").lower();
    if (orient == "portrait") portrait = true;
    else if (orient == "landscape") portrait = false;
    else qWarning("Invalid orientation: " + orient);

    QString noData = root.attribute("whenNoData", "NoDetail").lower();
    if (noData == "nopages") whenNoData = NO_PAGES;
    else if (noData == "blankpage") whenNoData = BLANK_PAGE;
    else if (noData == "nodetail") whenNoData = NO_DETAIL;
    else qWarning("Invalid whenNoData: " + noData);

    QDomNodeList nodes = root.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	QString tag = e.tagName();
	if (tag == "dataSource") {
	    QString type = e.attribute("type");
	    source = DataSourceFactory::create(type);
	    if (source != NULL)
		source->fromXML(e);
	    else
		qWarning("Invalid data source type: " + type);
	} else if (tag == "initialize")
	    initCode = e.text();
	else if (tag == "parameter")
	    parameters.push_back(ReportParam(e));
	else if (tag == "variable")
	    variables.push_back(ReportVar(e));
	else if (tag == "group")
	    groups.push_back(ReportGroup(e));
	else if (tag == "background")
	    backgrounds.push_back(ReportBand(e));
	else if (tag == "title")
	    titles.push_back(ReportBand(e));
	else if (tag == "pageHeader")
	    pageHeaders.push_back(ReportBand(e));
	else if (tag == "detail")
	    details.push_back(ReportBand(e));
	else if (tag == "pageFooter")
	    pageFooters.push_back(ReportBand(e));
	else if (tag == "summary")
	    summaries.push_back(ReportBand(e));
	else
	    qWarning("Unknown report defn tag: " + tag);
    }

    return true;
}

bool
ReportDefn::save(const QString&) const
{
    // TODO: save to XML
    return false;
}

bool
ReportDefn::getParams(QWidget* parent, ParamMap& params)
{
    if (parameters.size() == 0)
	return true;

    ParamDialog* dialog = new ParamDialog(parent, parameters, params);
    int result = dialog->exec();
    ParamMap newParams = dialog->getParameters();
    delete dialog;

    if (result != QDialog::Accepted)
	return false;

    params = newParams;
    return true;
}

bool
ReportDefn::generate(const ParamMap& params, ReportOutput& out)
{
    output = &out;
    output->initialize(*this);

    interp = InterpFactory::create("tcl");
    interp->initialize();

    // Set parameter values
    for (unsigned int i = 0; i < parameters.size(); ++i) {
	QString name = parameters[i].name;
	interp->setVar("param", name, params[name]);
    }

    // Initialize code
    interp->execute(initCode);
    interp->setVar("sys", "row_number", "1");

    // Initialize data source
    if (!source->initialize(interp))
	return false;

    // Reset group values
    for (unsigned int i = 0; i < groups.size(); ++i)
	groups[i].currentValue = "junk foo bar";

    // Process report rows
    reportStart();
    rowNumber = 1;
    while (next()) {
	reportRow();
	++rowNumber;
    }
    reportEnd();

    // Special handling when there were no rows
    if (rowNumber == 1) {
	switch (whenNoData) {
	case NO_PAGES:
	    output->pages.clear();
	    break;
	case BLANK_PAGE:
	    output->pages.clear();
	    addPage(true);
	    break;
	case NO_DETAIL:
	    break;
	}
    }

    delete interp;
    return true;
}

bool
ReportDefn::next()
{
    if (!source->next(interp)) return false;

    for (unsigned int i = 0; i < source->columns.size(); ++i) {
	QString column = source->columns[i];

	QVariant oldValue = interp->getVar("column", column);
	interp->setVar("column_old", column, oldValue);

	QVariant value = source->get(interp, i);
	interp->setVar("column", column, value);
    }
    return true;
}

void
ReportDefn::initializeVars(int resetType, const QString& groupName)
{
    for (unsigned int i = 0; i < variables.size(); ++i) {
	ReportVar& var = variables[i];
	if (var.resetType == ReportVar::RESET_NONE)
	    continue;

	bool initialize = false;
	switch (resetType) {
	case ReportVar::RESET_REPORT:
	    initialize = true;
	    break;
	case ReportVar::RESET_PAGE:
	    if (var.resetType == ReportVar::RESET_PAGE)
		initialize = true;
	    break;
	case ReportVar::RESET_GROUP:
	    if (var.resetType == ReportVar::RESET_GROUP)
		initialize = (var.resetGroup == groupName);
	    break;
	}

	if (initialize)
	    var.initialize(interp);
    }
}

void
ReportDefn::calculateVars()
{
    for (unsigned int i = 0; i < variables.size(); ++i) {
	ReportVar& var = variables[i];
	var.calculate(interp);
    }
}

PageElement&
ReportDefn::boundElement(ElementBinding& binding)
{
    return output->pages[binding.first].elements[binding.second];
}

void
ReportDefn::resolveReportBoundTexts()
{
    unsigned int pageCount = output->pages.size();
    interp->setVar("sys", "page_count", QString::number(pageCount));

    for (unsigned int i = 0; i < reportBoundTexts.size(); ++i) {
	ElementBinding& binding = reportBoundTexts[i];

	int pageNum = binding.first + 1;
	interp->setVar("sys", "page_number", QString::number(pageNum));

	resolveText(boundElement(binding));
    }
    reportBoundTexts.clear();
}

void
ReportDefn::resolvePageBoundTexts()
{
    for (unsigned int i = 0; i < pageBoundTexts.size(); ++i)
	resolveText(boundElement(pageBoundTexts[i]));
    pageBoundTexts.clear();
}

void
ReportDefn::resolveGroupBoundTexts()
{
    for (unsigned int i = 0; i < groupBoundTexts.size(); ++i)
	resolveText(boundElement(groupBoundTexts[i]));
    groupBoundTexts.clear();
}

void
ReportDefn::resolveText(PageElement& element)
{
    element.text = element.evaluate(interp);
}

void
ReportDefn::reportStart()
{
    initializeVars(ReportVar::RESET_REPORT);
    addPage(true);
    fillTitles();
    fillPageHeaders();
}

void
ReportDefn::reportRow()
{
    // Check for groups that have changed their value
    for (unsigned int i = 0; i < groups.size(); ++i) {
	QVariant value = groups[i].newValue(interp);
	if (value == groups[i].currentValue) continue;

	// Print group footers only if this isn't the first groups printed
	if (rowNumber > 1) {
	    for (int j = groups.size() - 1; j >= int(i); --j)
		fillGroupFooters(groups[j]);
	}

	// Print the group headers and then break since we know we are done
	for (unsigned int j = i; j < groups.size(); ++j)
	    fillGroupHeaders(groups[j]);
	break;
    }

    calculateVars();
    interp->setVar("sys", "row_number", QString::number(rowNumber));

    for (unsigned int i = 0; i < details.size(); ++i) {
	const ReportBand& detail = details[i];
	if (detail.shouldPrint(interp)) {
	    checkSpace(detail);
	    addBand(detail);
	}
    }
}

void
ReportDefn::reportEnd()
{
    if (rowNumber != 1) {
	for (int i = int(groups.size() - 1); i >= 0; --i)
	    fillGroupFooters(groups[i]);
    }
    fillSummaries();
    fillPageFooters();
    resolveReportBoundTexts();
}

void
ReportDefn::fillTitles()
{
    for (unsigned int i = 0; i < titles.size(); ++i) {
	ReportBand& title = titles[i];
	if (title.shouldPrint(interp)) {
	    checkSpace(title);
	    addBand(title);
	}
    }
}

void
ReportDefn::fillSummaries()
{
    for (unsigned int i = 0; i < summaries.size(); ++i) {
	ReportBand& summary = summaries[i];
	if (summary.shouldPrint(interp)) {
	    checkSpace(summary);
	    addBand(summary);
	}
    }
}

void
ReportDefn::fillPageHeaders()
{
    initializeVars(ReportVar::RESET_PAGE);
    for (unsigned int i = 0; i < pageHeaders.size(); ++i) {
	ReportBand& pageHeader = pageHeaders[i];
	if (pageHeader.shouldPrint(interp))
	    addBand(pageHeader);
    }
}

void
ReportDefn::fillPageFooters()
{
    offsetY = pageHeight - bottomMargin;
    for (int i = int(pageFooters.size()) - 1; i >= 0; --i) {
	ReportBand& pageFooter = pageFooters[i];
	if (pageFooter.shouldPrint(interp)) {
	    offsetY -= pageFooter.height;
	    addBand(pageFooter);
	    offsetY -= pageFooter.height;
	}
    }
    resolvePageBoundTexts();
}

void
ReportDefn::fillGroupHeaders(ReportGroup& group)
{
    initializeVars(ReportVar::RESET_GROUP, group.name);
    group.currentValue = group.newValue(interp);
    interp->setVar("group", group.name, group.currentValue);

    for (unsigned int i = 0; i < group.headers.size(); ++i) {
	const ReportBand& header = group.headers[i];
	if (header.shouldPrint(interp)) {
	    checkSpace(header);
	    addBand(header);
	}
    }
}

void
ReportDefn::fillGroupFooters(ReportGroup& group)
{
    for (unsigned int i = 0; i < group.footers.size(); ++i) {
	const ReportBand& footer = group.footers[i];
	if (footer.shouldPrint(interp)) {
	    checkSpace(footer);
	    addBand(footer);
	}
    }
    resolveGroupBoundTexts();
}

void
ReportDefn::checkSpace(const ReportBand& band)
{
    int pageFooterY = pageHeight - bottomMargin - sectionHeight(pageFooters);
    if (offsetY + band.height > pageFooterY) {
	fillPageFooters();
	addPage();
	fillPageHeaders();
    }
}

int
ReportDefn::sectionHeight(QValueVector<ReportBand>& section)
{
    int height = 0;
    for (unsigned int i = 0; i < section.size(); ++i) {
	ReportBand& band = section[i];
	if (band.shouldPrint(interp))
	    height += band.height;
    }
    return height;
}

void
ReportDefn::addBand(const ReportBand& band)
{
    // Check if band should be at bottom of page
    if (!band.onBottom.isEmpty()) {
	bool onBottom = interp->evalBoolean(band.onBottom);
	if (onBottom) {
	    int pageFooterY = pageHeight - bottomMargin -
		sectionHeight(pageFooters);
	    offsetY = pageFooterY - band.height;
	}
    }

    // Check if band should be on a new page
    if (!band.onNewPage.isEmpty()) {
	bool onNewPage = interp->evalBoolean(band.onNewPage);
	int pageHeaderY = topMargin + sectionHeight(pageHeaders);
	if (rowNumber == 1) pageHeaderY += sectionHeight(titles);
	if (onNewPage && offsetY != pageHeaderY) {
	    offsetY = pageHeight;
	    checkSpace(band);
	}
    }

    interp->execute(band.beforeCode);

    for (unsigned int i = 0; i < band.elements.size(); ++i) {
	ReportElement* element = band.elements[i];
	if (!element->shouldPrint(interp)) continue;

	QValueVector<PageElement> elements;
	element->generate(interp, leftMargin, offsetY, elements);

	for (unsigned int j = 0; j < elements.size(); ++j) {
	    currentPage->elements.push_back(elements[j]);

	    int pageNum = output->pages.size() - 1;
	    int elementNum = currentPage->elements.size() - 1;
	    ElementBinding binding(pageNum, elementNum);

	    PageElement& pe = currentPage->elements.back();
	    if (pe.type == PageElement::TEXT && pe.text.isEmpty()) {
		switch (pe.evalTime) {
		case TextElement::NOW:
		    resolveText(pe);
		    break;
		case TextElement::REPORT:
		    reportBoundTexts.push_back(binding);
		    break;
		case TextElement::PAGE:
		    pageBoundTexts.push_back(binding);
		    break;
		}
	    }
	}
    }

    offsetY += band.height;
    interp->execute(band.afterCode);

    // Check if should force a new page after this band
    if (!band.newPageAfter.isEmpty()) {
	bool newPageAfter = interp->evalBoolean(band.newPageAfter);
	if (newPageAfter)
	    offsetY = pageHeight;
    }
}

// Adds a new page to the report initializing the 
void
ReportDefn::addPage(bool resetPageNumber)
{
    currentPage = output->addPage();

    if (resetPageNumber)
	pageNumber = 1;
    else
	pageNumber += 1;

    interp->setVar("sys", "page_number", QString::number(pageNumber));

    // Print out each background band on top of each other always
    // resetting the offsetY after printing
    offsetY = topMargin;
    for (unsigned int i = 0; i < backgrounds.size(); ++i) {
	ReportBand& background = backgrounds[i];
	if (background.height <= pageHeight - bottomMargin - offsetY) {
	    if (background.shouldPrint(interp)) {
		addBand(background);
		offsetY = topMargin;
	    }
	}
    }
}
