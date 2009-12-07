// $Id: worker_thread.cpp,v 1.24 2005/04/12 09:42:01 bpepers Exp $
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

#include "worker_thread.h"
#include "quasar_version.h"
#include "conn.h"
#include "quasar_db.h"
#include "quasar_misc.h"
#include "server_config.h"
#include "message_log.h"
#include "shared_state.h"
#include "license.h"
#include "sha1.h"
#include "base64.h"

#include "account.h"
#include "company.h"
#include "user.h"

#include "customer.h"
#include "dept.h"
#include "discount.h"
#include "employee.h"
#include "expense.h"
#include "item_price.h"
#include "pos_work.h"
#include "station.h"
#include "subdept.h"
#include "tax.h"
#include "tender.h"
#include "tender_count.h"
#include "security_type.h"


#include <qdom.h>
#include <qdir.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>

WorkerThread::WorkerThread()
    : _nextStmtId(0), _haveWork(1), _done(false)
{
    _connection = NULL;
    _db = NULL;
}

WorkerThread::~WorkerThread()
{
    delete _db;
    delete _connection;
}

void
WorkerThread::queueCommand(const QString& command, const QStringList& args)
{
    _commandMutex.lock();
    _command = command;
    _args = args;
    clearResults();
    _commandMutex.unlock();

    _haveWork--;
    _haveResult.wait(100);
}

bool
WorkerThread::resultsAvailable()
{
    return !_resultType.isEmpty();
}

void
WorkerThread::setResults(const QString& type, const QStringList& results)
{
    _resultMutex.lock();
    _resultType = type;
    _results = results;
    _resultMutex.unlock();
}

void
WorkerThread::clearResults()
{
    _resultMutex.lock();
    _resultType = "";
    _results.clear();
    _resultMutex.unlock();
}

void
WorkerThread::getResults(QString& type, QStringList& results)
{
    _resultMutex.lock();
    type = _resultType;
    results = _results;
    _resultMutex.unlock();
}

void
WorkerThread::shutdown()
{
    _done = true;
    _command = "";
    _haveWork--;
    wait();
}

void
WorkerThread::run()
{
    logStatus("Worker thread started");
    while (!_done) {
	_haveWork++;

	_commandMutex.lock();
	QString command = _command;
	QStringList args = _args;
	_commandMutex.unlock();
        if (command.isEmpty()) continue;

	processCommand(command, args);
	_haveResult.wakeOne();
    }
    logStatus("Worker thread stopped");
}

void
WorkerThread::processCommand(const QString& command, const QStringList& args)
{
    _command = command;
    _args = args;
    clearResults();

    if (command == "version") versionCommand();
    else if (command == "system") systemCommand();
    else if (command == "system_id") systemIdCommand();
    else if (command == "resources") resourcesCommand();
    else if (command == "checksum") checksumCommand();
    else if (command == "sendfile") sendFileCommand();
    else if (command == "password") passwordCommand();
    else if (command == "add_user") addUserCommand();
    else if (command == "update_user") updateUserCommand();
    else if (command == "delete_user") deleteUserCommand();
    else if (command == "login") loginCommand();
    else if (command == "set") setCommand();
    else if (command == "commit") commitCommand();
    else if (command == "rollback") rollbackCommand();
    else if (command == "exec_cmd") execCommand();
    else if (command == "alloc_stmt") allocStmtCommand();
    else if (command == "free_stmt") freeStmtCommand();
    else if (command == "set_stmt") setStmtCommand();
    else if (command == "execute") executeCommand();
    else if (command == "next") nextCommand();
    else {
	qWarning("Unknown command: " + command);
	error("Unknown command: " + command);
    }
}

void
WorkerThread::error(const QString& message)
{
    setResults("error:", message);
}

void
WorkerThread::versionCommand()
{
    if (_args.size() != 0) {
	error("Wrong number of version args");
	return;
    }

    QStringList results;
    results << QUASAR_MAJOR_S;
    results << QUASAR_MINOR_S;
    results << QUASAR_PATCH_S;

    setResults("version:", results);
}

