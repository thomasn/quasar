// $Id: cheque_print.h,v 1.9 2004/09/20 17:35:55 bpepers Exp $
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

#ifndef CHEQUE_PRINT_H
#define CHEQUE_PRINT_H

#include "quasar_window.h"
#include "cheque.h"
#include "label_defn.h"

class LookupEdit;
class ListView;
class QCheckBox;
class QComboBox;

class ChequePrint: public QuasarWindow {
    Q_OBJECT
public:
    ChequePrint(MainWindow* main);
    ~ChequePrint();

    void setCheque(Id cheque_id);

protected slots:
    void slotAccountChanged();
    void slotPrintChanged();
    void slotOk();

protected:
    LookupEdit* _account;
    ListView* _cheques;
    QCheckBox* _printed;
    QComboBox* _type;

    QStringList _labelNames;
    QStringList _labelFiles;

    void loadCheques();
};

#endif // CHEQUE_PRINT_H
