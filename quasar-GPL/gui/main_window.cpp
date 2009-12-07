// $Id: main_window.cpp,v 1.143 2005/03/01 20:08:24 bpepers Exp $
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

#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "quasar_version.h"
#include "company.h"
#include "user_config.h"
#include "store.h"
#include "station.h"
#include "employee.h"
#include "images.h"
#include "open_company.h"
#include "help_window.h"
#include "arrow_line.h"
#include "screen_decl.h"
#include "param_type_factory.h"
#include "lookup_param_type.h"
#include "report_box.h"
#include "report_viewer.h"
#include "base64.h"
#include "tcl_interp.h"
#include "money_valcon.h"
#include "percent_valcon.h"
#include "query_data_source.h"
#include "data_source_factory.h"
#include "param_dialog.h"

#include <qapplication.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qpopupmenu.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qwidgetstack.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidgetlist.h>
#include <qmessagebox.h>

#define USE_COMPAT_CONST
#include <tcl.h>

static Tcl_Obj*
getObject(const QVariant& v)
{
    Tcl_Obj* value;
    QString text;

    switch (v.type()) {
    case QVariant::Int:
    case QVariant::UInt:
	value = Tcl_NewLongObj(v.toInt());
	break;
    case QVariant::Bool:
	value = Tcl_NewBooleanObj(v.toBool());
	break;
    case QVariant::Double:
	value = Tcl_NewDoubleObj(v.toDouble());
	break;
    case QVariant::Date:
	text = v.toDate().toString(Qt::ISODate);
	value = Tcl_NewStringObj(text.utf8(), text.utf8().length());
	break;
    default:
	text = v.toString();
	value = Tcl_NewStringObj(text.utf8(), text.utf8().length());
	break;
    }

    Tcl_IncrRefCount(value);
    return value;
}

static int
selectCmd(ClientData data, Tcl_Interp* interp, int argc, char* argv[])
{
    Connection* connection = (Connection*)data;

    if (argc != 3) {
	Tcl_SetResult(interp, "wrong # args: should be "
		      "\"select query cmd\"", TCL_STATIC);
	return TCL_ERROR;
    }

    Stmt stmt(connection);
    stmt.setCommand(argv[1]);

    if (!stmt.execute()) {
	qWarning("Error: " + stmt.lastError());
	Tcl_SetResult(interp, "query failed", TCL_STATIC);
	return TCL_ERROR;
    }

    while (stmt.next()) {
	for (int i = 1; i <= stmt.columnCount(); ++i) {
	    char number[10];
	    sprintf(number, "%d", i);
	    QString text = stmt.getString(i);

	    Tcl_Obj* array = getObject("column");
	    Tcl_Obj* element = getObject(number);
	    Tcl_Obj* value = getObject(text);
	    Tcl_ObjSetVar2(interp, array, element, value, 0);
	}
	Tcl_Eval(interp, argv[2]);
    }

    return TCL_OK;
}

static int
oconvCmd(ClientData, Tcl_Interp* interp, int argc, char* argv[])
{
    if (argc != 3) {
	Tcl_SetResult(interp, "wrong # args: should be "
		      "\"oconv type value\"", TCL_STATIC);
	return TCL_ERROR;
    }

    QString type = argv[1];
    QString value = argv[2];

    QString result;
    if (value.isEmpty())
	result = "";
    else if (type == "number")
	result = NumberValcon().format(value);
    else if (type == "money")
	result = MoneyValcon().format(value);
    else if (type == "percent")
	result = PercentValcon().format(value);
    else
	result = "unknown type: " + type;

    Tcl_Obj* object = getObject(result);
    Tcl_SetObjResult(interp, object);

    return TCL_OK;
}

void
quasarTclInit(ReportInterp* i, void* arg)
{
    TclInterp* interp = (TclInterp*)i;
    Connection* connection = (Connection*)arg;

    Tcl_CreateCommand(interp->interp, "select", &selectCmd, connection, NULL);
    Tcl_CreateCommand(interp->interp, "oconv", &oconvCmd, connection, NULL);
}