void
WorkerThread::systemCommand()
{
    if (_args.size() != 0) {
	error("Wrong number of system args");
	return;
    }

#ifdef LINUX
    setResults("system:", "Linux");
#else
    setResults("system:", "Windows");
#endif
}

void
WorkerThread::systemIdCommand()
{
    if (_args.size() != 0) {
	error("Wrong number of system_id args");
	return;
    }

    ServerConfig config;
    config.load();

    setResults("system_id:", QString::number(config.systemId));
}

static QStringList
resourceValues(const QString& filePath, const QStringList& tags)
{
    QStringList values;
    if (tags.size() == 0)
	return values;

    for (unsigned int i = 0; i < tags.size(); ++i)
	values.push_back("");

    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) {
	qWarning("Failed to open " + filePath + " for reading");
	return values;
    }

    QString errorMsg;
    int errorLine = 0;
    int errorCol = 0;
    QDomDocument doc;
    if (!doc.setContent(&file, &errorMsg, &errorLine, &errorCol)) {
	qWarning("Failed loading " + filePath + " as XML");
	qWarning("Error at %d (%d): " + errorMsg, errorLine, errorCol);
	return values;
    }

    QDomElement root = doc.documentElement();
    for (unsigned int i = 0; i < tags.count(); ++i) {
	if (tags[i].left(1) == ".") {
	    QString value = root.attribute(tags[i].mid(1), "");
	    values[i] = value;
	}
    }

    QDomNodeList nodes = root.childNodes();
    for (unsigned int i = 0; i < nodes.count(); ++i) {
	QDomNode node = nodes.item(i);
	if (node.isNull()) continue;

	QDomElement e = node.toElement();
	if (e.isNull()) continue;

	for (unsigned int j = 0; j < tags.size(); ++j)
	    if (e.tagName() == tags[j])
		values[j] = e.text();
    }

    return values;
}

void
WorkerThread::resourcesCommand()
{
    if (_args.size() < 1) {
	error("Wrong number of resource args");
	return;
    }

    QString type = _args[0];
    QStringList tags;
    for (unsigned int i = 1; i < _args.size(); ++i)
	tags.push_back(_args[i]);

    ServerConfig config;
    config.load();

    // Verify type is valid
    QString dataDir = parseDir(config.dataDir);
    if (!QDir(dataDir).exists(type)) {
	error("File type is invalid: " + type);
	return;
    }

    QDir dir(parseDir(config.dataDir) + "/" + type, "*.xml");
    QStringList entries = dir.entryList();
    QStringList results;
    for (unsigned int i = 0; i < entries.size(); ++i) {
	QString fileName = entries[i];
	QStringList values = resourceValues(dir.filePath(fileName), tags);

	results.append(fileName);
	for (unsigned int i = 0; i < values.size(); ++i)
	    results.append(values[i]);
    }

    setResults("resources:", results);
}

QString
WorkerThread::getPath(const QString& type, const QString& fileName)
{
    ServerConfig config;
    config.load();

    // Verify type is valid
    QString dataDir = parseDir(config.dataDir);
    if (!QDir(dataDir).exists(type)) {
	error("File type is invalid: " + type);
	return "";
    }

    // Verify fileName is just a name and not a path
    QFileInfo info(fileName);
    if (info.dirPath() != ".") {
	error("File name is invalid: " + fileName);
	return "";
    }

    return dataDir + "/" + type + "/" + fileName;
}

