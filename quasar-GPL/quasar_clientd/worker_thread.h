// $Id: worker_thread.h,v 1.10 2005/04/04 06:11:25 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Accounting
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

#ifndef WORKER_THREAD_H
#define WORKER_THREAD_H

#include "db_driver.h"
#include "company_defn.h"
#include "store.h"
#include "station.h"
#include "tcl_object.h"

#include <qthread.h>
#include <qmutex.h>
#include <qsemaphore.h>
#include <qstringlist.h>

class Gltx;
class QuasarDB;

class WorkerThread: public QThread {
public:
    WorkerThread();
    ~WorkerThread();

    // Queue a command for the thread to execute
    void queueCommand(const QString& command, const QStringList& args);

    // Execute a command immediately
    void processCommand(const QString& command, const QStringList& args);

    // Check if results of queued command are ready yet
    bool resultsAvailable();

    // Results (with mutex locking)
    void setResults(const QString& type, const QStringList& results);
    void clearResults();
    void getResults(QString& type, QStringList& results);

    // Ask worker thread to stop
    void shutdown();

    // Commands
    void versionCommand();		// Return version
    void systemCommand();		// Return operating system
    void systemIdCommand();		// Return system id
    void resourcesCommand();		// Return list of resources
    void checksumCommand();		// Return the checksum of a file
    void sendFileCommand();		// Return the contents of a file
    void passwordCommand();		// Change a users password
    void addUserCommand();		// Add a user
    void updateUserCommand();		// Update a user
    void deleteUserCommand();		// Delete a user
    void clientLogin();			// Login from client
    void loginCommand();		// Login command shared for client/POS
    void setCommand();			// Set driver options
    void commitCommand();		// Commit work
    void rollbackCommand();		// Rollback work
    void execCommand();			// Execute SQL command immediately
    void allocStmtCommand();		// Create a new statement
    void freeStmtCommand();		// Free an allocated statement
    void setStmtCommand();		// Set statement SQL command
    void setParamsCommand();		// Set statement parameters
    void executeCommand();		// Execute a statement
    void nextCommand();			// Return next rows of select stmt

protected:
    void run();
    void error(const QString& message);

    // Get the path to a config file
    QString getPath(const QString& type, const QString& fileName);

    // Create pos_work information
    void createWork(const QString& id, const QString& type);

    QStringList split(const QString& line);
    void append(TclObject& list, Variant data, bool blankZero=false);

    Id getId(const QString& text);
    QDate getDate(const QString& text);
    QTime getTime(const QString& text);
    fixed getFixed(const QString& text);
    fixed getMoney(const QString& text);
    bool getBoolean(const QString& text);

    QString idToString(Id id);
    DataObject::DataType nameToType(const QString& name);
    QString typeToName(DataObject::DataType type);
    QString stateName(const Gltx& gltx);

    // Return the cash tender for change (should likely be redesigned)
    Id cashId();

    // Global POS data
    Connection* _connection;
    QuasarDB* _db;
    QString _company;
    QString _userName;

    // Active statements
    int _nextStmtId;
    QValueList<int> _stmtIds;
    QValueList<Statement*> _stmts;
    Statement* findStmt(int id);

    // Current work
    QMutex _commandMutex;
    QString _command;
    QStringList _args;
    QMutex _resultMutex;
    QString _resultType;
    QStringList _results;
    QSemaphore _haveWork;
    QWaitCondition _haveResult;
    bool _done;

    // Data for next command
    TclObject _data;
    int _index;

    // Store and station POS is working in
    Store _store;
    Station _station;
};

#endif

