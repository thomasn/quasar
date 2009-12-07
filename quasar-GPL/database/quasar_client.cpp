// $Id: quasar_client.cpp,v 1.42 2005/03/01 19:59:42 bpepers Exp $
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

#include "quasar_client.h"
#include "quasar_version.h"
#include "quasar_socket.h"
#include "client_conn.h"
#include "client_config.h"
#include "user_config.h"
#include "sequence.h"
#include "quasar_db.h"
#include "screen_defn.h"
#include "label_defn.h"
#include "company.h"
#include "account.h"
#include "store.h"
#include "store_select.h"
#include "station.h"
#include "employee.h"
#include "date_valcon.h"
#include "sha1.h"
#include "base64.h"

#include <qapplication.h>
#include <qdir.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qsocketdevice.h>
#include <unicode/locid.h>

#ifdef LINUX
#include <errno.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <winsock.h>
#endif

#define tr(text) qApp->translate("QuasarClient", text)

extern void qt_generate_epsf(bool b);
static QTranslator* translator = NULL;
static Locale systemLocale;

QuasarClient::QuasarClient(QObject* parent)
    : QObject(parent), _socket(new QuasarSocket(this)), _connection(_socket),
      _db(new QuasarDB(&_connection))
{
    connect(_socket, SIGNAL(hostFound()), SIGNAL(hostFound()));
    connect(_socket, SIGNAL(connected()), SIGNAL(connected()));
    connect(_socket, SIGNAL(connectionClosed()), SLOT(slotServerClosed()));
    connect(_socket, SIGNAL(error(int)), SLOT(slotServerError(int)));
}

QuasarClient::~QuasarClient()
{
    serverDisconnect();
    delete _db;
    delete _socket;
}

QHostAddress
hostAddress()
{
    // Get host name
    char hostname[256];
    gethostname(hostname, 256);

    // Lookup host name
    struct hostent* hostent = NULL;
    int error = 0;
#ifdef WIN32
    WORD version = MAKEWORD(1, 0);
    WSADATA wsaData;
    if (WSAStartup(version, &wsaData) == 0) {
	hostent = gethostbyname(hostname);
	if (hostent == NULL) {
	    unsigned long addr = inet_addr(hostname);
	    hostent = gethostbyaddr((char*)(&addr), sizeof(addr), AF_INET);
	}
	error = WSAGetLastError();
	WSACleanup();
    }
#else
    hostent = gethostbyname(hostname);
    error = h_errno;
#endif

    // Check if hostname lookup failed
    QHostAddress addr;
    if (hostent == NULL) {
	qWarning("Error: failed looking up '%s' (%d)", hostname, error);
    } else {
	int n1 = ((unsigned char*)hostent->h_addr)[0];
	int n2 = ((unsigned char*)hostent->h_addr)[1];
	int n3 = ((unsigned char*)hostent->h_addr)[2];
	int n4 = ((unsigned char*)hostent->h_addr)[3];
	QString text = QString("%1.%2.%3.%4").arg(n1).arg(n2).arg(n3).arg(n4);
	addr.setAddress(text);
    }

    return addr;
}

void
QuasarClient::serverConnect(const QString& hostname, int port)
{
    serverDisconnect();

    _hostname = hostname;
    _port = port;
    _socket->connectToHost(hostname, port);
}

void
QuasarClient::serverDisconnect()
{
    closeCompany();

    _hostname = "";
    _port = -1;
    _socket->close();
}

bool
QuasarClient::serverConnected()
{
    return _socket->state() == QSocket::Connected;
}

int
QuasarClient::serverState()
{
    return _socket->state();
}

QStringList
QuasarClient::serverCommand(const QString& type, const QStringList& args)
{
    _socket->send(type, args);
    return _socket->recv();
}

bool
QuasarClient::error(const QString& message)
{
    qWarning("Error: " + message);
    _lastError = message;
    return false;
}

bool
QuasarClient::resourceList(const QString& type, const QString& tag1,
			   const QString& tag2, QStringList& filenames,
			   QStringList& vals1, QStringList& vals2)
{
    if (!serverConnected())
	return error("No connected");

    filenames.clear();
    vals1.clear();
    vals2.clear();

    QStringList args;
    args << type;
    if (!tag1.isEmpty())
	args << tag1;
    if (!tag2.isEmpty())
	args << tag2;

    QStringList results = serverCommand("resources", args);
    if (results[0] != "resources:")
	return error(results[1]);

    for (unsigned int i = 1; i < results.size(); i++) {
	filenames.push_back(results[i]);
	if (!tag1.isEmpty())
	    vals1.push_back(results[++i]);
	if (!tag2.isEmpty())
	    vals2.push_back(results[++i]);
    }

    return true;
}

bool
QuasarClient::resourceList(const QString& type, const QString& tag,
			   QStringList& filenames, QStringList& values)
{
    QStringList junk;
    return resourceList(type, tag, "", filenames, values, junk);
}