void
WorkerThread::checksumCommand()
{
    if (_args.size() != 2) {
	error("Wrong number of checksum args");
	return;
    }

    // Get file path validating values
    QString type = _args[0];
    QString fileName = _args[1];
    QString filePath = getPath(type, fileName);
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.exists()) {
	error("File doesn't exist");
	return;
    }
    if (!file.open(IO_ReadOnly)) {
	error("Can't open file for reading");
	return;
    }

    SHA1_CTX context;
    SHA1Init(&context);

    while (true) {
	char buffer[4096];
	int len = file.readBlock(buffer, 4096);
	if (len == 0) break;
	if (len == -1) {
	    error("Error reading file");
	    return;
	}

	SHA1Update(&context, (uint8_t*)buffer, len);
    }

    uint8_t digest[SHA1_DIGEST_SIZE];
    SHA1Final(&context, digest);

    QString checksum;
    for (unsigned int i = 0; i < SHA1_DIGEST_SIZE; ++i) {
	QString value;
	value.sprintf("%02x", (int)digest[i]);
	checksum += value;
    }

    setResults("checksum:", checksum);
}

void
WorkerThread::sendFileCommand()
{
    if (_args.size() != 3) {
	error("Wrong number of sendfile args");
	return;
    }

    QString type = _args[0];
    QString fileName = _args[1];
    QString flags = _args[2];

    // Get file path validating values
    QString filePath = getPath(type, fileName);
    if (filePath.isEmpty()) return;

    QFile file(filePath);
    if (!file.exists()) {
	error("File doesn't exist");
	return;
    }
    if (!file.open(IO_ReadOnly)) {
	error("Can't open file for reading");
	return;
    }

    // Read in file and decide on compression based on flags and Qt version
    QByteArray data = file.readAll();
#if QT_VERSION >= 0x030100
    if (flags == "compressed")
	data = qCompress(data);
#else
    flags = "";
#endif

    QString text = base64Encode(data);
    QStringList list = QStringList::split("\r\n", text);

    QStringList results;
    results << flags;
    results += list;

    setResults("sendfile:", results);
}

void
WorkerThread::passwordCommand()
{
    if (_args.count() != 4) {
	error("Wrong number of password args");
	return;
    }

    QString fileName = _args[0];
    QString username = _args[1];
    QString oldPassword = _args[2];
    QString newPassword = _args[3];

    // Get company definition
    CompanyDefn company;
    if (!company.load(getPath("companies", fileName))) {
	error("Failed loading company: " + fileName);
	return;
    }

    // Encrypt the passed in password
    QString oldCrypted = sha1Crypt(oldPassword);
    QString newCrypted = sha1Crypt(newPassword);

    Driver* driver = Driver::getDriver(company.dbType());
    if (driver == NULL) {
	logError("Can't load driver: " + company.dbType());
	error("Password change failed");
	return;
    }

    Conn connection(driver);
    if (!connection.dbaConnect(company.database())) {
	logError("Failed to connect to database: " + company.database());
	error("Password change failed");
	return;
    }

    Stmt stmt(connection);
    stmt.setCommand("select passwd from users where name=?");
    stmt.setString(-1, username);

    if (!stmt.execute()) {
	logError("Failed selecting users in: " + company.database());
	error("Password change failed");
	return;
    }

    if (!stmt.next()) {
	logError("Error: select on users failed: " + company.database());
	error("Password change failed");
	return;
    }

    QString currPassword = stmt.getString(1);
    if (currPassword != oldCrypted) {
	logStatus("Password change failed for '" + username + "' in '" +
		   company.database() + "'");
	error("Password change failed");
	return;
    }

    stmt.setCommand("update users set passwd=? where name=?");
    stmt.setString(-1, newCrypted);
    stmt.setString(-1, username);

    if (!stmt.execute()) {
	logError("Failed updating users in: " + company.database());
	error("Password change failed");
	return;
    }

    if (stmt.getUpdateCount() != 1) {
	logError("Error: select on users failed: " + company.database());
	error("Password change failed");
	return;
    }

    if (!connection.commit()) {
	logError("Commit failed in password change");
	error("Password change failed");
	return;
    }

    logStatus("Password change for '" + username + "' in '" +
	       company.database() + "'");

    setResults("password:", "Password changed");
}

