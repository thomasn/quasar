// $Id: quasar_db.cpp,v 1.78 2005/03/01 19:59:42 bpepers Exp $
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

#include "quasar_db.h"
#include "quasar_client.h"
#include "store.h"
#include "card.h"
#include "gltx.h"
#include "gltx_select.h"
#include "account.h"
#include "account_select.h"
#include "company.h"
#include "sequence.h"

QValueStack<QString*> QuasarDB::_errors;

QuasarDB::QuasarDB(Connection* connection, QObject* parent)
    : QObject(parent, "QuasarDB"), _connection(connection),
      _system_id(0), _auto_commit(true)
{
}

QuasarDB::~QuasarDB()
{
}

QString
QuasarDB::version() const
{
    return QString::number(versionMajor()) + "." +
	QString::number(versionMinor()) + "." +
	QString::number(versionPatch());
}

int
QuasarDB::versionMajor() const
{
    return 1;
}

int
QuasarDB::versionMinor() const
{
    return 0;
}

int
QuasarDB::versionPatch() const
{
    return 0;
}

Id
QuasarDB::defaultStore(bool selling)
{
    if (!_connection->connected())
	return INVALID_ID;

    if (selling) {
	Store store;
	lookup(_default_store, store);
	if (!store.canSell())
	    return INVALID_ID;
    }

    return _default_store;
}

void
QuasarDB::setUsername(const QString& name)
{
    _username = name;
}

void
QuasarDB::setSystemId(int system_id)
{
    _system_id = system_id;
}

void
QuasarDB::setDefaultStore(Id store_id)
{
    _default_store = store_id;
}

bool
QuasarDB::sequenceLock(const Sequence& seq)
{
    QString cmd = "update id_table set table_name=table_name where "
	"table_name=? and attr_name=? and system_id=?";
    Stmt stmt(_connection, cmd);
    stmtSetString(stmt, seq.tableName());
    stmtSetString(stmt, seq.attrName());
    stmtSetInt(stmt, seq.systemId());
    return execute(stmt);
}

bool
QuasarDB::sequenceGet(Sequence& seq)
{
    QString cmd = "select min_id,max_id,next_id from id_table where "
	"table_name=? and attr_name=? and system_id=?";
    Stmt stmt(_connection, cmd);
    stmtSetString(stmt, seq.tableName());
    stmtSetString(stmt, seq.attrName());
    stmtSetInt(stmt, seq.systemId());
    if (!execute(stmt)) return false;

    if (stmt.next()) {
	seq.setMinNumber(stmtGetFixed(stmt, 1));
	seq.setMaxNumber(stmtGetFixed(stmt, 2));
	seq.setNextNumber(stmtGetFixed(stmt, 3));
	return true;
    }

    // Set starting default values
    fixed added = seq.systemId() * DEFAULT_MAX;
    seq.setMinNumber(DEFAULT_MIN + added);
    seq.setMaxNumber(DEFAULT_MAX + added);
    seq.setNextNumber(DEFAULT_MIN + added);

    cmd = "insert into id_table (table_name,attr_name,next_id,"
	"system_id,min_id,max_id) values (?,?,?,?,?,?)";
    stmt.setCommand(cmd);
    stmtSetString(stmt, seq.tableName());
    stmtSetString(stmt, seq.attrName());
    stmtSetFixed(stmt, seq.nextNumber());
    stmtSetInt(stmt, seq.systemId());
    stmtSetFixed(stmt, seq.minNumber());
    stmtSetFixed(stmt, seq.maxNumber());
    return execute(stmt);
}

bool
QuasarDB::sequenceSet(const Sequence& seq)
{
    QString cmd = "update id_table set min_id=?,max_id=?,next_id=? where "
	"table_name=? and attr_name=? and system_id=?";
    Stmt stmt(_connection, cmd);
    stmtSetFixed(stmt, seq.minNumber());
    stmtSetFixed(stmt, seq.maxNumber());
    stmtSetFixed(stmt, seq.nextNumber());
    stmtSetString(stmt, seq.tableName());
    stmtSetString(stmt, seq.attrName());
    stmtSetInt(stmt, seq.systemId());
    return execute(stmt);
}

