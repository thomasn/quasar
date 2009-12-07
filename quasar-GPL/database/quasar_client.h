// $Id: quasar_client.h,v 1.27 2005/03/01 19:59:42 bpepers Exp $
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

#ifndef QUASAR_CLIENT_H
#define QUASAR_CLIENT_H

#include <qobject.h>

#include "data_object.h"
#include "company_defn.h"
#include "user.h"
#include "security_type.h"
#include "db_driver.h"
#include "client_conn.h"

class QuasarDB;
class QuasarSocket;

class QuasarClient: public QObject {
    Q_OBJECT
public:
    QuasarClient(QObject* parent=NULL);
    ~QuasarClient();

    // Connect to a quasar server.  This call is asynchronous and will
    // return almost immediately and use signals to indicate the status
    // as the connection is made (hostnameFound, connected, ...)
    void serverConnect(const QString& hostname, int port);
    void serverDisconnect();
    bool serverConnected();
    int serverState();

    // Get a list of resources available of a given type.  Two tags
    // can be specified and if the data file is XML, the value of
    // those tags will be returned as well (used to get name or other
    // data from the XML file without reading it all locally).
    bool resourceList(const QString& type, const QString& tag1,
		      const QString& tag2, QStringList& filenames,
		      QStringList& values1, QStringList& values2);
    bool resourceList(const QString& type, const QString& tag,
		      QStringList& filenames, QStringList& values);
    bool resourceList(const QString& type, QStringList& filenames);

    // Fetch a resource file to the client computer so it can be used.
    // If the file already exists in the users resource cache and the
    // SHA-1 hash matches, it will be used directly instead of asking
    // for the file to be sent again.  The local filename is set on
    // return and points to the local copy of the data.
    bool resourceFetch(const QString& type, const QString& fileName,
		       QString& localFilename);

    // Open a company database.  You must be connected to a server.
    bool openCompany(const QString& fileName, const QString& username,
		     const QString& password);
    void closeCompany();
    bool companyOpen();

    // Commands to work with users from the client.  The Quasar client
    // must use these instead of the direct QuasarDB methods since the
    // client connects to the database as a user without permissions to
    // insert/update/delete from the users table.
    bool addUser(User& user);
    bool updateUser(const User& orig, User& usr);
    bool deleteUser(const User& user);

    // Change the password of a user.  The old password needs to be
    // given as well as the new one and the request is sent off to the
    // server.  The password is only changed in the current company.
    bool changePassword(const QString& current, const QString& password);

    // Current connection
    QString hostname()		{ return _hostname; }
    QString fileName()		{ return _fileName; }
    QString username()		{ return _username; }
    Connection* connection()	{ return &_connection; }
    QuasarDB* db()		{ return _db; }
    const User& user()		{ return _user; }

    // Defaults for store/station/employee (not saved)
    Id defaultStore(bool selling=false);
    Id defaultStation();
    Id defaultEmployee();
    void setDefaultStore(Id store_id);
    void setDefaultStation(Id station_id);
    void setDefaultEmployee(Id employee_id);

    // Security
    bool securityCheck(const QString& screen, const QString& operation);

    // Misc
    int storeCount();
    QString lastError() const { return _lastError; }

    // Directories where client information can be found
    static QString localesDir();
    static QString helpDir();

    // Locales
    static void setLocale(const QString& locale);

signals:
    // From serverConnect
    void hostFound();
    void connected();
    void connectionError(int errorNum);
    void disconnected();

private slots:
    void slotServerClosed();
    void slotServerError(int errorNum);

protected:
    QStringList serverCommand(const QString& command, const QStringList& args);
    bool error(const QString& message);

    // Server connection
    QString _hostname;
    int _port;
    QuasarSocket* _socket;

    // company connection
    QString _fileName;
    QString _username;
    ClientConn _connection;
    User _user;
    SecurityType _security;

    // Other info
    QuasarDB* _db;
    Id _default_station;
    Id _default_employee;
    QString _lastError;

    friend class ClientConn;
    friend class ClientStmt;
};

#endif // QUASAR_CLIENT_H
