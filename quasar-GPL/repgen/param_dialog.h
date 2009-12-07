// $Id: param_dialog.h,v 1.5 2005/01/24 10:01:22 bpepers Exp $
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

#ifndef PARAM_DIALOG_H
#define PARAM_DIALOG_H

#include <qdialog.h>
#include "report_param.h"

class ReportParamType;

// Used for passing parameter values to generate
typedef QMap<QString,QVariant> ParamMap;

class ParamDialog: public QDialog {
    Q_OBJECT
public:
    ParamDialog(QWidget* parent, const ReportParamVector& params,
		const ParamMap& inputParams);
    virtual ~ParamDialog();

    ParamMap getParameters() const;

protected slots:
    void accept();

protected:
    ReportParamVector _params;
    QValueVector<ReportParamType*> _types;
    QValueVector<QWidget*> _widgets;
};

#endif // PARAM_DIALOG_H
