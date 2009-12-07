// $Id: company_config_tab.h,v 1.4 2005/01/11 12:37:56 bpepers Exp $
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

#ifndef COMPANY_CONFIG_TAB_H
#define COMPANY_CONFIG_TAB_H

#include "config_tab.h"
#include "server_config.h"
#include "company_defn.h"

class ListView;

class CompanyConfigTab: public ConfigTab
{
    Q_OBJECT
public:
    // Constructors
    CompanyConfigTab(QWidget* parent);
    virtual ~CompanyConfigTab();

public slots:
    void loadData();

private slots:
    void slotNewCompany();
    void slotRestoreCompany();
    void slotUpdate();
    void slotImport();
    void slotRunScript();
    void slotBackup();
    void slotDelete();

private:
    void companyList(QValueList<CompanyDefn>& companies); 
    bool findCompany(const QString& name, CompanyDefn& company);
    bool currentCompany(CompanyDefn& company);
    void critical(const QString& message);

    ListView* _companies;
};

#endif // COMPANY_CONFIG_TAB_H
