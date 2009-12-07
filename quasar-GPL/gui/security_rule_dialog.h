// $Id: security_rule_dialog.h,v 1.2 2004/01/31 01:50:31 arandell Exp $
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

#ifndef SECURITY_RULE_DIALOG_H
#define SECURITY_RULE_DIALOG_H

#include <qdialog.h>
#include "security_type.h"

class ComboBox;
class QCheckBox;

class SecurityRuleDialog: public QDialog {
    Q_OBJECT
public:
    SecurityRuleDialog(QWidget* parent);
    virtual ~SecurityRuleDialog();

    void setRule(const SecurityRule& rule);
    SecurityRule getRule();

protected:
    ComboBox* _screen;
    QCheckBox* _allowView;
    QCheckBox* _allowCreate;
    QCheckBox* _allowUpdate;
    QCheckBox* _allowDelete;
};

#endif // SECURITY_RULE_DIALOG_H
