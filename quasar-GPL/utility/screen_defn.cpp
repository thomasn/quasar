// $Id: screen_defn.cpp,v 1.17 2005/04/12 08:56:14 bpepers Exp $
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

#include "screen_defn.h"
#include "quasar_misc.h"

#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qregexp.h>

ScreenDefn::ScreenDefn()
{
}

ScreenDefn::ScreenDefn(const QString& filePath)
{
    load(filePath);
}

bool
ScreenDefn::operator==(const ScreenDefn& rhs) const
{
    if (rhs.name != name) return false;
    if (rhs.description != description) return false;
    if (!(rhs.menus == menus)) return false;
    if (!(rhs.buttons == buttons)) return false;
    if (!(rhs.panels == panels)) return false;
    if (!(rhs.shortcuts == shortcuts)) return false;
    return true;
}

bool
ScreenDefn::operator!=(const ScreenDefn& rhs) const
{
    return !(*this == rhs);
}

MenuItem*
ScreenDefn::findMenuItem(int id, MenuItem& top)
{
    if (top.id == id) return &top;

    for (unsigned int i = 0; i < top.items.size(); ++i) {
	MenuItem* item = findMenuItem(id, top.items[i]);
	if (item != NULL) return item;
    }

    return NULL;
}

MenuItem*
ScreenDefn::findMenuItem(int id)
{
    for (unsigned int i = 0; i < menus.size(); ++i) {
	MenuItem* item = findMenuItem(id, menus[i]);
	if (item != NULL) return item;
    }
    return NULL;
}

Button*
ScreenDefn::findToolButton(QButton* widget)
{
    for (unsigned int i = 0; i < buttons.size(); ++i) {
	Button& button = buttons[i];
	if (button.widget == widget)
	    return &button;
    }
    return NULL;
}

Button*
ScreenDefn::findPanelButton(QButton* widget)
{
    for (unsigned int i = 0; i < panels.size(); ++i) {
	Panel& panel = panels[i];
	for (unsigned int j = 0; j < panel.items.size(); ++j) {
	    Button& button = panel.items[j].button;
	    if (button.widget == widget)
		return &button;
	}
    }
    return NULL;
}

Button*
ScreenDefn::findShortcut(QButton* widget)
{
    for (unsigned int i = 0; i < shortcuts.size(); ++i) {
	Button& button = shortcuts[i];
	if (button.widget == widget)
	    return &button;
    }
    return NULL;
}

bool
ScreenDefn::load(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly))
	return error("Failed to open \"" + filePath + "\" for reading");

    QDomDocument doc("QuasarScreen");
    QString errorMsg;
    int errorLine, errorColumn;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorColumn)) {
	QString message = "Failed parsing \"" + filePath + "\" as XML\n"
	    "Error on line " + QString::number(errorLine) + ": " + errorMsg;
	return error(message);
    }

    QDomElement root = doc.documentElement();
    QDomNodeList nodes = root.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString text = e.text();

	if (tag == "name")
	    name = text;
	else if (tag == "description")
	    description = text;
	else if (tag == "menu")
	    menus.push_back(MenuItem(e));
	else if (tag == "toolButton")
	    buttons.push_back(Button(e));
	else if (tag == "panel")
	    panels.push_back(Panel(e));
	else if (tag == "shortcut")
	    shortcuts.push_back(Button(e));
	else
	    qWarning("Unknown tag: " + e.tagName());
    }

    // Validate newly loaded screen but catch errors
    if (!validate())
	qWarning("Validation Failed: " + lastError);

    return true;
}

bool
ScreenDefn::save(const QString& filePath, bool overwrite)
{
    // Must be valid before it can be saved
    if (!validate())
	return false;
    if (!overwrite && QFile::exists(filePath))
	return error("File \"" + filePath + "\" already exists");
    if (filePath.isEmpty())
	return error("Blank screen file path in save");

    QDomDocument doc("QuasarScreen");
    QDomElement root = doc.createElement("QuasarScreen");

    QDomElement nameNode = doc.createElement("name");
    nameNode.appendChild(doc.createTextNode(name));
    root.appendChild(nameNode);

    QDomElement descNode = doc.createElement("description");
    descNode.appendChild(doc.createTextNode(description));
    root.appendChild(descNode);

    unsigned int i;
    for (i = 0; i < menus.size(); ++i) {
	QDomElement e = doc.createElement("menu");
	menus[i].toXML(e);
	root.appendChild(e);
    }
    for (i = 0; i < buttons.size(); ++i) {
	QDomElement e = doc.createElement("toolButton");
	buttons[i].toXML(e);
	root.appendChild(e);
    }
    for (i = 0; i < panels.size(); ++i) {
	QDomElement e = doc.createElement("panel");
	panels[i].toXML(e);
	root.appendChild(e);
    }
    for (i = 0; i < shortcuts.size(); ++i) {
	QDomElement e = doc.createElement("shortcut");
	shortcuts[i].toXML(e);
	root.appendChild(e);
    }

    QString xmlInstruction = "version=\"1.0\" encoding=\"UTF-8\"";
    doc.appendChild(doc.createProcessingInstruction("xml", xmlInstruction));
    doc.appendChild(root);

    QFile file(filePath);
    if (!file.open(IO_WriteOnly))
	return error("Failed to open \"" + filePath + "\" for writing");

    QTextStream stream(&file);
    stream << doc.toString();

    file.close();
    setPermissions(filePath, 0640, "root", "quasar");

    return true;
}