void
WorkerThread::addUserCommand()
{
    if (_args.count() != 7) {
	error("Wrong number of add_user args");
	return;
    }

    if (_company.isEmpty()) {
	error("Client has not logged in yet");
	return;
    }

    QString username = _args[0];
    QString password = _args[1];
    Id store_id = Id::fromStringStatic(_args[2]);
    Id employee_id = Id::fromStringStatic(_args[3]);
    Id security_id = Id::fromStringStatic(_args[4]);
    QString screen = _args[5];
    QString state = _args[6];

    // Get company definition
    CompanyDefn company;
    if (!company.load(getPath("companies", _company))) {
	error("Failed loading company: " + _company);
	return;
    }

    Driver* driver = Driver::getDriver(company.dbType());
    if (driver == NULL) {
	logError("Can't load driver: " + company.dbType());
	error("Add user failed");
	return;
    }

    Conn connection(driver);
    if (!connection.dbaConnect(company.database())) {
	logError("Failed to connect to database: " + company.database());
	error("Add user failed");
	return;
    }

    ServerConfig config;
    if (!config.load()) {
	error("Failed loading server config");
	return;
    }

    QuasarDB db(connection);
    db.setUsername("system");
    db.setSystemId(config.systemId);

    // Get current user and their security type
    User currentUser;
    if (!db.lookup(_userName, currentUser)) {
	logError("Failed user lookup: " + _userName);
	error("Add user failed");
	return;
    }
    SecurityType currentType;
    if (currentUser.securityType() != INVALID_ID) {
	if (!db.lookup(currentUser.securityType(), currentType)) {
	    logError("Failed security type lookup for: " + _userName);
	    error("Add user failed");
	    return;
	}
    }

    // Validate can create users
    if (!currentType.securityCheck("UserMaster", "Create")) {
	logError("User not allowed to create users: " + _userName);
	error("Add user failed");
	return;
    }

    User user;
    user.setName(username);
    user.setPassword(sha1Crypt(password));
    user.setDefaultStore(store_id);
    user.setDefaultEmployee(employee_id);
    user.setSecurityType(security_id);
    user.setScreen(screen);
    db.setActive(user, state.lower() == "active");

    if (!db.create(user)) {
	logError("Failed adding user in: " + company.database());
	error("Add user failed");
	return;
    }

    logStatus("Added user '" + username + "' to '" + company.database() + "'");

    setResults("add_user:", user.id().toString());
}

void
WorkerThread::updateUserCommand()
{
    if (_args.count() != 8) {
	error("Wrong number of update_user args");
	return;
    }

    if (_company.isEmpty()) {
	error("Client has not logged in yet");
	return;
    }

    Id user_id = Id::fromStringStatic(_args[0]);
    QString username = _args[1];
    QString password = _args[2];
    Id store_id = Id::fromStringStatic(_args[3]);
    Id employee_id = Id::fromStringStatic(_args[4]);
    Id security_id = Id::fromStringStatic(_args[5]);
    QString screen = _args[6];
    QString state = _args[7];

    // Get company definition
    CompanyDefn company;
    if (!company.load(getPath("companies", _company))) {
	error("Failed loading company: " + _company);
	return;
    }

    Driver* driver = Driver::getDriver(company.dbType());
    if (driver == NULL) {
	logError("Can't load driver: " + company.dbType());
	error("Update user failed");
	return;
    }

    Conn connection(driver);
    if (!connection.dbaConnect(company.database())) {
	logError("Failed to connect to database: " + company.database());
	error("Update user failed");
	return;
    }

    ServerConfig config;
    if (!config.load()) {
	error("Failed loading server config");
	return;
    }

    QuasarDB db(connection);
    db.setUsername("system");
    db.setSystemId(config.systemId);

    // Get current user and their security type
    User currentUser;
    if (!db.lookup(_userName, currentUser)) {
	logError("Failed user lookup: " + _userName);
	error("Update user failed");
	return;
    }
    SecurityType currentType;
    if (currentUser.securityType() != INVALID_ID) {
	if (!db.lookup(currentUser.securityType(), currentType)) {
	    logError("Failed security type lookup for: " + _userName);
	    error("Update user failed");
	    return;
	}
    }

    // Validate can update users
    if (!currentType.securityCheck("UserMaster", "Update")) {
	logError("User not allowed to update users: " + _userName);
	error("Update user failed");
	return;
    }

    User orig;
    if (!db.lookup(user_id, orig)) {
	logError("Failed updating user in: " + company.database());
	error("Update user failed");
	return;
    }

    User user = orig;
    user.setName(username);
    user.setDefaultStore(store_id);
    user.setDefaultEmployee(employee_id);
    user.setSecurityType(security_id);
    user.setScreen(screen);
    db.setActive(user, state.lower() == "active");

    if (password != "<<<no change>>>")
	user.setPassword(sha1Crypt(password));

    if (!db.update(orig, user)) {
	logError("Failed updating user in: " + company.database());
	error("Update user failed");
	return;
    }

    logStatus("Updated user '" + username + "' in '" + company.database()+"'");

    setResults("update_user:", QString::number(user.version()));
}

