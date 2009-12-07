// $Id: todo.h,v 1.8 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef TODO_H
#define TODO_H

#include "data_object.h"

class Todo: public DataObject {
public:
    // Constructors and Destructor
    Todo();
    ~Todo();

    // Get methods
    const QString& note()		const { return _note; }
    QDate remindOn()			const { return _remind_on; }

    // Set methods
    void setNote(const QString& note)		{ _note = note; }
    void setRemindOn(QDate date)		{ _remind_on = date; }

    // Special methods
    QString description() const;

    // Operations
    bool operator==(const Todo& rhs) const;
    bool operator!=(const Todo& rhs) const;

protected:
    QString _note;		// Note of what to do
    QDate _remind_on;		// Date to remind user
};

#endif // TODO_H