bool
ScreenDefn::validate()
{
    if (name.isEmpty())
	return error("ScreenDefn name is manditory");

    unsigned int i;
    for (i = 0; i < menus.size(); ++i)
	menus[i].validate();
    for (i = 0; i < buttons.size(); ++i)
	buttons[i].validate();
    for (i = 0; i < panels.size(); ++i)
	panels[i].validate();
    for (i = 0; i < shortcuts.size(); ++i)
	shortcuts[i].validate();

    return true;
}

bool
ScreenDefn::error(const QString& error)
{
    qWarning(error);
    lastError = error;
    return false;
}

MenuItem::MenuItem()
    : type(ITEM), id(-1)
{
}

MenuItem::MenuItem(QDomElement e)
    : type(ITEM), id(-1)
{
    fromXML(e);
}

bool
MenuItem::validate()
{
    switch (type) {
    case SEPARATOR:
	break;
    case MENU:
	if (text.isEmpty())
	    return error("Menu text is manditory");
	for (unsigned int i = 0; i < items.size(); ++i)
	    items[i].validate();
	break;
    case ITEM:
	if (text.isEmpty())
	    return error("Menu item text is manditory");
	if (slotType.isEmpty())
	    return error("Menu item slot type is manditory");
	// TODO: validate slotType and slotArgs
	break;
    }

    return true;
}

bool
MenuItem::fromXML(QDomElement root)
{
    QString tag = root.tagName();
    if (tag != "menu" && tag != "menuItem" && tag != "menuSeparator")
	return error("Expecting menu, menuItem, or menuSeparator, got " + tag);

    if (tag == "menu") type = MENU;
    else if (tag == "menuItem") type = ITEM;
    else if (tag == "menuSeparator") type = SEPARATOR;

    QDomNodeList nodes = root.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString content = e.text();

	if (tag == "text" && type == ITEM)
	    text = content;
	else if (tag == "name" && type == MENU)
	    text = content;
	else if (tag == "slotType" && type == ITEM)
	    slotType = content;
	else if (tag == "slotArgs" && type == ITEM)
	    slotArgs = content;
	else if (tag == "menu" && type == MENU)
	    items.push_back(MenuItem(e));
	else if (tag == "menuItem" && type == MENU)
	    items.push_back(MenuItem(e));
	else if (tag == "menuSeparator" && type == MENU)
	    items.push_back(MenuItem(e));
	else
	    qWarning("Unknown " + root.tagName() + " tag: " + tag);
    }

    return true;
}

void
MenuItem::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    if (type == ITEM) {
	QDomElement textNode = doc.createElement("text");
	textNode.appendChild(doc.createTextNode(text));
	e.appendChild(textNode);

	QDomElement typeNode = doc.createElement("slotType");
	typeNode.appendChild(doc.createTextNode(slotType));
	e.appendChild(typeNode);

	QDomElement argsNode = doc.createElement("slotArgs");
	argsNode.appendChild(doc.createTextNode(slotArgs));
	e.appendChild(argsNode);
    } else if (type == MENU) {
	QDomElement nameNode = doc.createElement("name");
	nameNode.appendChild(doc.createTextNode(text));
	e.appendChild(nameNode);

	for (unsigned int i = 0; i < items.size(); ++i) {
	    QDomElement itemNode;
	    switch (items[i].type) {
	    case MENU:
		itemNode = doc.createElement("menu");
		break;
	    case ITEM:
		itemNode = doc.createElement("menuItem");
		break;
	    case SEPARATOR:
		itemNode = doc.createElement("menuSeparator");
		break;
	    }
	    items[i].toXML(itemNode);
	    e.appendChild(itemNode);
	}
    }
}

bool
MenuItem::operator==(const MenuItem& rhs) const
{
    if (rhs.type != type) return false;
    if (rhs.text != text) return false;
    if (rhs.slotType != slotType) return false;
    if (rhs.slotArgs != slotArgs) return false;
    if (!(rhs.items == items)) return false;
    return true;
}

bool
MenuItem::operator!=(const MenuItem& rhs) const
{
    return !(*this == rhs);
}