MainWindow::MainWindow(QuasarClient* quasar, ScreenDefn& screen)
    : QMainWindow(0, "MainWindow", WType_TopLevel | WDestructiveClose)
{
    _quasar = quasar;
    _db = quasar->db();
    _screen = screen;

#ifndef _WIN32
    setIcon(QPixmap(window_icon_xpm));
#endif

    // Top level frame for contents
    QFrame* top = new QFrame(this);
    setCentralWidget(top);

    // Sections in body
    QHBox* toolbar = new QHBox(top);
    QVBox* tabs = new QVBox(top);
    QWidgetStack* panelStack = new QWidgetStack(top);
    int nextPanel = 0;

    // Shortcuts are a vertical set of buttons
    QFrame* shortcut = new QFrame(top);
    QGridLayout* shortcutGrid = new QGridLayout(shortcut);
    shortcutGrid->setSpacing(1);
    shortcutGrid->setMargin(3);
    shortcutGrid->setRowStretch(0, 1);
    int nextShortcut = 1;

    // Button group for panel tabs
    QButtonGroup* group = new QButtonGroup(this);
    group->hide();
    group->setExclusive(true);
    connect(group, SIGNAL(clicked(int)), panelStack, SLOT(raiseWidget(int)));

    // Create menus
    unsigned int i;
    for (i = 0; i < _screen.menus.size(); ++i) {
	MenuItem& menu = _screen.menus[i];
	buildMenu(menu);
	menuBar()->insertItem(menu.text, menu.widget);
    }

    // Create toolbar buttons
    for (i = 0; i < _screen.buttons.size(); ++i) {
	Button& button = _screen.buttons[i];
	button.widget = new QPushButton(button.text, toolbar);
	connect(button.widget, SIGNAL(clicked()), SLOT(slotToolButton()));
    }

    // Create panels
    for (i = 0; i < _screen.panels.size(); ++i) {
	Panel& panel = _screen.panels[i];
	QToolButton* widget = new QToolButton(tabs);
	widget->setToggleButton(true);
	widget->setTextLabel(panel.text);
	widget->setUsesBigPixmap(true);
	widget->setUsesTextLabel(true);
	group->insert(widget);

	if (panel.image == "cardFile")
	    widget->setPixmap(QPixmap(card_file_xpm));
	else if (panel.image == "generalLedger")
	    widget->setPixmap(QPixmap(general_ledger_xpm));
	else if (panel.image == "chequeBook")
	    widget->setPixmap(QPixmap(cheque_book_xpm));
	else if (panel.image == "inventory")
	    widget->setPixmap(QPixmap(inventory_xpm));
	else if (panel.image == "sales")
	    widget->setPixmap(QPixmap(sales_xpm));
	else if (panel.image == "purchases")
	    widget->setPixmap(QPixmap(purchases_xpm));
	else {
	    QByteArray data;
	    if (!base64Decode(panel.image, data)) {
		qWarning("Error: failed decoding image data");
	    } else {
		QPixmap pixmap;
		if (!pixmap.loadFromData(data))
		    qWarning("Error: failed loading image data");
		else
		    widget->setPixmap(pixmap);
	    }
	}

	panel.widget = new QFrame(panelStack);
	panelStack->addWidget(panel.widget, nextPanel++);

	if (nextPanel == 1) {
	    widget->setOn(true);
	    panelStack->raiseWidget(panel.widget);
	}

	QGridLayout* grid = new QGridLayout(panel.widget);
	grid->setMargin(3);
	grid->setSpacing(0);
	grid->setRowStretch(0, 1);
	grid->setRowStretch(panel.height + 1, 1);
	grid->setColStretch(0, 1);
	grid->setColStretch(panel.width + 1, 1);

	panel.repWidget = new ReportBox(panel.widget);
	connect(panel.repWidget, SIGNAL(clicked(QString,QString)),
		SLOT(slotFunction(QString,QString)));
	grid->addMultiCellWidget(panel.repWidget, panel.height + 2,
				 panel.height + 2, 0, panel.width + 1);

	// Add panel buttons
	for (unsigned int j = 0; j < panel.items.size(); ++j) {
	    PanelButton& pbutton = panel.items[j];
	    if (pbutton.sides != -1) {
		ArrowLine* widget = new ArrowLine(pbutton.sides, panel.widget);
		grid->addWidget(widget, pbutton.row, pbutton.column);
	    } else {
		Button& button = pbutton.button;
		button.widget = new QPushButton(button.text, panel.widget);
		grid->addWidget(button.widget, pbutton.row, pbutton.column);
		connect(button.widget, SIGNAL(clicked()), 
			SLOT(slotPanelButton()));
	    }
	}

	// Add panel reports
	for (unsigned int j = 0; j < panel.reports.size(); ++j) {
	    PanelReport& report = panel.reports[j];
	    int row = report.row - 1;
	    int column = report.column - 1;
	    panel.repWidget->setText(row, column, report.text);
	    panel.repWidget->setType(row, column, report.slotType);
	    panel.repWidget->setArgs(row, column, report.slotArgs);
	}
    }

    // Create shortcut buttons
    for (i = 0; i < _screen.shortcuts.size(); ++i) {
	Button& button = _screen.shortcuts[i];
	button.widget = new QPushButton(button.text, shortcut);
	shortcutGrid->addWidget(button.widget, nextShortcut++, 0);
	connect(button.widget, SIGNAL(clicked()), SLOT(slotShortcut()));
    }

    // Layout at the top level
    QGridLayout* grid = new QGridLayout(top);
    grid->setRowStretch(1, 1);
    grid->setColStretch(1, 1);
    grid->addMultiCellWidget(toolbar, 0, 0, 0, 2);
    grid->addWidget(tabs, 1, 0);
    grid->addWidget(panelStack, 1, 1);
    grid->addWidget(shortcut, 1, 2);

    // Current store and employee in status bar
    _store = new QLabel(" ", statusBar());
    _station = new QLabel(" ", statusBar());
    _employee = new QLabel(" ", statusBar());

    // Set font to 3/4 or normal size
    QFont font = _store->font();
    if (font.pointSize() == -1)
	font.setPixelSize(font.pixelSize() * 3 / 4);
    else
	font.setPointSize(font.pointSize() * 3 / 4);
    QFontMetrics metrics(font);

    _store->setFont(font);
    _store->setMinimumWidth(metrics.maxWidth() * 14);
    _station->setFont(font);
    _station->setMinimumWidth(metrics.maxWidth() * 14);
    _employee->setFont(font);
    _employee->setMinimumWidth(metrics.maxWidth() * 14);

    // Add to status bar
    statusBar()->setSizeGripEnabled(false);
    statusBar()->addWidget(_store, 1, true);
    statusBar()->addWidget(_station, 1, true);
    statusBar()->addWidget(_employee, 1, true);

    Company company;
    _db->lookup(company);
    setCaption("Quasar: " + company.name());

    // Get default store
    Id store_id = _quasar->defaultStore();
    Store store;
    if (store_id == INVALID_ID || !_db->lookup(store_id, store)) {
	store_id = _quasar->user().defaultStore();
	if (store_id == INVALID_ID)
	    store_id = company.defaultStore();
	_quasar->setDefaultStore(store_id);
    }
    _db->lookup(store_id, store);
    _store->setText(store.name());

    // Get default station
    Station station;
    _db->lookup(_quasar->defaultStation(), station);
    _station->setText(station.name());

    // Get default employee
    Employee employee;
    _db->lookup(_quasar->defaultEmployee(), employee);
    _employee->setText(employee.name());

    // Set param default value mappings
    ReportParamType::setInternal("current_company", company.id().toString());
    ReportParamType::setInternal("current_store", store.id().toString());
    ReportParamType::setInternal("current_station", station.id().toString());
    ReportParamType::setInternal("current_employee", employee.id().toString());

    // Register lookup param type
    if (!ParamTypeFactory::typeExists("lookup"))
	ParamTypeFactory::registerType("lookup", createLookupParamType);
    LookupParamType::quasar = _quasar;

    // Register query data source
    if (!DataSourceFactory::typeExists("query"))
	DataSourceFactory::registerType("query", createQueryDataSource);
    QueryDataSource::connection = _quasar->connection();

    // Register callback on Tcl interp init
    ReportInterp::addCallback("tcl", quasarTclInit, _quasar->connection());
}