fixed
QuasarDB::uniqueNumber(const QString& table, const QString& attr)
{
    Sequence seq;
    seq.setTableName(table);
    seq.setAttrName(attr);
    seq.setSystemId(_system_id);

    if (!sequenceLock(seq)) return -1;
    if (!sequenceGet(seq)) return -1;
    fixed number = seq.nextNumber();

    // Check number selected is unique
    QString cmd = "select count(*) from " + table + " where " +
	attr + " = ?";
    Stmt stmt(_connection, cmd);
    while (true) {
	stmtSetString(stmt, number.toString());
	if (!execute(stmt)) return -1;
	stmt.next();
	int count = stmtGetInt(stmt, 1);
	if (count == 0) break;

	number = number + 1;
	if (number > seq.maxNumber())
	    number = seq.minNumber();

	if (number == seq.nextNumber()) {
	    qWarning("Out of " + table + " numbers");
	    number = seq.minNumber();
	    break;
	}
    }

    seq.setNextNumber(number + 1);
    if (seq.nextNumber() > seq.maxNumber())
	seq.setNextNumber(seq.minNumber());
    if (!sequenceSet(seq)) return -1;

    return number;
}

Id
QuasarDB::uniqueId()
{
    Sequence seq;
    seq.setTableName("data_object");
    seq.setAttrName("object_id");
    seq.setSystemId(_system_id);

    Id id;
    if (!sequenceLock(seq)) return id;
    if (!sequenceGet(seq)) return id;
    fixed number = seq.nextNumber();

    // Check id selected is unique
    QString cmd = "select count(*) from data_object where object_id=?";
    Stmt stmt(_connection, cmd);
    stmtSetFixed(stmt, number);
    if (!execute(stmt)) return id;
    stmt.next();
    int cnt = stmtGetInt(stmt, 1);

    // If not unique, look for maximum in range
    if (cnt != 0) {
	cmd = "select max(object_id) from data_object where "
	    "object_id between ? and ?";
	stmt.setCommand(cmd);
	stmtSetFixed(stmt, seq.minNumber());
	stmtSetFixed(stmt, seq.maxNumber());
	if (!execute(stmt)) return id;
	stmt.next();
	number = stmtGetFixed(stmt, 1) + 1;
    }

    seq.setNextNumber(number + 1);
    if (!sequenceSet(seq)) return id;

    id.setObjectId(number);
    return id;
}

// This is separate since transactions share the gltx table but need
// different numbers.
fixed
QuasarDB::uniqueTxNumber(DataObject::DataType type)
{
    Sequence seq;
    seq.setTableName("gltx");
    seq.setAttrName(DataObject::dataTypeName(type));
    seq.setSystemId(_system_id);

    if (!sequenceLock(seq)) return -1;
    if (!sequenceGet(seq)) return -1;
    fixed number = seq.nextNumber();

    // Check id selected is unique
    QString cmd = "select count(*) from gltx where number=? and "
	"data_type=?";
    Stmt stmt(_connection, cmd);
    while (true) {
	stmtSetString(stmt, number.toString());
	stmtSetInt(stmt, type);
	if (!execute(stmt)) return -1;
	stmt.next();
	if (stmtGetInt(stmt,1) == 0) break;

	number = number + 1;
	if (number > seq.maxNumber())
	    number = seq.minNumber();

	if (number == seq.nextNumber()) {
	    qWarning("Out of transaction numbers");
	    number = -1;
	    break;
	}
    }

    seq.setNextNumber(number + 1);
    if (seq.nextNumber() > seq.maxNumber())
	seq.setNextNumber(seq.minNumber());
    if (!sequenceSet(seq)) return -1;

    return number;
}

bool
QuasarDB::getSequence(Sequence& seq)
{
    if (!sequenceGet(seq)) return false;
    commit();
    return true;
}

bool
QuasarDB::setSequence(const Sequence& seq)
{
    if (!sequenceSet(seq)) return false;
    commit();
    return true;
}

bool
QuasarDB::setConfig(const QString& key, const QString& value)
{
    // First try to insert
    QString cmd = "insert into db_config (config_key,config_value) "
	"values (?,?)";
    Stmt stmt(_connection, cmd);
    stmtSetString(stmt, key);
    stmtSetString(stmt, value);
    if (!stmt.execute()) {
	// Insert failed so try update
	cmd = "update db_config set config_value=? where config_key=?";
	stmt.setCommand(cmd);
	stmtSetString(stmt, value);
	stmtSetString(stmt, key);
	if (!stmt.execute()) {
	    rollback();
	    qWarning("Failed setting config: " + key);
	    return false;
	}
    }

    commit();
    return true;
}