bool
QuasarClient::resourceList(const QString& type, QStringList& filenames)
{
    QStringList junk1, junk2;
    return resourceList(type, "", "", filenames, junk1, junk2);
}

static QString
fileChecksum(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(IO_ReadOnly)) {
	return "";
    }

    SHA1_CTX context;
    SHA1Init(&context);

    while (true) {
	char buffer[4096];
	int len = file.readBlock(buffer, 4096);
	if (len == 0) break;
	if (len == -1) return "";

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

    return checksum;
}

bool
QuasarClient::resourceFetch(const QString& type, const QString& fileName,
			    QString& localFilename)
{
    if (!serverConnected())
	return error("Not connected");

    // Get the local cache directory
    UserConfig config;
    config.load();
    QString cacheStart = parseDir(config.cacheDir);
    if (cacheStart.isEmpty())
#ifdef WIN32
	cacheStart = QDir::homeDirPath() + "/QuasarConfig/cache";
#else
	cacheStart = QDir::homeDirPath() + "/.quasar/cache";
#endif

    // Drill down to bottom directory creating directories as needed
    QDir cacheDir(cacheStart);
    if (!cacheDir.exists()) {
	cacheDir.cdUp();
	cacheDir.mkdir("cache");
	cacheDir.cd("cache");
    }
    if (!cacheDir.cd(_hostname)) {
	cacheDir.mkdir(_hostname);
	cacheDir.cd(_hostname);
    }
    if (!cacheDir.cd(type)) {
	cacheDir.mkdir(type);
	cacheDir.cd(type);
    }

    // If file exists in cache, initiate verifying its SHA-1 hash
    QString cachePath = cacheDir.filePath(fileName);
    if (QFileInfo(cachePath).exists()) {
	QString cacheChecksum = fileChecksum(cachePath);

	QStringList args;
	args << type;
	args << fileName;

	QStringList results = serverCommand("checksum", args);
	if (results[0] == "checksum:" && results[1] == cacheChecksum) {
	    localFilename = cachePath;
	    return true;
	}
    }

    QStringList args;
    args << type;
    args << fileName;
#if QT_VERSION >= 0x030100
    args << "compressed";
#else
    args << "";
#endif

    // Not in cache at all so just request it gets sent
    QStringList results = serverCommand("sendfile", args);
    if (results[0] != "sendfile:")
	return error(results[1]);

    // Open file for writing and write data to the file
    QFile file(cachePath);
    if (!file.open(IO_WriteOnly))
	return error("Can't write to '" + cachePath + "'");

    // Check for compressed
    bool compressed = false;
    QString flags = results[1];
    if (flags == "compressed")
	compressed = true;

    // Convert received base64 data to real data
    results.pop_front();
    results.pop_front();
    QString text = results.join("\n");
    QByteArray data;
    if (!base64Decode(text, data))
	return error("Can't base64 decode data");

#if QT_VERSION >= 0x030100
    if (compressed)
	data = qUncompress(data);
#endif

    file.writeBlock(data, data.size());
    file.close();

#ifdef LINUX
    chmod(cachePath, 0600);
#endif

    localFilename = cachePath;
    return true;
}

bool
QuasarClient::openCompany(const QString& fileName, const QString& username,
			  const QString& password)
{
    if (!serverConnected())
	return error("Not connected");
    closeCompany();

    // Connect to client
    if (!_connection.connect(fileName, username, password)) {
	qWarning("Error: " + _connection.lastError());
	return error("Client database connection failed");
    }

    // Verify version matches
    if (_db->getConfig("version") != QUASAR_DB_VERSION) {
	qWarning("Error: database is for a different Quasar version");
	return error("Database is for a different Quasar version");
    }

    // Get system id from server
    QStringList results = serverCommand("system_id", QStringList());
    int system_id = results[1].toInt();

    _fileName = fileName;
    _username = username;
    _db->setUsername(_username);
    _db->setSystemId(system_id);
    _db->lookup(_username, _user);
    return true;
}

void
QuasarClient::closeCompany()
{
    _connection.disconnect();
}

bool
QuasarClient::companyOpen()
{
    return _connection.connected();
}

bool
QuasarClient::addUser(User& user)
{
    if (!serverConnected())
	return error("Not connected");
    if (!companyOpen())
	return error("No company");

    QStringList args;
    args << user.name();
    args << user.password();
    args << user.defaultStore().toString();
    args << user.defaultEmployee().toString();
    args << user.securityType().toString();
    args << user.screen();
    args << (user.isActive() ? "active" : "inactive");

    QStringList results = serverCommand("add_user", args);
    if (results[0] != "add_user:")
	return error(results[1]);

    user.setId(Id::fromStringStatic(results[1]));
    return true;
}

