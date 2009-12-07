// $Id: param_dialog.cpp,v 1.7 2005/01/24 10:01:22 bpepers Exp $
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

#include "param_dialog.h"
#include "param_type_factory.h"

#include <qlabel.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qapplication.h>
#include <qmessagebox.h>

ParamDialog::ParamDialog(QWidget* parent, const ReportParamVector& params,
			 const ParamMap& inputParams)
    : QDialog(parent, "ParamDialog", true)
{
    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(1, 1);

    for (unsigned int i = 0; i < params.size(); ++i) {
	const ReportParam& param = params[i];
	ReportParamType* type = ParamTypeFactory::create(param.type);
	if (type == NULL) continue;

	type->setParam(param);
	QLabel* label = new QLabel(param.description, this);
	QWidget* widget = type->getWidget(this);

	if (inputParams.contains(param.name)) {
	    type->setValue(widget, inputParams[param.name]);
	} else if (!param.defaultValue.isEmpty()) {
	    QVariant value;
	    if (type->convert(param.defaultValue, value))
		type->setValue(widget, value);
	}

	_params.push_back(param);
	_types.push_back(type);
	_widgets.push_back(widget);

	grid->addWidget(label, i, 0);
	grid->addWidget(widget, i, 1, AlignLeft | AlignVCenter);
    }

    QFrame* buttons = new QFrame(this);
    QPushButton* ok = new QPushButton(tr("OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("Cancel"), buttons);

    ok->setDefault(true);
    ok->setMinimumSize(cancel->sizeHint());
    cancel->setMinimumSize(cancel->sizeHint());
    connect(ok, SIGNAL(clicked()), SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(6);
    buttonGrid->setMargin(6);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    int row = params.size();
    grid->addMultiCellWidget(buttons, row, row, 0, 1);

    setCaption(tr("Report Parameters"));
}

ParamDialog::~ParamDialog()
{
    for (unsigned int i = 0; i < _types.size(); ++i)
	delete _types[i];
}

ParamMap
ParamDialog::getParameters() const
{
    ParamMap map;
    for (unsigned int i = 0; i < _params.size(); ++i) {
	const ReportParam& param = _params[i];
	ReportParamType* type = _types[i];
	QWidget* widget = _widgets[i];

	QVariant value = type->getValue(widget);
	map[param.name] = value;
    }
    return map;
}

void
ParamDialog::accept()
{
    for (unsigned int i = 0; i < _params.size(); ++i) {
	const ReportParam& param = _params[i];
	ReportParamType* type = _types[i];
	QWidget* widget = _widgets[i];

	bool valid = type->validate(widget);
	if (!valid) {
	    qApp->beep();
	    QMessageBox::critical(this, tr("Error"), tr("Invalid data"));
	    widget->setFocus();
	    return;
	}

	QVariant val = type->getValue(widget);
#if QT_VERSION >= 0x030100
	if (param.manditory && (val.isNull() || val.toString().isEmpty())) {
#else
	if (param.manditory && val.toString().isEmpty()) {
#endif
	    qApp->beep();
	    QMessageBox::critical(this, tr("Error"), tr("Data is manditory"));
	    widget->setFocus();
	    return;
	}
    }

    QDialog::accept();
}