QString
QuasarDB::getConfig(const QString& key)
{
    QString cmd = "select config_value from db_config where config_key=?";
    Stmt stmt(_connection, cmd);
    stmtSetString(stmt, key);
    if (!stmt.execute()) {
	rollback();
	qWarning("Failed getting config: " + key);
	return "";
    }

    QString value;
    if (stmt.next())
	value = stmtGetString(stmt, 1);

    commit();
    return value;
}

bool
QuasarDB::validate(const DataObject&)
{
    // TODO: validate DataObject
    return true;
}

bool
QuasarDB::execute(Stmt& stmt)
{
    if (!stmt.execute()) {
	rollback();
	return error(stmt.lastError());
    }
    return true;
}

void
QuasarDB::commit(bool force)
{
    if (!_auto_commit && !force) return;
    if (!_connection->commit()) {
	qWarning("commit failed");
	rollback();
    }
}

void
QuasarDB::rollback(bool force)
{
    if (!_auto_commit && !force) return;
    if (!_connection->rollback())
	qWarning("rollback failed");
}

QString*
QuasarDB::nextError()
{
    if (_errors.empty()) return NULL;
    QString* next = _errors.top();
    _errors.pop();
    return next;
}

void
QuasarDB::clearErrors()
{
    QString* msg = NULL;
    while ((msg = nextError())) {
	delete msg;
    }
}

bool
QuasarDB::error(const QString& msg)
{
    _errors.push(new QString(msg));
    qWarning(tr("Error: %1").arg(msg));
    rollback();
    return false;
}

void
QuasarDB::dataSignal(DataEvent::Type type, const DataObject& data)
{
    emit dataEvent(new DataEvent(type, data.dataType(), data.id()));
}

QString
QuasarDB::insertText(const QString& table, const QString& id,
		     const QString& fields)
{
    QStringList list = QStringList::split(",", fields);
    QString values = "?";
    for (unsigned int i = 0; i < list.count(); ++i)
	values += ",?";

    QString columns = id;
    if (!fields.isEmpty()) columns += "," + fields;

    return "insert into " + table + " (" + columns + ") values (" +
	values + ")";
}

QString
QuasarDB::updateText(const QString& table, const QString& id,
		     const QString& fields)
{
    QStringList list = QStringList::split(",", fields);
    QString values = "";
    for (unsigned int i = 0; i < list.count(); ++i) {
	if (values != "") values += ",";
	values += list[i] + "=?";
    }

    QString cmd = "update " + table + " set " + values + " where " +
	id + "=?";
    return cmd;
}

QString
QuasarDB::insertCmd(const QString& table, const QString& id,
		    const QString& fields)
{
    return insertText(table, id, "version,created_by,updated_by,inactive_by,"
		      "created_on,updated_on,inactive_on,external_id," +
		      fields);
}

QString
QuasarDB::updateCmd(const QString& table, const QString& id,
		    const QString& fields)
{
    QStringList list = QStringList::split(",", fields);
    QString values = "version=?,updated_by=?,inactive_by=?,updated_on=?,"
	"inactive_on=?,external_id=?";
    for (unsigned int i = 0; i < list.count(); ++i)
	values += "," + list[i] + "=?";

    QString cmd = "update " + table + " set " + values + " where " +
	id + "=? and version=?";
    return cmd;
}

QString
QuasarDB::removeCmd(const QString& table, const QString& id)
{
    return "delete from " + table + " where " + id + "=?";
}

QString
QuasarDB::selectCmd(const QString& table, const QString& id,
		    const QString& fields, const Select& where,
		    const QString& order)
{
    QString cmd = "select " + id + ",version,created_by,updated_by,"
	"inactive_by,created_on,updated_on,inactive_on,external_id," +
	fields + " from " + table + where.where();
    if (!order.isEmpty()) cmd += " order by " + order;
    return cmd;
}