bool
QuasarClient::updateUser(const User& orig, User& user)
{
    if (!serverConnected())
	return error("Not connected");
    if (!companyOpen())
	return error("No company");

    QStringList args;
    args << orig.id().toString();
    args << user.name();
    args << user.password();
    args << user.defaultStore().toString();
    args << user.defaultEmployee().toString();
    args << user.securityType().toString();
    args << user.screen();
    args << (user.isActive() ? "active" : "inactive");

    QStringList results = serverCommand("update_user", args);
    if (results[0] != "update_user:")
	return error(results[1]);

    if (_user.id() == user.id())
	_user = user;

    user.setVersion(results[1].toInt());
    return true;
}

bool
QuasarClient::deleteUser(const User& user)
{
    if (!serverConnected())
	return error("Not connected");
    if (!companyOpen())
	return error("No company");

    QStringList args;
    args << user.id().toString();

    QStringList results = serverCommand("delete_user", args);
    if (results[0] != "delete_user:")
	return error(results[1]);

    return true;
}

bool
QuasarClient::changePassword(const QString& current, const QString& password)
{
    if (!serverConnected())
	return error("Not connected");
    if (!companyOpen())
	return error("No company");

    QStringList args;
    args << _fileName;
    args << _username;
    args << current;
    args << password;

    QStringList results = serverCommand("password", args);
    if (results[0] != "password:")
	return error(results[1]);

    return true;
}

void
QuasarClient::slotServerClosed()
{
    serverDisconnect();
    emit disconnected();
}

void
QuasarClient::slotServerError(int errorNum)
{
    serverDisconnect();
    emit connectionError(errorNum);
}

QString
QuasarClient::localesDir()
{
    ClientConfig config;
    config.load();
    return parseDir(config.localeDir);
}

QString
QuasarClient::helpDir()
{
    Locale locale = Locale::getDefault();
    QString language(locale.getLanguage());
    QString country(locale.getCountry());

    QDir dir(localesDir());
    if (!dir.cd(language + "_" + country)) {
	if (!dir.cd(language)) {
	    qWarning("No help for locale %s", locale.getName());
	    dir.cd("en_CA");
	}
    }
    dir.cd("help");
    return dir.path();
}

void
QuasarClient::setLocale(const QString& name)
{
    // Get locale from ICU using name (or system one if name is blank)
    Locale locale;
    if (name.isEmpty())
	locale = systemLocale;
    else
	locale = Locale::createFromName(name);

    // Set default locale for ICU
    UErrorCode status = U_ZERO_ERROR;
    Locale::setDefault(locale, status);
    if (U_FAILURE(status)) {
	qWarning("Failed setting locale to " + name);
	return;
    }

    // Remove old translator and install new one if found
    if (translator != NULL) {
	qApp->removeTranslator(translator);
	delete translator;
    }

    translator = new QTranslator(NULL);
    bool found = false;
    QString language(locale.getLanguage());
    QString country(locale.getCountry());

    if (!country.isEmpty()) {
	QString dir = localesDir() + "/" + language + "_" + country;
	if (translator->load("messages.qm", dir))
	    found = true;
    }
    if (!found) {
	QString dir = localesDir() + "/" + language;
	if (translator->load("messages.qm", dir))
	    found = true;
    }

    if (found)
	qApp->installTranslator(translator);
    else
	qWarning("Failed to find translation for locale " + name);
}

Id
QuasarClient::defaultStore(bool selling)
{
    return _db->defaultStore(selling);
}

Id
QuasarClient::defaultStation()
{
    if (!_connection.connected())
	return INVALID_ID;

    if (_default_station != INVALID_ID)
	return _default_station;

    ClientConfig config;
    if (!config.load())
	return INVALID_ID;

    if (config.stationNumber.isEmpty())
	return INVALID_ID;

    Station station;
    if (!_db->lookup(config.stationNumber, station))
	return INVALID_ID;

    _default_station = station.id();
    return _default_station;
}

Id
QuasarClient::defaultEmployee()
{
    if (!_connection.connected())
	return INVALID_ID;

    if (_default_employee != INVALID_ID)
	return _default_employee;

    _default_employee = _user.defaultEmployee();
    return _default_employee;
}

void
QuasarClient::setDefaultStore(Id store_id)
{
    _db->setDefaultStore(store_id);
}

void
QuasarClient::setDefaultStation(Id station_id)
{
    _default_station = station_id;
}

void
QuasarClient::setDefaultEmployee(Id employee_id)
{
    _default_employee = employee_id;
}

bool
QuasarClient::securityCheck(const QString& screen, const QString& operation)
{
    if (_security.id() != _user.securityType()) {
	_security = SecurityType();
	_db->lookup(_user.securityType(), _security);
    }

    return _security.securityCheck(screen, operation);
}

int
QuasarClient::storeCount()
{
    if (!_connection.connected())
	return 0;

    StoreSelect conditions;
    conditions.activeOnly = true;
    vector<Store> stores;
    _db->select(stores, conditions);
    return stores.size();
}
