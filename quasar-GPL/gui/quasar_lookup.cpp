// $Id: quasar_lookup.cpp,v 1.8 2004/01/31 01:50:30 arandell Exp $
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

#include "quasar_lookup.h"
#include "main_window.h"
#include "quasar_client.h"

QuasarLookup::QuasarLookup(MainWindow* main, QWidget* parent, const char* name)
    : LookupWindow(parent, name)
{
    _main = main;
    _quasar = main->quasar();
    _db = _quasar->db();
}

QuasarLookup::QuasarLookup(QuasarClient* quasar, QWidget* p, const char* name)
    : LookupWindow(p, name)
{
    _main = NULL;
    _quasar = quasar;
    _db = _quasar->db();

    setAllowNew(false);
    setAllowEdit(false);
}

QuasarLookup::~QuasarLookup()
{
}