void
QuasarDB::insertData(DataObject& data, Stmt& stmt)
{
    Id id = uniqueId();
    if (id.isNull()) {
	error("uniqueId failed");
	return;
    }

    Stmt stmt2(_connection, "insert into data_object "
		    "(object_id,data_type) values (?,?)");
    stmtSetId(stmt2, id);
    stmtSetInt(stmt2, data.dataType());
    if (!execute(stmt2)) {
	error("creating data_object failed");
	return;
    }

    data.setId(id);
    data.setVersion(0);
    data.setCreatedBy(_username);
    data.setUpdatedBy(QString::null);
    data.setCreatedOn(QDate::currentDate());
    data.setUpdatedOn(QDate());
    if (data.inactiveBy().isEmpty())
	data.setInactiveOn(QDate());
    else
	data.setInactiveOn(QDate::currentDate());

    stmtSetId(stmt, data.id());
    stmtSetInt(stmt, data.version());
    stmtSetString(stmt, data.createdBy());
    stmtSetString(stmt, data.updatedBy());
    stmtSetString(stmt, data.inactiveBy());
    stmtSetDate(stmt, data.createdOn());
    stmtSetDate(stmt, data.updatedOn());
    stmtSetDate(stmt, data.inactiveOn());
    if (data.externalId() == -1)
	stmt.setNull(-1);
    else
	stmtSetFixed(stmt, data.externalId());
}

void
QuasarDB::updateData(const DataObject& orig, DataObject& data, Stmt& stmt)
{
    if (data.dataType() != orig.dataType()) {
	Stmt stmt2(_connection, "update data_object set data_type = ? "
			"where object_id = ?");
	stmtSetInt(stmt2, data.dataType());
	stmtSetId(stmt2, data.id());
	execute(stmt2);
    }

    data.setVersion(orig.version() + 1);
    data.setUpdatedBy(_username);
    data.setUpdatedOn(QDate::currentDate());

    stmtSetInt(stmt, data.version());
    stmtSetString(stmt, data.updatedBy());
    stmtSetString(stmt, data.inactiveBy());
    stmtSetDate(stmt, data.updatedOn());
    stmtSetDate(stmt, data.inactiveOn());
    if (data.externalId() == -1)
	stmt.setNull(-1);
    else
	stmtSetFixed(stmt, data.externalId());
}

bool
QuasarDB::removeData(const DataObject& data, const QString& table,
		     const QString& id_field)
{
    if (!remove(data, table, id_field)) return false;
    return remove(data, "data_object", "object_id");
}

bool
QuasarDB::remove(const DataObject& data, const QString& table,
		 const QString& id_field)
{
    Stmt stmt(_connection, removeCmd(table, id_field));
    stmtSetId(stmt, data.id());
    if (!stmt.execute())
	return error(stmt.lastError());
    return true;
}

void
QuasarDB::selectData(DataObject& data, Stmt& stmt, int& next)
{
    data.setId(stmtGetId(stmt, next++));
    data.setVersion(stmtGetInt(stmt, next++));
    data.setCreatedBy(stmtGetString(stmt, next++));
    data.setUpdatedBy(stmtGetString(stmt, next++));
    data.setInactiveBy(stmtGetString(stmt, next++));
    data.setCreatedOn(stmtGetDate(stmt, next++));
    data.setUpdatedOn(stmtGetDate(stmt, next++));
    data.setInactiveOn(stmtGetDate(stmt, next++));
    if (stmt.isNull(next))
	data.setExternalId(-1);
    else
	data.setExternalId(stmtGetFixed(stmt, next));
    ++next;
}

void
QuasarDB::selectCard(Card& card, Stmt& stmt, int& next)
{
    card.setCompany(stmtGetBool(stmt, next++));
    card.setFirstName(stmtGetString(stmt, next++));
    card.setLastName(stmtGetString(stmt, next++));
    card.setNumber(stmtGetString(stmt, next++));
    card.setStreet(stmtGetString(stmt, next++));
    card.setStreet2(stmtGetString(stmt, next++));
    card.setCity(stmtGetString(stmt, next++));
    card.setProvince(stmtGetString(stmt, next++));
    card.setCountry(stmtGetString(stmt, next++));
    card.setPostal(stmtGetString(stmt, next++));
    card.setPhoneNumber(stmtGetString(stmt, next++));
    card.setPhone2Number(stmtGetString(stmt, next++));
    card.setFaxNumber(stmtGetString(stmt, next++));
    card.setEmail(stmtGetString(stmt, next++));
    card.setWebPage(stmtGetString(stmt, next++));
    card.setContact(stmtGetString(stmt, next++));
    card.setComments(stmtGetString(stmt, next++));
    card.setPatGroupId(stmtGetId(stmt, next++));
}