bool
MenuItem::error(const QString& error)
{
    qWarning(error);
    lastError = error;
    return false;
}

Button::Button()
{
}

Button::Button(QDomElement e)
{
    fromXML(e);
}

bool
Button::validate()
{
    if (text.isEmpty())
	return error("Button text is manditory");
    if (slotType.isEmpty())
	return error("Button slot type is manditory");

    // TODO: validate slotType and slotArgs

    return true;
}

bool
Button::fromXML(QDomElement e)
{
    QString tag = e.tagName();
    if (tag != "toolButton" && tag != "shortcut" && tag != "button")
	return error("Expecting button tag, got " + tag);

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString content = e.text();

	if (tag == "text")
	    text = content;
	else if (tag == "slotType")
	    slotType = content;
	else if (tag == "slotArgs")
	    slotArgs = content;
	else
	    qWarning("Unknown button tag: " + tag);
    }

    return true;
}

void
Button::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    QDomElement textNode = doc.createElement("text");
    textNode.appendChild(doc.createTextNode(text));
    e.appendChild(textNode);

    QDomElement typeNode = doc.createElement("slotType");
    typeNode.appendChild(doc.createTextNode(slotType));
    e.appendChild(typeNode);

    QDomElement argsNode = doc.createElement("slotArgs");
    argsNode.appendChild(doc.createTextNode(slotArgs));
    e.appendChild(argsNode);
}

bool
Button::operator==(const Button& rhs) const
{
    if (rhs.text != text) return false;
    if (rhs.slotType != slotType) return false;
    if (rhs.slotArgs != slotArgs) return false;
    return true;
}

bool
Button::operator!=(const Button& rhs) const
{
    return !(*this == rhs);
}

bool
Button::error(const QString& error)
{
    qWarning(error);
    lastError = error;
    return false;
}

Panel::Panel()
    : width(0), height(0)
{
}

Panel::Panel(QDomElement e)
    : width(0), height(0)
{
    fromXML(e);
}

bool
Panel::validate()
{
    if (text.isEmpty())
	return error("Blank panel text");
    if (width <= 0)
	return error("Invalid panel width");
    if (height <= 0)
	return error("Invalid panel height");
    return true;
}

bool
Panel::fromXML(QDomElement e)
{
    QString tag = e.tagName();
    if (tag != "panel")
	return error("Expecting panel tag, got " + tag);

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString content = e.text();

	if (tag == "text")
	    text = content;
	else if (tag == "image")
	    image = content;
	else if (tag == "width")
	    width = content.toInt();
	else if (tag == "height")
	    height = content.toInt();
	else if (tag == "panelButton")
	    items.push_back(PanelButton(e));
	else if (tag == "panelLine")
	    items.push_back(PanelButton(e));
	else if (tag == "panelReport")
	    reports.push_back(PanelReport(e));
	else
	    qWarning("Unknown panel tag: " + tag);
    }

    return true;
}

void
Panel::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    QDomElement textNode = doc.createElement("text");
    textNode.appendChild(doc.createTextNode(text));
    e.appendChild(textNode);

    if (!image.isEmpty()) {
	QDomElement imageNode = doc.createElement("image");
	imageNode.appendChild(doc.createTextNode(image));
	e.appendChild(imageNode);
    }

    QDomElement widthNode = doc.createElement("width");
    widthNode.appendChild(doc.createTextNode(QString::number(width)));
    e.appendChild(widthNode);

    QDomElement heightNode = doc.createElement("height");
    heightNode.appendChild(doc.createTextNode(QString::number(height)));
    e.appendChild(heightNode);

    for (unsigned int i = 0; i < items.size(); ++i) {
	if (items[i].sides != -1) {
	    QDomElement itemNode = doc.createElement("panelLine");
	    items[i].toXML(itemNode);
	    e.appendChild(itemNode);
	} else {
	    QDomElement itemNode = doc.createElement("panelButton");
	    items[i].toXML(itemNode);
	    e.appendChild(itemNode);
	}
    }

    for (unsigned int i = 0; i < reports.size(); ++i) {
	QDomElement itemNode = doc.createElement("panelReport");
	reports[i].toXML(itemNode);
	e.appendChild(itemNode);
    }
}

bool
Panel::operator==(const Panel& rhs) const
{
    if (rhs.text != text) return false;
    if (rhs.image != image) return false;
    if (rhs.width != width) return false;
    if (rhs.height != height) return false;
    if (!(rhs.items == items)) return false;
    if (!(rhs.reports == reports)) return false;
    return true;
}

bool
Panel::operator!=(const Panel& rhs) const
{
    return !(*this == rhs);
}

bool
Panel::error(const QString& error)
{
    qWarning(error);
    lastError = error;
    return false;
}

PanelButton::PanelButton()
    : row(0), column(0), sides(-1)
{
}

