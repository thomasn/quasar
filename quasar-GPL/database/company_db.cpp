// $Id: company_db.cpp,v 1.13 2005/03/29 22:40:48 bpepers Exp $
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

#include "company.h"
#include "company_select.h"
#include "account.h"
#include "station.h"
#include "employee.h"
#include "store.h"

// Create a Company
bool
QuasarDB::create(Company& company)
{
    if (!validate(company)) return false;

    // Check there is only one company
    vector<Company> companies;
    select(companies, CompanySelect());
    if (companies.size() > 0)
	return error("Can only have one company defined");

    // Auto allocate company number
    if (company.number().stripWhiteSpace() == "#") {
	fixed number = uniqueNumber("company", "number");
	company.setNumber(number.toString());
    }

    QString cmd = insertCmd("company", "company_id", "name,number,street,"
			    "street2,city,province,country,postal,phone_num,"
			    "phone2_num,fax_num,email,web_page,store_id,"
			    "shift_method,year_start,ly_closed,close_date,"
			    "re_account,hb_account,bank_account,customer_acct,"
			    "vendor_acct,c_terms_acct,v_terms_acct,"
			    "deposit_acct,transfer_acct,physical_acct,"
			    "split_account,charge_acct,over_short_id,last_sc,"
			    "safe_store_id,safe_station_id,safe_employee_id");
    Stmt stmt(_connection, cmd);

    insertData(company, stmt);
    stmtSetString(stmt, company.name());
    stmtSetString(stmt, company.number());
    stmtSetString(stmt, company.street());
    stmtSetString(stmt, company.street2());
    stmtSetString(stmt, company.city());
    stmtSetString(stmt, company.province());
    stmtSetString(stmt, company.country());
    stmtSetString(stmt, company.postal());
    stmtSetString(stmt, company.phoneNumber());
    stmtSetString(stmt, company.phone2Number());
    stmtSetString(stmt, company.faxNumber());
    stmtSetString(stmt, company.email());
    stmtSetString(stmt, company.webPage());
    stmtSetId(stmt, company.defaultStore());
    stmtSetInt(stmt, company.shiftMethod());
    stmtSetDate(stmt, company.startOfYear());
    stmtSetBool(stmt, company.lastYearClosed());
    stmtSetDate(stmt, company.closeDate());
    stmtSetId(stmt, company.retainedEarnings());
    stmtSetId(stmt, company.historicalBalancing());
    stmtSetId(stmt, company.bankAccount());
    stmtSetId(stmt, company.customerAccount());
    stmtSetId(stmt, company.vendorAccount());
    stmtSetId(stmt, company.customerTermsAcct());
    stmtSetId(stmt, company.vendorTermsAcct());
    stmtSetId(stmt, company.depositAccount());
    stmtSetId(stmt, company.transferAccount());
    stmtSetId(stmt, company.physicalAccount());
    stmtSetId(stmt, company.splitAccount());
    stmtSetId(stmt, company.chargeAccount());
    stmtSetId(stmt, company.overShortAccount());
    stmtSetDate(stmt, company.lastServiceCharge());
    stmtSetId(stmt, company.safeStore());
    stmtSetId(stmt, company.safeStation());
    stmtSetId(stmt, company.safeEmployee());

    if (!execute(stmt)) return false;
    if (!sqlCreateLines(company)) return false;

    commit();
    dataSignal(DataEvent::Insert, company);
    return true;
}

// Delete a Company
bool
QuasarDB::remove(const Company& company)
{
    if (company.id() == INVALID_ID) return false;
    if (!sqlDeleteLines(company)) return false;
    if (!removeData(company, "company", "company_id")) return false;

    commit();
    dataSignal(DataEvent::Delete, company);
    return true;
}