void
WorkerThread::deleteUserCommand()
{
    if (_args.count() != 1) {
	error("Wrong number of delete_user args");
	return;
    }

    if (_company.isEmpty()) {
	error("Client has not logged in yet");
	return;
    }

    Id user_id = Id::fromStringStatic(_args[0]);

    // Get company definition
    CompanyDefn company;
    if (!company.load(getPath("companies", _company))) {
	error("Failed loading company: " + _company);
	return;
    }

    Driver* driver = Driver::getDriver(company.dbType());
    if (driver == NULL) {
	logError("Can't load driver: " + company.dbType());
	error("Delete user failed");
	return;
    }

    Conn connection(driver);
    if (!connection.dbaConnect(company.database())) {
	logError("Failed to connect to database: " + company.database());
	error("Delete user failed");
	return;
    }

    ServerConfig config;
    if (!config.load()) {
	error("Failed loading server config");
	return;
    }

    QuasarDB db(connection);
    db.setUsername("system");
    db.setSystemId(config.systemId);

    // Get current user and their security type
    User currentUser;
    if (!db.lookup(_userName, currentUser)) {
	logError("Failed user lookup: " + _userName);
	error("Delete user failed");
	return;
    }
    SecurityType currentType;
    if (currentUser.securityType() != INVALID_ID) {
	if (!db.lookup(currentUser.securityType(), currentType)) {
	    logError("Failed security type lookup for: " + _userName);
	    error("Delete user failed");
	    return;
	}
    }

    // Validate can update users
    if (!currentType.securityCheck("UserMaster", "Delete")) {
	logError("User not allowed to delete users: " + _userName);
	error("Delete user failed");
	return;
    }

    User user;
    if (!db.lookup(user_id, user)) {
	logError("Failed deleting user in: " + company.database());
	error("Delete user failed");
	return;
    }
    if (!db.remove(user)) {
	logError("Failed deleting user in: " + company.database());
	error("Delete user failed");
	return;
    }

    logStatus("Deleted user '" + user.name() + "' from '" +
	      company.database() + "'");

    setResults("delete_user:", "User deleted");
}

void
WorkerThread::clientLogin()
{
    if (_args.count() != 3) {
	error("Wrong number of login args");
	return;
    }

    QString fileName = _args[0];
    QString userName = _args[1];
    QString password = _args[2];

    // Get company definition
    CompanyDefn company;
    if (!company.load(getPath("companies", fileName))) {
	error("Failed loading company definition");
	return;
    }

    ServerConfig config;
    if (!config.load()) {
	error("Failed loading server config");
	return;
    }

    // Try to connect to company database
    Driver* driver = Driver::getDriver(company.dbType());
    if (driver == NULL) {
	logError("Can't load driver: " + company.dbType());
	error("Loading server database driver failed");
	return;
    }

    Connection* connection = driver->allocConnection();
    if (!connection->connect(company.database())) {
	logError("Failed to connect to database " + company.database() +
	    ": " + connection->lastError());
	error("Server database connection failed");
	return;
    }

    QuasarDB* db = new QuasarDB(connection);

    // Verify user name and password
    User user;
    if (!db->lookup(userName, user)) {
	error("Invalid user name");
	delete db;
	delete connection;
	return;
    }
    if (user.password() != sha1Crypt(password)) {
	error("Invalid password");
	delete db;
	delete connection;
	return;
    }

    db->setUsername(userName);
    db->setSystemId(config.systemId);
    db->setDefaultStore(user.defaultStore());

    delete _db;
    delete _connection;
    _db = db;
    _connection = connection;
    _company = fileName;
    _userName = userName;

    logStatus("Client login to " + fileName + " as " + userName);

    setResults("login:", "good");
}

