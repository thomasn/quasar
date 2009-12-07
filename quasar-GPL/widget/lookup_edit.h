// $Id: lookup_edit.h,v 1.21 2004/02/11 00:20:44 bpepers Exp $
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

#ifndef LOOKUP_EDIT_H
#define LOOKUP_EDIT_H

#include "line_edit.h"
#include "lookup_window.h"
#include "id.h"

class LookupEdit: public LineEdit
{
    Q_OBJECT
public:
    LookupEdit(QWidget* parent, const char* name=0);
    LookupEdit(LookupWindow* lookup, QWidget* parent, const char* name=0);
    virtual ~LookupEdit();

    Variant value();
    Id getId();

    LookupWindow* lookupWindow() { return _lookup; }
    void setLookupWindow(LookupWindow* window) { _lookup = window; }

public slots:
    void setValue(Variant value);
    void setId(Id id);
    void created(Id id);
    bool popup(QKeySequence key);

protected:
    bool doValidation(Reason reason);

    LookupWindow* _lookup;
    Id _id;
};

#endif // LOOKUP_EDIT_H