// Update a Company
bool
QuasarDB::update(const Company& orig, Company& company)
{
    if (orig.id() == INVALID_ID || company.id() == INVALID_ID) return false;
    if (!validate(company)) return false;

    // Update the company table
    QString cmd = updateCmd("company", "company_id", "name,number,street,"
			    "street2,city,province,country,postal,phone_num,"
			    "phone2_num,fax_num,email,web_page,store_id,"
			    "shift_method,year_start,ly_closed,close_date,"
			    "re_account,hb_account,bank_account,customer_acct,"
			    "vendor_acct,c_terms_acct,v_terms_acct,"
			    "deposit_acct,transfer_acct,physical_acct,"
			    "split_account,charge_acct,over_short_id,last_sc,"
			    "safe_store_id,safe_station_id,safe_employee_id");
    Stmt stmt(_connection, cmd);

    updateData(orig, company, stmt);
    stmtSetString(stmt, company.name());
    stmtSetString(stmt, company.number());
    stmtSetString(stmt, company.street());
    stmtSetString(stmt, company.street2());
    stmtSetString(stmt, company.city());
    stmtSetString(stmt, company.province());
    stmtSetString(stmt, company.country());
    stmtSetString(stmt, company.postal());
    stmtSetString(stmt, company.phoneNumber());
    stmtSetString(stmt, company.phone2Number());
    stmtSetString(stmt, company.faxNumber());
    stmtSetString(stmt, company.email());
    stmtSetString(stmt, company.webPage());
    stmtSetId(stmt, company.defaultStore());
    stmtSetInt(stmt, company.shiftMethod());
    stmtSetDate(stmt, company.startOfYear());
    stmtSetBool(stmt, company.lastYearClosed());
    stmtSetDate(stmt, company.closeDate());
    stmtSetId(stmt, company.retainedEarnings());
    stmtSetId(stmt, company.historicalBalancing());
    stmtSetId(stmt, company.bankAccount());
    stmtSetId(stmt, company.customerAccount());
    stmtSetId(stmt, company.vendorAccount());
    stmtSetId(stmt, company.customerTermsAcct());
    stmtSetId(stmt, company.vendorTermsAcct());
    stmtSetId(stmt, company.depositAccount());
    stmtSetId(stmt, company.transferAccount());
    stmtSetId(stmt, company.physicalAccount());
    stmtSetId(stmt, company.splitAccount());
    stmtSetId(stmt, company.chargeAccount());
    stmtSetId(stmt, company.overShortAccount());
    stmtSetDate(stmt, company.lastServiceCharge());
    stmtSetId(stmt, company.safeStore());
    stmtSetId(stmt, company.safeStation());
    stmtSetId(stmt, company.safeEmployee());
    stmtSetId(stmt, orig.id());
    stmtSetInt(stmt, orig.version());

    if (!execute(stmt)) return false;
    if (stmt.getUpdateCount() != 1)
	return error("Data has been changed by another user");
    if (!sqlDeleteLines(orig)) return false;
    if (!sqlCreateLines(company)) return false;

    commit();
    dataSignal(DataEvent::Update, orig);
    return true;
}

// Lookup a Company.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Id company_id, Company& company)
{
    if (company_id == INVALID_ID) return false;
    CompanySelect conditions;
    vector<Company> companies;

    conditions.id = company_id;
    if (!select(companies, conditions)) return false;
    if (companies.size() != 1) return false;

    company = companies[0];
    return true;
}

// Lookup a Company by its number
bool
QuasarDB::lookup(const QString& number, Company& company)
{
    if (number.isEmpty()) return false;
    CompanySelect conditions;
    vector<Company> companies;

    conditions.number = number;
    if (!select(companies, conditions)) return false;
    if (companies.size() != 1) return false;

    company = companies[0];
    return true;
}