void
WorkerThread::loginCommand()
{
    if (_args.count() == 3)
	clientLogin();
    else
	error("Wrong number of login args");
}

void
WorkerThread::setCommand()
{
    if (_args.count() != 2) {
	error("Wrong number of set args");
	return;
    }

    if (_connection == NULL) {
	error("Company connection closed");
	return;
    }

    QString type = _args[0];
    QString value = _args[1].lower();

    if (type == "auto_commit") {
	bool autoCommit = false;
	if (value == "on" || value == "true" || value == "1")
	    autoCommit = true;
	else if (value == "off" || value == "false" || value == "0")
	    autoCommit = false;
	else {
	    error("Invalid auto_commit value: " + value);
	    return;
	}

	if (!_connection->setAutoCommit(autoCommit)) {
	    error("Set auto_commit failed");
	    return;
	}
    } else if (type == "isolation") {
	IsoLevel level;
	if (value == "read_uncommitted")
	    level = READ_UNCOMMITTED;
	else if (value == "read_committed")
	    level = READ_COMMITTED;
	else if (value == "repeatable_read")
	    level = REPEATABLE_READ;
	else if (value == "serializable")
	    level = SERIALIZABLE;
	else {
	    error("Invalid isolation value: " + value);
	    return;
	}

	if (!_connection->setIsolationLevel(level)) {
	    error("Set isolation failed");
	    return;
	}
    } else {
	error("Invalid set type: " + type);
	return;
    }

    setResults("set:", "good");
}

void
WorkerThread::commitCommand()
{
    if (_args.count() != 0) {
	error("Wrong number of commit args");
	return;
    }

    if (_connection == NULL) {
	error("Company connection closed");
	return;
    }

    if (!_connection->commit()) {
	error("Commit failed");
	return;
    }

    setResults("commit:", "good");
}

void
WorkerThread::rollbackCommand()
{
    if (_args.count() != 0) {
	error("Wrong number of rollback args");
	return;
    }

    if (_connection == NULL) {
	error("Company connection closed");
	return;
    }

    if (!_connection->rollback()) {
	error("Rollback failed");
	return;
    }

    setResults("rollback:", "good");
}

void
WorkerThread::execCommand()
{
    if (_args.count() != 1) {
	error("Wrong number of exec_cmd args");
	return;
    }

    if (_connection == NULL) {
	error("Company connection closed");
	return;
    }

    if (!_connection->execute(_args[0])) {
	error("Execute failed");
	return;
    }

    setResults("exec_cmd:", "good");
}

void
WorkerThread::allocStmtCommand()
{
    if (_args.count() != 0 && _args.count() != 1) {
	error("Wrong number of alloc_stmt args");
	return;
    }

    if (_connection == NULL) {
	error("Company connection closed");
	return;
    }

    Statement* stmt = _connection->allocStatement();
    if (stmt == NULL) {
	error("Allocate statement failed");
	return;
    }

    if (_args.count() == 1) {
	if (!stmt->setCommand(_args[0])) {
	    logError("Invalid SQL command:");
	    logError(stmt->lastError());
	    error("Invalid SQL command");
	    delete stmt;
	    return;
	}
    }

    int id = _nextStmtId++;
    _stmtIds.push_back(id);
    _stmts.push_back(stmt);

    QStringList results;
    results << QString::number(id);
    results << QString::number(stmt->paramCount());

    setResults("alloc_stmt:", results);
}