MainWindow::~MainWindow()
{
    slotCloseAll();
    delete _quasar;
}

void
MainWindow::buildMenu(MenuItem& menu)
{
    menu.widget = new QPopupMenu(this);
    connect(menu.widget, SIGNAL(activated(int)), SLOT(slotMenuItem(int)));

    for (unsigned int j = 0; j < menu.items.size(); ++j) {
	MenuItem& item = menu.items[j];
	switch (item.type) {
	case MenuItem::MENU:
	    buildMenu(item);
	    item.id = menu.widget->insertItem(item.text, item.widget);
	    break;
	case MenuItem::ITEM:
	    item.id = menu.widget->insertItem(item.text);
	    break;
	case MenuItem::SEPARATOR:
	    menu.widget->insertSeparator();
	    break;
	}
    }
}

void
MainWindow::slotMenuItem(int id)
{
    MenuItem* item = _screen.findMenuItem(id);
    if (item != NULL)
	slotFunction(item->slotType, item->slotArgs);
}

void
MainWindow::slotToolButton()
{
    QButton* widget = (QButton*)sender();
    Button* button = _screen.findToolButton(widget);
    if (button != NULL)
	slotFunction(button->slotType, button->slotArgs);
}

void
MainWindow::slotPanelButton()
{
    QButton* widget = (QButton*)sender();
    Button* button = _screen.findPanelButton(widget);
    if (button != NULL)
	slotFunction(button->slotType, button->slotArgs);
}

