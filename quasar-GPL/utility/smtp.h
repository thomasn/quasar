// $Id: smtp.h,v 1.3 2004/02/03 00:13:26 arandell Exp $
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

#ifndef SMTP_H
#define SMTP_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

class QSocket;
class QTextStream;
class QTimer;

class Smtp: public QObject {
    Q_OBJECT
public:
    Smtp(const QString& smtpHost, int port=25);
    ~Smtp();

    void setFrom(const QString& from)		{ _from = from; }
    void setTo(const QString& to)		{ _to.clear(); _to << to; }
    void setTo(const QStringList& to)		{ _to = to; }
    void setSubject(const QString& text)	{ _subject = text; }
    void setBody(const QString& text)		{ _body = text; }

    bool sendMail();
    void cancelSend();

signals:
    void status(const QString& message);
    void finished(bool result);

private slots:
    void readyRead();
    void connected();
    void timeout();

private:
    void done(bool result);
    void setTimeout(int seconds);

    QString _host;
    int _port;
    QString _from;
    QStringList _to;
    QString _subject;
    QString _body;

    enum State { Init, Mail, Rcpt, Data, Body, Quit, Close };
    int _state;
    QString _message;
    unsigned int _toPos;
    QSocket* _socket;
    QTextStream* _stream;
    QTimer* _timer;
};

#endif