void
WorkerThread::freeStmtCommand()
{
    if (_args.count() != 1) {
	error("Wrong number of free_stmt args");
	return;
    }

    if (_connection == NULL) {
	error("Company connection closed");
	return;
    }

    int id = _args[0].toInt();

    Statement* stmt = findStmt(id);
    if (stmt == NULL) {
	error("Stmt id not found");
	return;
    }

    delete stmt;
    _stmtIds.remove(id);
    _stmts.remove(stmt);

    setResults("free_stmt:", "good");
}

void
WorkerThread::setStmtCommand()
{
    if (_args.count() != 2) {
	error("Wrong number of set_stmt args");
	return;
    }

    if (_connection == NULL) {
	error("Company connection closed");
	return;
    }

    int id = _args[0].toInt();
    QString cmd = _args[1];

    Statement* stmt = findStmt(id);
    if (stmt == NULL) {
	error("Stmt id not found");
	return;
    }

    if (!stmt->setCommand(cmd)) {
	logError("Invalid SQL command:");
	logError(stmt->lastError());
	error("Invalid SQL command");
	return;
    }

    setResults("set_stmt:", QString::number(stmt->paramCount()));
}

void
WorkerThread::executeCommand()
{
    if (_args.count() != 1 && _args.count() != 3) {
	error("Wrong number of execute args");
	return;
    }

    if (_connection == NULL) {
	error("Company connection closed");
	return;
    }

    int id = _args[0].toInt();

    Statement* stmt = findStmt(id);
    if (stmt == NULL) {
	error("Stmt id not found");
	return;
    }

    // Set parameters if they have been passed
    if (_args.count() == 3) {
	QStringList values = split(_args[1]);
	QStringList nulls = split(_args[2]);

	for (int i = 0; i < stmt->paramCount(); ++i) {
	    if (nulls.contains(QString::number(i)))
		stmt->setNull(i);
	    else
		stmt->setString(i, values[i]);
	}
    }

    if (!stmt->execute()) {
	logError("SQL execute failed:");
	logError(stmt->lastError());
	error("Execute SQL command failed");
	return;
    }

    QStringList results;
    results << QString::number(stmt->getUpdateCount());
    results << QString::number(stmt->columnCount());

    setResults("execute:", results);
}

void
WorkerThread::nextCommand()
{
    if (_args.count() != 1) {
	error("Wrong number of next args");
	return;
    }

    if (_connection == NULL) {
	error("Company connection closed");
	return;
    }

    int id = _args[0].toInt();

    Statement* stmt = findStmt(id);
    if (stmt == NULL) {
	error("Stmt id not found");
	return;
    }

    QStringList rows;
    int columnCnt = stmt->columnCount();
    while (rows.size() <= 50 && stmt->next()) {
	TclObject values;
	TclObject nulls;
	for (int i = 0; i < columnCnt; ++i) {
	    Variant value = stmt->getValue(i + 1);
	    values.lappend(value.toString());
	    if (stmt->isNull(i + 1))
		nulls.lappend(QString::number(i));
	}

	TclObject row;
	row.lappend(values);
	row.lappend(nulls);

	rows.push_back(row.toString());
    }

    setResults("next:", rows);
}

Statement*
WorkerThread::findStmt(int id)
{
    int index = _stmtIds.findIndex(id);
    if (index == -1) return NULL;
    return _stmts[index];
}

QStringList
WorkerThread::split(const QString& text)
{
    return TclObject(text).toStringList();
}

void
WorkerThread::append(TclObject& list, Variant value, bool blankZero)
{
    QString text = value.toString();
    if (value.type() == Variant::BOOL)
	text = value.toBool() ? QString("Y") : QString("N");
    if (value.type() == Variant::FIXED && text == "0" && blankZero)
	text = "";
    list.lappend(text);
}


