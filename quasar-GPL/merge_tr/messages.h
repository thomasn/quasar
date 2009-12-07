// $Id: messages.h,v 1.4 2004/02/02 22:42:03 arandell Exp $
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

#ifndef MESSAGES_H
#define MESSAGES_H

#include <qstring.h>
#include <qvaluevector.h>

class QDomNodeList;

struct Message {
    QString source;
    QString type;
    QString translation;

    void load(QDomNodeList& nodes);
};

struct Context {
    QString name;
    QValueVector<Message> messages;

    void load(QDomNodeList& nodes);
    void merge(QDomNodeList& nodes);

    bool messageExists(const QString& source);
    Message& findMessage(const QString& source);
};

class Messages {
public:
    // Constructors and Destructor
    Messages();
    ~Messages();

    // Load/save to ts file
    bool load(const QString& filePath);
    bool merge(const QString& filePath);
    bool save(const QString& filePath) const;

    void setCodecName(const QString& codec);
    QString codecName() const;

    bool contextExists(const QString& name) const;
    Context& findContext(const QString& name);

protected:
    QString _codecName;
    QValueVector<Context> _contexts;
};

#endif // MESSAGES_H
