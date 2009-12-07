// $Id: screen_defn.h,v 1.11 2004/02/03 00:13:26 arandell Exp $
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

#ifndef SCREEN_DEFN_H
#define SCREEN_DEFN_H

#include <qstring.h>
#include <qstringlist.h>
#include <qvaluevector.h>
#include <qdom.h>

class QPopupMenu;
class QButton;
class QFrame;
class ReportBox;

class MenuItem {
public:
    MenuItem();
    MenuItem(QDomElement e);

    // Types
    enum { MENU, ITEM, SEPARATOR };

    // Properties
    int type;				// Type of menu item
    QString text;			// Text for menu or item
    QValueVector<MenuItem> items;	// Sub-items for menus
    QString slotType;			// Slot for items
    QString slotArgs;			// Arguments for items
    int id;				// Internal run-time id for items
    QPopupMenu* widget;			// Widget for menus
    QString lastError;			// Last error message

    // Validate
    bool validate();

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    void toXML(QDomElement& e) const;

    // Operations
    bool operator==(const MenuItem& rhs) const;
    bool operator!=(const MenuItem& rhs) const;

private:
    bool error(const QString& message);
};

class Button {
public:
    Button();
    Button(QDomElement e);

    // Properties
    QString text;		// Text to show on button
    QString slotType;		// Type of slot
    QString slotArgs;		// Args depending on slot type
    QButton* widget;		// Internal widget
    QString lastError;		// Last error from validate or toXML

    // Validate
    bool validate();

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    void toXML(QDomElement& e) const;

    // Operations
    bool operator==(const Button& rhs) const;
    bool operator!=(const Button& rhs) const;

private:
    bool error(const QString& message);
};

class PanelButton {
public:
    PanelButton();
    PanelButton(QDomElement e);

    // Properties
    int row;		// Row number (starting with 1)
    int column;		// Column number (starting with 1)
    int sides;		// If not -1 then this is a line
    Button button;	// Button to show
    QString lastError;	// Last error from validate or toXML

    // Validate
    bool validate();

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    void toXML(QDomElement& e) const;

    // Operations
    bool operator==(const PanelButton& rhs) const;
    bool operator!=(const PanelButton& rhs) const;

private:
    bool error(const QString& message);
};

class PanelReport {
public:
    PanelReport();
    PanelReport(QDomElement e);

    // Properties
    int row;		// Row number (starting with 1)
    int column;		// Column number (starting with 1)
    QString text;	// Text to show
    QString slotType;	// Type of slot
    QString slotArgs;	// Args depending on slot type
    QString lastError;	// Last error from validate or toXML

    // Validate
    bool validate();

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    void toXML(QDomElement& e) const;

    // Operations
    bool operator==(const PanelReport& rhs) const;
    bool operator!=(const PanelReport& rhs) const;

private:
    bool error(const QString& message);
};

class Panel {
public:
    Panel();
    Panel(QDomElement e);

    // Properties
    QString text;			// Text to show on button
    QString image;			// Image to show on button
    int width;				// Width of panel in cells
    int height;				// Height of panel in cells
    QValueVector<PanelButton> items;	// Items in the panel
    QValueVector<PanelReport> reports;	// Reports in the panel
    QFrame* widget;			// Frame for panel
    ReportBox* repWidget;		// Reports
    QString lastError;			// Last error from validate or toXML

    // Validate
    bool validate();

    // Convert from/to XML element
    bool fromXML(QDomElement e);
    void toXML(QDomElement& e) const;

    // Operations
    bool operator==(const Panel& rhs) const;
    bool operator!=(const Panel& rhs) const;

private:
    bool error(const QString& message);
};

class ScreenDefn {
public:
    // Constructors and Destructor
    ScreenDefn();
    ScreenDefn(const QString& filePath);

    // Public information
    QString name;			// Name of screen
    QString description;		// Longer description of screen
    QValueVector<MenuItem> menus;	// Menus
    QValueVector<Button> buttons;	// Toolbar buttons
    QValueVector<Panel> panels;		// Panels
    QValueVector<Button> shortcuts;	// Shortcut buttons
    QString lastError;			// Last error message

    // Find infomation
    MenuItem* findMenuItem(int id);
    Button* findToolButton(QButton* widget);
    Button* findPanelButton(QButton* widget);
    Button* findShortcut(QButton* widget);

    // Load/save screen in XML format
    bool load(const QString& filePath);
    bool save(const QString& filePath, bool overwrite=false);

    // Validate all aspects of screen
    bool validate();

    // Operations
    bool operator==(const ScreenDefn& rhs) const;
    bool operator!=(const ScreenDefn& rhs) const;

private:
    bool error(const QString& message);
    MenuItem* findMenuItem(int id, MenuItem& top);
};

#endif // SCREEN_DEFN_H