// Returns a vector of Companys.  Returns companys sorted by name.
bool
QuasarDB::select(vector<Company>& companies, const CompanySelect& conditions)
{
    companies.clear();

    QString cmd = selectCmd("company", "company_id", "name,number,street,"
			    "street2,city,province,country,postal,phone_num,"
			    "phone2_num,fax_num,email,web_page,store_id,"
			    "shift_method,year_start,ly_closed,close_date,"
			    "re_account,hb_account,bank_account,customer_acct,"
			    "vendor_acct,c_terms_acct,v_terms_acct,"
			    "deposit_acct,transfer_acct,physical_acct,"
			    "split_account,charge_acct,over_short_id,last_sc,"
			    "safe_store_id,safe_station_id,safe_employee_id",
			    conditions);
    Stmt stmt(_connection, cmd);

    if (!execute(stmt)) return false;
    while (stmt.next()) {
	Company company;
	int next = 1;
	selectData(company, stmt, next);
	company.setName(stmtGetString(stmt, next++));
	company.setNumber(stmtGetString(stmt, next++));
	company.setStreet(stmtGetString(stmt, next++));
	company.setStreet2(stmtGetString(stmt, next++));
	company.setCity(stmtGetString(stmt, next++));
	company.setProvince(stmtGetString(stmt, next++));
	company.setCountry(stmtGetString(stmt, next++));
	company.setPostal(stmtGetString(stmt, next++));
	company.setPhoneNumber(stmtGetString(stmt, next++));
	company.setPhone2Number(stmtGetString(stmt, next++));
	company.setFaxNumber(stmtGetString(stmt, next++));
	company.setEmail(stmtGetString(stmt, next++));
	company.setWebPage(stmtGetString(stmt, next++));
	company.setDefaultStore(stmtGetId(stmt, next++));
	company.setShiftMethod(stmtGetInt(stmt, next++));
	company.setStartOfYear(stmtGetDate(stmt, next++));
	company.setLastYearClosed(stmtGetBool(stmt, next++));
	company.setCloseDate(stmtGetDate(stmt, next++));
	company.setRetainedEarnings(stmtGetId(stmt, next++));
	company.setHistoricalBalancing(stmtGetId(stmt, next++));
	company.setBankAccount(stmtGetId(stmt, next++));
	company.setCustomerAccount(stmtGetId(stmt, next++));
	company.setVendorAccount(stmtGetId(stmt, next++));
	company.setCustomerTermsAcct(stmtGetId(stmt, next++));
	company.setVendorTermsAcct(stmtGetId(stmt, next++));
	company.setDepositAccount(stmtGetId(stmt, next++));
	company.setTransferAccount(stmtGetId(stmt, next++));
	company.setPhysicalAccount(stmtGetId(stmt, next++));
	company.setSplitAccount(stmtGetId(stmt, next++));
	company.setChargeAccount(stmtGetId(stmt, next++));
	company.setOverShortAccount(stmtGetId(stmt, next++));
	company.setLastServiceCharge(stmtGetDate(stmt, next++));
	company.setSafeStore(stmtGetId(stmt, next++));
	company.setSafeStation(stmtGetId(stmt, next++));
	company.setSafeEmployee(stmtGetId(stmt, next++));
	companies.push_back(company);
    }

    QString cmd1 = "select ends_in,add_amt from company_pround where "
	"company_id = ? order by seq_num";
    Stmt stmt1(_connection, cmd1);

    for (unsigned int i = 0; i < companies.size(); ++i) {
	Id company_id = companies[i].id();

	stmtSetId(stmt1, company_id);
	if (!execute(stmt1)) return false;
	while (stmt1.next()) {
	    RoundingRule rounding;
	    int next = 1;
	    rounding.endsIn = stmtGetString(stmt1, next++);
	    rounding.addAmt = stmtGetFixed(stmt1, next++);
	    companies[i].rounding().push_back(rounding);
	}
    }

    commit();
    return true;
}