PanelButton::PanelButton(QDomElement e)
    : row(0), column(0), sides(-1)
{
    fromXML(e);
}

bool
PanelButton::validate()
{
    if (row < 0 || row > 16)
	return error("Invalid row number");
    if (column < 0 || column > 16)
	return error("Invalid column number");
    if (sides == -1)
	return button.validate();
    return true;
}

bool
PanelButton::fromXML(QDomElement e)
{
    QString type = e.tagName();
    if (type != "panelButton" && type != "panelLine")
	return error("Expecting panelButton or panelLine tag, got " + type);

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString content = e.text();

	if (tag == "row")
	    row = content.toInt();
	else if (tag == "column")
	    column = content.toInt();
	else if (tag == "sides" && type == "panelLine")
	    sides = content.toInt();
	else if (tag == "button" && type == "panelButton")
	    button = Button(e);
	else
	    qWarning("Unknown panelButton tag: " + tag);
    }

    return true;
}

void
PanelButton::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    QDomElement rowNode = doc.createElement("row");
    rowNode.appendChild(doc.createTextNode(QString::number(row)));
    e.appendChild(rowNode);

    QDomElement columnNode = doc.createElement("column");
    columnNode.appendChild(doc.createTextNode(QString::number(column)));
    e.appendChild(columnNode);

    if (sides != -1) {
	QDomElement sidesNode = doc.createElement("sides");
	sidesNode.appendChild(doc.createTextNode(QString::number(sides)));
	e.appendChild(sidesNode);
    } else {
	QDomElement buttonNode = doc.createElement("button");
	button.toXML(buttonNode);
	e.appendChild(buttonNode);
    }
}

bool
PanelButton::operator==(const PanelButton& rhs) const
{
    if (rhs.row != row) return false;
    if (rhs.column != column) return false;
    if (rhs.sides != sides) return false;
    if (rhs.button != button) return false;
    return true;
}

bool
PanelButton::operator!=(const PanelButton& rhs) const
{
    return !(*this == rhs);
}

bool
PanelButton::error(const QString& error)
{
    qWarning(error);
    lastError = error;
    return false;
}

PanelReport::PanelReport()
    : row(0), column(0)
{
}

PanelReport::PanelReport(QDomElement e)
    : row(0), column(0)
{
    fromXML(e);
}

bool
PanelReport::validate()
{
    if (row < 0 || row > 16)
	return error("Invalid row number");
    if (column < 0 || column > 16)
	return error("Invalid column number");
    if (text.isEmpty())
	return error("Blank report text");
    if (slotType.isEmpty())
	return error("Report slot type is manditory");

    // TODO: validate slotType and slotArgs

    return true;
}

bool
PanelReport::fromXML(QDomElement e)
{
    QString type = e.tagName();
    if (type != "panelReport")
	return error("Expecting panelReport tag, got " + type);

    QDomNodeList nodes = e.childNodes();
    for (uint i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;
	QString tag = e.tagName();
	QString content = e.text();

	if (tag == "row")
	    row = content.toInt();
	else if (tag == "column")
	    column = content.toInt();
	else if (tag == "text")
	    text = content;
	else if (tag == "slotType")
	    slotType = content;
	else if (tag == "slotArgs")
	    slotArgs = content;
	else
	    qWarning("Unknown panelReport tag: " + tag);
    }

    return true;
}

void
PanelReport::toXML(QDomElement& e) const
{
    QDomDocument doc = e.ownerDocument();

    QDomElement rowNode = doc.createElement("row");
    rowNode.appendChild(doc.createTextNode(QString::number(row)));
    e.appendChild(rowNode);

    QDomElement columnNode = doc.createElement("column");
    columnNode.appendChild(doc.createTextNode(QString::number(column)));
    e.appendChild(columnNode);

    QDomElement textNode = doc.createElement("text");
    textNode.appendChild(doc.createTextNode(text));
    e.appendChild(textNode);

    QDomElement typeNode = doc.createElement("slotType");
    typeNode.appendChild(doc.createTextNode(slotType));
    e.appendChild(typeNode);

    QDomElement argsNode = doc.createElement("slotArgs");
    argsNode.appendChild(doc.createTextNode(slotArgs));
    e.appendChild(argsNode);
}

bool
PanelReport::operator==(const PanelReport& rhs) const
{
    if (rhs.row != row) return false;
    if (rhs.column != column) return false;
    if (rhs.text != text) return false;
    if (rhs.slotType != slotType) return false;
    if (rhs.slotArgs != slotArgs) return false;
    return true;
}

bool
PanelReport::operator!=(const PanelReport& rhs) const
{
    return !(*this == rhs);
}

bool
PanelReport::error(const QString& error)
{
    qWarning(error);
    lastError = error;
    return false;
}