QString
QuasarDB::gltxCmd(const QString& table, const QString& id,
		  const QString& fields, const GltxSelect& where,
		  const QString& order)
{
    QString gltxTable = table;
    if (table != "gltx")
	gltxTable = "gltx join " + table + " on gltx.gltx_id = " + table +
	    "." + id;

    QString gltxFields = "number,reference_str,post_date,post_time,"
	"memo,station_id,employee_id,card_id,store_id,gltx.shift_id,"
	"link_id,printed,paid,gltx.amount";
    if (!fields.isEmpty()) gltxFields += "," + fields;

    return selectCmd(gltxTable, "gltx_id", gltxFields, where, order);
}

void
QuasarDB::selectGltx(Gltx& gltx, Stmt& stmt, int& next)
{
    gltx.setNumber(stmtGetString(stmt, next++));
    gltx.setReference(stmtGetString(stmt, next++));
    gltx.setPostDate(stmtGetDate(stmt, next++));
    gltx.setPostTime(stmtGetTime(stmt, next++));
    gltx.setMemo(stmtGetString(stmt, next++));
    gltx.setStationId(stmtGetId(stmt, next++));
    gltx.setEmployeeId(stmtGetId(stmt, next++));
    gltx.setCardId(stmtGetId(stmt, next++));
    gltx.setStoreId(stmtGetId(stmt, next++));
    gltx.setShiftId(stmtGetId(stmt, next++));
    gltx.setLinkId(stmtGetId(stmt, next++));
    gltx.setPrinted(stmtGetBool(stmt, next++));
    gltx.setPaid(stmtGetBool(stmt, next++));
    gltx.setAmount(stmtGetFixed(stmt, next++));
}

Id
QuasarDB::defaultBankId()
{
    Company company;
    lookup(company);
    Id id = company.bankAccount();

    // Verify bank id is valid
    if (id != INVALID_ID) {
	Account account;
	if (lookup(id, account)) {
	    if (account.type() != Account::Bank) id = INVALID_ID;
	    if (account.isHeader()) id = INVALID_ID;
	} else {
	    id = INVALID_ID;
	}
    }

    // If no bank id, use the first one in chart
    if (id == INVALID_ID) {
	AccountSelect conditions;
	conditions.type = Account::Bank;
	conditions.postingOnly = true;
	vector<Account> accounts;
	select(accounts, conditions);
	if (accounts.size() != 0)
	    id = accounts[0].id();
    }
    return id;
}

void
QuasarDB::setActive(DataObject& data, bool active)
{
    if (active) {
	data.setInactiveBy("");
	data.setInactiveOn(QDate());
    } else {
	data.setInactiveBy(_username);
	data.setInactiveOn(QDate::currentDate());
    }
}

bool
QuasarDB::lookup(Id object_id, DataObject& object)
{
    bool result = false;

    QString cmd = "select data_type from data_object where object_id=?";
    Stmt stmt(_connection, cmd);
    stmtSetId(stmt, object_id);
    if (!execute(stmt)) return false;

    if (stmt.next()) {
	object.setDataType(DataObject::DataType(stmtGetInt(stmt, 1)));
	result = true;
    }
    commit();

    return result;
}

bool
QuasarDB::linkTx(Id from_id, Id to_id)
{
    QString cmd = "update gltx set link_id = ? where gltx_id = ?";
    Stmt stmt(_connection, cmd);

    stmtSetId(stmt, from_id);
    stmtSetId(stmt, to_id);
    if (!execute(stmt)) return false;

    stmtSetId(stmt, to_id);
    stmtSetId(stmt, from_id);
    if (!execute(stmt)) return false;

    return true;
}

void
QuasarDB::invoiceTerms(vector<Id>& invoice_ids, vector<Id>& term_ids)
{
    invoice_ids.clear();
    term_ids.clear();

    QString cmd = "select invoice_id,term_id from invoice where "
	"term_id is not null";
    Stmt stmt(_connection, cmd);

    execute(stmt);
    while (stmt.next()) {
	invoice_ids.push_back(stmtGetId(stmt, 1));
	term_ids.push_back(stmtGetId(stmt, 2));
    }
}

void
QuasarDB::receiveTerms(vector<Id>& receive_ids, vector<Id>& term_ids)
{
    receive_ids.clear();
    term_ids.clear();

    QString cmd = "select receive_id,term_id from receive where "
	"term_id is not null";
    Stmt stmt(_connection, cmd);

    execute(stmt);
    while (stmt.next()) {
	receive_ids.push_back(stmtGetId(stmt, 1));
	term_ids.push_back(stmtGetId(stmt, 2));
    }
}
