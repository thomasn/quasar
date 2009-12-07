// $Id: importer.h,v 1.2 2004/04/05 08:53:30 bpepers Exp $
//
// Copyright (C) 1998-2003 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Accounting
//
// This file may be distributed and/or modified under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL included in the
// packaging of this file.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// See http://www.linuxcanada.com or email sales@linuxcanada.com for
// information about Quasar Accounting support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#ifndef IMPORTER_H
#define IMPORTER_H

#include <qobject.h>
#include "company_defn.h"
#include "quasar_include.h"

class DataImport;

struct ImportMsg {
    QString type;
    QString name;
    QString text;
};

class Importer: public QObject {
    Q_OBJECT
public:
    Importer(const CompanyDefn& company);
    ~Importer();

    void processFile(const QString& filePath);

protected slots:
    void slotMessage(int count, QString severity, QString message);

protected:
    DataImport* _import;
    vector<ImportMsg> _errors;
    vector<ImportMsg> _warnings;
    vector<ImportMsg> _infos;
};

#endif // IMPORTER_H