void
MainWindow::slotShortcut()
{
    QButton* widget = (QButton*)sender();
    Button* button = _screen.findShortcut(widget);
    if (button != NULL)
	slotFunction(button->slotType, button->slotArgs);
}

void
MainWindow::slotFunction(QString type, QString args)
{
    if (type.isEmpty()) return;

    if (type == "exit")
	slotExit();
    else if (type == "screen")
	ScreenDecl::createScreen(args, this);
    else if (type == "report")
	slotReport(args);
    else if (type == "changeCompany")
	slotChangeCompany();
    else if (type == "help")
	slotHelp();
    else if (type == "aboutQt")
	slotAboutQt();
    else if (type == "aboutQuasar")
	slotAboutQuasar();
    else if (type == "program")
	system(args);
    else
	qWarning("Unknown function: " + type + "(" + args + ")");

}

void
MainWindow::slotReport(const QString& fileName)
{
    QString filePath;
    if (!_quasar->resourceFetch("reports", fileName, filePath)) {
	QApplication::restoreOverrideCursor();
	QString message = tr("Error fetching report '%1'").arg(fileName);
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    ReportDefn report;
    if (!report.load(filePath)) {
	QApplication::restoreOverrideCursor();
	QString message = tr("Report '%1' is invalid").arg(fileName);
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QApplication::restoreOverrideCursor();

    ParamMap params;
    if (!report.getParams(this, params))
	return;

    QApplication::setOverrideCursor(waitCursor);
    qApp->processEvents();

    ReportOutput* output = new ReportOutput();
    if (!report.generate(params, *output)) {
	QApplication::restoreOverrideCursor();
	delete output;
	QString message = tr("Report '%1' failed").arg(fileName);
	qApp->beep();
	QMessageBox::critical(this, tr("Error"), message);
	return;
    }

    QApplication::restoreOverrideCursor();
    ReportViewer* view = new ReportViewer(report, params, output);
    view->show();
}

void
MainWindow::setDefaultStore(Id store_id)
{
    Store store;
    _db->lookup(store_id, store);
    _store->setText(store.name());
}

Id
MainWindow::defaultStore()
{
    return _quasar->defaultStore();
}

bool
MainWindow::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease) {
	QKeyEvent* ke = (QKeyEvent*)e;
	if (ke->key() == Key_Return || ke->key() == Key_Enter) {
	    bool replace = false;
	    if (o->inherits("QWidget")) {
		QWidget* w = (QWidget*)o;
		if (w->inherits("QLineEdit")) replace = true;
		if (w->topLevelWidget()->isDialog()) replace = false;
	    }

	    if (replace) {
		QKeyEvent rep(ke->type(), Key_Tab, '\t', ke->state(),
			      QString::null, ke->isAutoRepeat(), ke->count());
		*ke = rep;
	    }
	}
    }
    return QMainWindow::eventFilter(o, e);
}

