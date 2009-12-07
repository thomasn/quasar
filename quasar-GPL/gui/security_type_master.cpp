// $Id: security_type_master.cpp,v 1.10 2004/12/06 18:40:04 bpepers Exp $
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

#include "security_type_master.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "security_rule_dialog.h"
#include "line_edit.h"
#include "list_view_item.h"
#include "combo_box.h"

#include <qapplication.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qmessagebox.h>

SecurityTypeMaster::SecurityTypeMaster(MainWindow* main, Id type_id)
    : DataWindow(main, "SecurityTypeMaster", type_id)
{
    _helpSource = "security_type_master.html";

    // Create widgets
    QLabel* nameLabel = new QLabel(tr("&Name:"), _frame);
    _name = new LineEdit(30, _frame);
    nameLabel->setBuddy(_name);

    _rules = new ListView(_frame);
    _rules->addTextColumn(tr("Screen"), 30);
    _rules->addCheckColumn(tr("View?"));
    _rules->addCheckColumn(tr("Create?"));
    _rules->addCheckColumn(tr("Update?"));
    _rules->addCheckColumn(tr("Delete?"));
    _rules->setAllColumnsShowFocus(true);
    _rules->setSorting(-1);

    QFrame* buttons = new QFrame(_frame);
    QPushButton* add = new QPushButton(tr("Add"), buttons);
    QPushButton* edit = new QPushButton(tr("Edit"), buttons);
    QPushButton* del = new QPushButton(tr("Delete"), buttons);
    QPushButton* up = new QPushButton(tr("Up"), buttons);
    QPushButton* down = new QPushButton(tr("Down"), buttons);

    connect(add, SIGNAL(clicked()), SLOT(slotAddRule()));
    connect(edit, SIGNAL(clicked()), SLOT(slotEditRule()));
    connect(del, SIGNAL(clicked()), SLOT(slotDeleteRule()));
    connect(up, SIGNAL(clicked()), SLOT(slotUp()));
    connect(down, SIGNAL(clicked()), SLOT(slotDown()));
    connect(_rules, SIGNAL(doubleClicked(QListViewItem*)),
	    SLOT(slotEditRule()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(3, 1);
    buttonGrid->addWidget(add, 0, 0);
    buttonGrid->addWidget(edit, 0, 1);
    buttonGrid->addWidget(del, 0, 2);
    buttonGrid->addWidget(up, 0, 4);
    buttonGrid->addWidget(down, 0, 5);

    QGridLayout* grid = new QGridLayout(_frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setRowStretch(1, 1);
    grid->setColStretch(2, 1);
    grid->addWidget(nameLabel, 0, 0);
    grid->addWidget(_name, 0, 1, AlignLeft | AlignVCenter);
    grid->addMultiCellWidget(_rules, 1, 1, 0, 2);
    grid->addMultiCellWidget(buttons, 2, 2, 0, 2);

    setCaption(tr("Security Type Master"));
    finalize();
}

SecurityTypeMaster::~SecurityTypeMaster()
{
}

void
SecurityTypeMaster::oldItem()
{
    _quasar->db()->lookup(_id, _orig);

    _curr = _orig;
    _firstField = _name;
}

void
SecurityTypeMaster::newItem()
{
    SecurityType blank;
    _orig = blank;

    _curr = _orig;
    _firstField = _name;
}

void
SecurityTypeMaster::cloneFrom(Id id)
{
    setCaption(cloneName());
    _quasar->db()->lookup(id, _curr);
    _curr.setId(INVALID_ID);
    _quasar->db()->setActive(_curr, true);
    dataToWidget();
    _name->selectAll();
}

bool
SecurityTypeMaster::fileItem()
{
    if (_orig.id() == INVALID_ID) {
	if (!_quasar->db()->create(_curr)) return false;
    } else {
	if (!_quasar->db()->update(_orig, _curr)) return false;
    }

    _orig = _curr;
    _id = _curr.id();

    return true;
}

bool
SecurityTypeMaster::deleteItem()
{
    return _quasar->db()->remove(_curr);
}

void
SecurityTypeMaster::restoreItem()
{
    _curr = _orig;
}

void
SecurityTypeMaster::cloneItem()
{
    SecurityTypeMaster* clone = new SecurityTypeMaster(_main);
    clone->cloneFrom(_curr.id());
    clone->show();
}

bool
SecurityTypeMaster::isChanged()
{
    return _curr != _orig;
}

// Set the widgets from the data object.
void 
SecurityTypeMaster::dataToWidget()
{
    _name->setText(_curr.name());
    _inactive->setChecked(!_curr.isActive());

    _rules->clear();
    ListViewItem* item = NULL;
    for (unsigned int i = 0; i < _curr.rules().size(); ++i) {
	const SecurityRule& rule = _curr.rules()[i];
	item = new ListViewItem(_rules, item);
	item->setValue(0, rule.screen);
	item->setValue(1, rule.allowView);
	item->setValue(2, rule.allowCreate);
	item->setValue(3, rule.allowUpdate);
	item->setValue(4, rule.allowDelete);
    }

    _rules->setCurrentItem(_rules->firstChild());
    _rules->setSelected(_rules->firstChild(), true);
}

// Set the data object from the widgets.
void
SecurityTypeMaster::widgetToData()
{
    _curr.setName(_name->text());
    _quasar->db()->setActive(_curr, !_inactive->isChecked());

    _curr.rules().clear();
    ListViewItem* item = _rules->firstChild();
    while (item != NULL) {
	SecurityRule rule;
	rule.screen = item->text(0);
	rule.allowView = item->value(1).toBool();
	rule.allowCreate = item->value(2).toBool();
	rule.allowUpdate = item->value(3).toBool();
	rule.allowDelete = item->value(4).toBool();
	_curr.rules().push_back(rule);
	item = item->nextSibling();
    }
}

void
SecurityTypeMaster::slotAddRule()
{
    SecurityRuleDialog* dialog = new SecurityRuleDialog(this);
    int result = dialog->exec();
    SecurityRule rule = dialog->getRule();
    delete dialog;

    if (result == QDialog::Accepted) {
	ListViewItem* current = _rules->currentItem();
	ListViewItem* item = new ListViewItem(_rules, current);
	item->setValue(0, rule.screen);
	item->setValue(1, rule.allowView);
	item->setValue(2, rule.allowCreate);
	item->setValue(3, rule.allowUpdate);
	item->setValue(4, rule.allowDelete);

	_rules->setCurrentItem(item);
	_rules->setSelected(item, true);
    }
}

void
SecurityTypeMaster::slotEditRule()
{
    ListViewItem* current = _rules->currentItem();
    if (current == NULL) return;

    SecurityRule rule;
    rule.screen = current->text(0);
    rule.allowView = current->value(1).toBool();
    rule.allowCreate = current->value(2).toBool();
    rule.allowUpdate = current->value(3).toBool();
    rule.allowDelete = current->value(4).toBool();

    SecurityRuleDialog* dialog = new SecurityRuleDialog(this);
    dialog->setRule(rule);
    int result = dialog->exec();
    rule = dialog->getRule();
    delete dialog;

    if (result == QDialog::Accepted) {
	current->setValue(0, rule.screen);
	current->setValue(1, rule.allowView);
	current->setValue(2, rule.allowCreate);
	current->setValue(3, rule.allowUpdate);
	current->setValue(4, rule.allowDelete);
    }
}

void
SecurityTypeMaster::slotDeleteRule()
{
    ListViewItem* current = _rules->currentItem();
    if (current == NULL) return;
    ListViewItem* next = current->itemBelow();
    if (next == NULL) next = current->itemAbove();

    delete current;

    _rules->setCurrentItem(next);
    _rules->setSelected(next, true);
}

void
SecurityTypeMaster::slotUp()
{
    ListViewItem* current = _rules->currentItem();
    if (current == NULL) return;

    ListViewItem* above = current->itemAbove();
    if (above == NULL) return;

    if (above->itemAbove() == NULL) {
	_rules->takeItem(current);
	_rules->insertItem(current);
	_rules->setCurrentItem(current);
	_rules->setSelected(current, true);
    } else {
	current->moveItem(above->itemAbove());
    }
}

void
SecurityTypeMaster::slotDown()
{
    ListViewItem* current = _rules->currentItem();
    if (current == NULL) return;

    current->moveItem(current->itemBelow());
}