// Validate company information
bool
QuasarDB::validate(const Company& company)
{
    if (!validate((DataObject&)company)) return false;

    if (company.name().stripWhiteSpace().isEmpty())
	return error("Blank company name");

    if (company.defaultStore() != INVALID_ID) {
	Store store;
	if (!lookup(company.defaultStore(), store))
	    return error("Default store is invalid");
	if (store.companyId() != company.id())
	    return error("Default store is not for this company");
    }

    if (company.retainedEarnings() != INVALID_ID) {
	Account account;
	lookup(company.retainedEarnings(), account);
	if (account.type() != Account::Equity)
	    return error("Retained earnings must be an equity account");
	if (account.isHeader())
	    return error("Retained earnings can't be a header account");
    }

    if (company.historicalBalancing() != INVALID_ID) {
	Account account;
	lookup(company.historicalBalancing(), account);
	if (account.type() != Account::Equity)
	    return error("Historical balancing must be an equity account");
	if (account.isHeader())
	    return error("Historical balancing can't be a header account");
    }

    if (company.bankAccount() != INVALID_ID) {
	Account account;
	lookup(company.bankAccount(), account);
	if (account.type() != Account::Bank)
	    return error("Banking account must be a bank account");
	if (account.isHeader())
	    return error("Banking account can't be a header account");
    }

    if (company.customerAccount() != INVALID_ID) {
	Account account;
	lookup(company.customerAccount(), account);
	if (account.type() != Account::AR)
	    return error("Customer account must be a AR account");
	if (account.isHeader())
	    return error("Customer account can't be a header account");
    }

    if (company.vendorAccount() != INVALID_ID) {
	Account account;
	lookup(company.vendorAccount(), account);
	if (account.type() != Account::AP)
	    return error("Vendor account must be a AP account");
	if (account.isHeader())
	    return error("Vendor account can't be a header account");
    }

    if (company.customerTermsAcct() != INVALID_ID) {
	Account account;
	lookup(company.customerTermsAcct(), account);
	if (account.isHeader())
	    return error("Terms account can't be a header account");
    }

    if (company.vendorTermsAcct() != INVALID_ID) {
	Account account;
	lookup(company.vendorTermsAcct(), account);
	if (account.isHeader())
	    return error("Terms account can't be a header account");
    }

    if (company.depositAccount() != INVALID_ID) {
	Account account;
	lookup(company.depositAccount(), account);
	if (account.isHeader())
	    return error("Container deposit can't be a header account");
    }

    if (company.transferAccount() != INVALID_ID) {
	Account account;
	lookup(company.transferAccount(), account);
	if (account.isHeader())
	    return error("Transfer account can't be a header account");
    }

    if (company.splitAccount() != INVALID_ID) {
	Account account;
	lookup(company.splitAccount(), account);
	if (account.isHeader())
	    return error("Split account can't be a header account");
    }

    if (company.chargeAccount() != INVALID_ID) {
	Account account;
	lookup(company.chargeAccount(), account);
	if (account.isHeader())
	    return error("Charge account can't be a header account");
    }

    if (company.overShortAccount() != INVALID_ID) {
	Account account;
	lookup(company.overShortAccount(), account);
	if (account.isHeader())
	    return error("Over/short account can't be a header account");
    }

    if (company.shiftMethod() == Company::BY_STATION) {
	if (company.safeStation() != INVALID_ID) {
	    Station station;
	    if (!lookup(company.safeStation(), station))
		return error("Safe is not a valid station");
	}
	if (company.safeEmployee() != INVALID_ID)
	    return error("Can't have safe employee set");
    }

    if (company.shiftMethod() == Company::BY_EMPLOYEE) {
	if (company.safeEmployee() != INVALID_ID) {
	    Employee employee;
	    if (!lookup(company.safeEmployee(), employee))
		return error("Safe is not a valid employee");
	}
	if (company.safeStation() != INVALID_ID)
	    return error("Can't have safe station set");
    }

    return true;
}

// Lookup a Company.  This is just a shortcut to using select.
bool
QuasarDB::lookup(Company& company)
{
    vector<Company> companies;
    select(companies, CompanySelect());
    if (companies.size() == 0)
	return false;

    company = companies[0];
    return true;
}

bool
QuasarDB::sqlCreateLines(const Company& company)
{
    QString cmd = insertText("company_pround", "company_id", "seq_num,"
			     "ends_in,add_amt");
    Stmt stmt(_connection, cmd);

    for (unsigned int i = 0; i < company.rounding().size(); ++i) {
	stmtSetId(stmt, company.id());
	stmtSetInt(stmt, i);
	stmtSetString(stmt, company.rounding()[i].endsIn);
	stmtSetFixed(stmt, company.rounding()[i].addAmt);
	if (!execute(stmt)) return false;
    }

    return true;
}

bool
QuasarDB::sqlDeleteLines(const Company& company)
{
    return remove(company, "company_pround", "company_id");
}