void
MainWindow::slotWindowShow()
{
    _window->clear();
    int cascadeId = _window->insertItem(tr("&Cascade"), this,
					SLOT(slotCascade()));
    int tileId = _window->insertItem(tr("&Tile"), this, SLOT(slotTile()));
    _window->insertItem(tr("&Close All"), this, SLOT(slotCloseAll()));

    _window->setItemEnabled(cascadeId, false);
    _window->setItemEnabled(tileId, false);
}

void
MainWindow::slotCascade()
{
}

void
MainWindow::slotTile()
{
}

void
MainWindow::slotCloseAll()
{
    QWidgetList* list = QApplication::topLevelWidgets();
    bool did_close = TRUE;
    QWidget* w = list->first();
    while (did_close && w) {
	if (w->className() != className()&& !w->testWState(WState_ForceHide)) {
	    did_close = w->close();
	    delete list;
	    list = QApplication::topLevelWidgets();
	    w = list->first();
	} else {
	    w = list->next();
	}
    }
    delete list;
}

void
MainWindow::showHelp(const QString& source)
{
    QString home = QuasarClient::helpDir();
    HelpWindow* help = new HelpWindow(source, home);
    help->show();
}

void
MainWindow::slotChangeCompany()
{
    UserConfig config;
    config.load();

    OpenCompany* screen = new OpenCompany(this);
    screen->setInfo(config.defaultServer, config.defaultCompany,
		    config.defaultUserName, config.defaultPassword);
    screen->show();
}

void
MainWindow::slotExit()
{
    slotCloseAll();
    qApp->closeAllWindows();
    qApp->quit();
}

QWidget*
MainWindow::findWindow(const QString& name)
{
    QWidget* found = NULL;
    QWidgetList* list = QApplication::topLevelWidgets();
    QWidgetListIt it(*list);
    QWidget* w;
    while ((w = it.current()) != 0) {
	++it;
	if (w->name() == name) found = w;
    }
    delete list;

    if (found) {
	// Force to come to top and get focus (would be nice to use the
	// setActiveWindow() and raise() here but it doesn't work at
	// least with KDE 1.2)
	found->hide();
	found->show();
    }

    return found;
}

void
MainWindow::slotHelp()
{
    showHelp("index.html");
}

void
MainWindow::slotAboutQt()
{
    QMessageBox::aboutQt(0, tr("Quasar Accounting"));
}

void
MainWindow::slotAboutQuasar()
{
    QDialog* dialog = new QDialog(this, "AboutDialog", true);
    dialog->setCaption("Quasar Accounting");

    QLabel* image = new QLabel(dialog);
    image->setPixmap(login_image);

    QString versionText = "Version " + QUASAR_VERSION_S;
    QString dbVersionText = "Database " + QString(QUASAR_DB_VERSION);
    QString copyrightText = "Copyright (C) 1998-2005 Linux Canada Inc.  "
	"All rights reserved.";

    QLabel* version = new QLabel(versionText, dialog);
    QLabel* dbVersion = new QLabel(dbVersionText, dialog);
    QLabel* copyright = new QLabel(copyrightText, dialog);

    QPushButton* ok = new QPushButton(tr("Ok"), dialog);
    connect(ok, SIGNAL(clicked()), dialog, SLOT(accept()));
    ok->setDefault(true);

    QGridLayout* grid = new QGridLayout(dialog);
    grid->setMargin(6);
    grid->setSpacing(10);
    grid->addWidget(image, 0, 0, AlignCenter);
    grid->addWidget(version, 1, 0, AlignCenter);
    grid->addWidget(dbVersion, 2, 0, AlignCenter);
    grid->addWidget(copyright, 3, 0, AlignCenter);
    grid->addWidget(ok, 4, 0, AlignCenter);

    dialog->exec();
    delete dialog;
}
