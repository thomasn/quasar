// $Id: company.h,v 1.36 2005/03/13 22:12:54 bpepers Exp $
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

#ifndef COMPANY_H
#define COMPANY_H

#include "data_object.h"
#include "price.h"

struct RoundingRule {
    RoundingRule();

    QString endsIn;		// Price ends in
    fixed addAmt;		// Amount to add to the price

    bool operator==(const RoundingRule& rhs) const;
    bool operator!=(const RoundingRule& rhs) const;
};

class Company: public DataObject {
public:
    // Constructors and Destructor
    Company();
    ~Company();

    // Shift methods
    enum { BY_STATION, BY_EMPLOYEE };

    // Get methods
    QString name() const			{ return _name; }
    QString number() const			{ return _number; }
    QString street() const			{ return _street; }
    QString street2() const			{ return _street2; }
    QString city() const			{ return _city; }
    QString province() const			{ return _province; }
    QString country() const			{ return _country; }
    QString postal() const			{ return _postal; }
    QString phoneNumber() const			{ return _phone_num; }
    QString phone2Number() const		{ return _phone2_num; }
    QString faxNumber() const			{ return _fax_num; }
    QString email() const			{ return _email; }
    QString webPage() const			{ return _web_page; }
    Id defaultStore() const			{ return _store_id; }
    int shiftMethod() const			{ return _shift_method; }
    QDate startOfYear() const			{ return _year_start; }
    bool lastYearClosed() const			{ return _ly_closed; }
    QDate closeDate() const			{ return _close_date; }
    Id retainedEarnings() const			{ return _re_account; }
    Id historicalBalancing() const		{ return _hb_account; }
    Id bankAccount() const			{ return _bank_account; }
    Id customerAccount() const			{ return _customer_acct; }
    Id vendorAccount() const			{ return _vendor_acct; }
    Id customerTermsAcct() const		{ return _c_terms_acct; }
    Id vendorTermsAcct() const			{ return _v_terms_acct; }
    Id depositAccount() const			{ return _deposit_acct; }
    Id transferAccount() const			{ return _transfer_acct; }
    Id physicalAccount() const			{ return _physical_acct; }
    Id splitAccount() const			{ return _split_account; }
    Id chargeAccount() const			{ return _charge_acct; }
    Id overShortAccount() const			{ return _over_short_id; }
    QDate lastServiceCharge() const		{ return _last_sc; }
    Id safeStore() const			{ return _safe_store_id; }
    Id safeStation() const			{ return _safe_station_id; }
    Id safeEmployee() const			{ return _safe_employee_id; }

    // Set methods
    void setName(const QString& name)		{ _name = name; }
    void setNumber(const QString& number)	{ _number = number; }
    void setStreet(const QString& street)	{ _street = street; }
    void setStreet2(const QString& street)	{ _street2 = street; }
    void setCity(const QString& city)		{ _city = city; }
    void setProvince(const QString& prov)	{ _province = prov; }
    void setCountry(const QString& country)	{ _country = country; }
    void setPostal(const QString& postal)	{ _postal = postal; }
    void setPhoneNumber(const QString& phone)	{ _phone_num = phone; }
    void setPhone2Number(const QString& phone)	{ _phone2_num = phone; }
    void setFaxNumber(const QString& fax)	{ _fax_num = fax; }
    void setEmail(const QString& email)		{ _email = email; }
    void setWebPage(const QString& webPage)	{ _web_page = webPage; }
    void setDefaultStore(Id store_id)		{ _store_id = store_id; }
    void setShiftMethod(int method)		{ _shift_method = method; }
    void setStartOfYear(QDate date)		{ _year_start = date; }
    void setLastYearClosed(bool flag)		{ _ly_closed = flag; }
    void setCloseDate(QDate date)		{ _close_date = date; }
    void setRetainedEarnings(Id id)		{ _re_account = id; }
    void setHistoricalBalancing(Id id)		{ _hb_account = id; }
    void setBankAccount(Id id)			{ _bank_account = id; }
    void setCustomerAccount(Id id)		{ _customer_acct = id; }
    void setVendorAccount(Id id)		{ _vendor_acct = id; }
    void setCustomerTermsAcct(Id id)		{ _c_terms_acct = id; }
    void setVendorTermsAcct(Id id)		{ _v_terms_acct = id; }
    void setDepositAccount(Id id)		{ _deposit_acct = id; }
    void setTransferAccount(Id id)		{ _transfer_acct = id; }
    void setPhysicalAccount(Id id)		{ _physical_acct = id; }
    void setSplitAccount(Id id)			{ _split_account = id; }
    void setChargeAccount(Id id)		{ _charge_acct = id; }
    void setOverShortAccount(Id id)		{ _over_short_id = id; }
    void setLastServiceCharge(QDate date)	{ _last_sc = date; }
    void setSafeStore(Id store_id)		{ _safe_store_id = store_id; }
    void setSafeStation(Id id)			{ _safe_station_id = id; }
    void setSafeEmployee(Id id)			{ _safe_employee_id = id; }

    // Rounding info
    vector<RoundingRule>& rounding()		{ return _rounding; }
    const vector<RoundingRule>& rounding() const{ return _rounding; }

    // Address string
    QString address(bool includeExtra=false) const;

    // Round a price
    fixed priceRound(fixed price) const;
    Price priceRound(Price price) const;

    // Operations
    bool operator==(const Company& rhs) const;
    bool operator!=(const Company& rhs) const;

protected:
    QString _name;
    QString _number;
    QString _street;
    QString _street2;
    QString _city;
    QString _province;
    QString _country;
    QString _postal;
    QString _phone_num;
    QString _phone2_num;
    QString _fax_num;
    QString _email;
    QString _web_page;
    Id _store_id;			// Default store for company
    int _shift_method;			// Tenders by station or employee?
    QDate _year_start;			// Start of the current year
    bool _ly_closed;			// Last fiscal year is closed?
    QDate _close_date;			// Date ledger is closed (no postings)
    Id _re_account;			// Retained earnings account
    Id _hb_account;			// Historical balancing account
    Id _bank_account;			// Default bank account
    Id _customer_acct;			// Default receivables account
    Id _vendor_acct;			// Default payables account
    Id _c_terms_acct;			// Account for cust. terms discount
    Id _v_terms_acct;			// Account for vendor terms discount
    Id _deposit_acct;			// Account for bottle deposits
    Id _physical_acct;			// Account for physical adjustments
    Id _transfer_acct;			// Account for transfers
    Id _split_account;			// Account for ledger splits
    Id _charge_acct;			// Account for service charges
    Id _over_short_id;			// Account for tender over/short
    QDate _last_sc;			// Last date of service charges
    Id _safe_store_id;			// Store that safe is in
    Id _safe_station_id;		// Station to use for the safe
    Id _safe_employee_id;		// Employee to use for the safe
    vector<RoundingRule> _rounding;	// Price rounding
};

#endif // COMPANY_H
