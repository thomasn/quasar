// $Id: quasar_tax.cpp,v 1.9 2005/05/13 23:54:44 bpepers Exp $
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
#include "object_cache.h"
#include "tax.h"

// Return the list of taxes given a tax that might be a group
void
QuasarDB::taxList(const Tax& tax, vector<Id>& tax_ids)
{
    tax_ids.clear();
    if (!tax.isGroup()) {
	tax_ids.push_back(tax.id());
    } else {
	for (unsigned int i = 0; i < tax.group_ids().size(); ++i)
	    tax_ids.push_back(tax.group_ids()[i]);
    }
}

fixed
QuasarDB::taxPercent(ObjectCache& cache, Id tax_id, vector<Id>& tax_ids,
		     vector<Id>& exempt_ids)
{
    // Check for exempt
    for (unsigned int i = 0; i < exempt_ids.size(); ++i)
	if (exempt_ids[i] == tax_id) return 0;

    Tax tax;
    if (!cache.findTax(tax_id, tax))
	return 0;

    fixed total_percent = 0;
    if (tax.isGroup()) {
	// Add up taxes in group
	for (unsigned int i = 0; i < tax.group_ids().size(); ++i) {
	    Id group_id = tax.group_ids()[i];
	    total_percent += taxPercent(cache, group_id, tax_ids, exempt_ids);
	}
    } else {
	// Tax is base rate plus tax on tax amounts
	total_percent = tax.rate();
	for (unsigned int i = 0; i < tax.tot_ids().size(); ++i) {
	    Id tot_id = tax.tot_ids()[i];

	    bool found = false;
	    for (unsigned int j = 0; j < exempt_ids.size(); ++j) {
		if (exempt_ids[j] == tot_id) {
		    found = true;
		    break;
		}
	    }
	    if (found) continue;

	    found = false;
	    for (unsigned int j = 0; j < tax_ids.size(); ++j) {
		if (tax_ids[j] == tot_id) {
		    found = true;
		    break;
		}
	    }
	    if (!found) continue;

	    fixed percent = taxPercent(cache, tot_id, tax_ids, exempt_ids);
	    total_percent += percent * tax.rate() / 100.0;
	}
    }

    return total_percent;
}

// Return the total tax rate taking into account tax groups
fixed
QuasarDB::taxRate(const Tax& tax)
{
    if (!tax.isGroup()) return tax.rate();

    fixed rate = 0.0;
    for (unsigned int i = 0; i < tax.group_ids().size(); ++i) {
	Tax child;
	lookup(tax.group_ids()[i], child);
	rate += taxRate(child);
    }

    return rate;
}

// Calculate the taxes adding on to a taxable amount.  Return the total tax
fixed
QuasarDB::calculateTaxOn(ObjectCache& cache, const Tax& tax, fixed taxable,
			 const Tax& exempt, vector<Id>& tax_ids,
			 vector<fixed>& tax_amts)
{
    tax_ids.clear();
    tax_amts.clear();
    if (tax.id() == INVALID_ID) return 0.0;

    vector<Id> exempt_ids;
    taxList(tax, tax_ids);
    taxList(exempt, exempt_ids);

    fixed tax_total = 0.0;
    for (unsigned int i = 0; i < tax_ids.size(); ++i) {
	Id tax_id = tax_ids[i];

	fixed rate = taxPercent(cache, tax_id, tax_ids, exempt_ids);
	fixed tax_amt = taxable * rate / 100.0;

	tax_amts.push_back(tax_amt);
	tax_total += tax_amt;
    }

    return tax_total;
}

// Simpler version where we don't care about the individual taxes
fixed
QuasarDB::calculateTaxOn(ObjectCache& cache, const Tax& tax, fixed taxable,
			 const Tax& exempt)
{
    vector<Id> ids;
    vector<fixed> amts;
    return calculateTaxOn(cache, tax, taxable, exempt, ids, amts);
}

// Even simpler version where we don't care about tax exempts
fixed
QuasarDB::calculateTaxOn(ObjectCache& cache, const Tax& tax, fixed taxable)
{
    Tax exempt;
    vector<Id> ids;
    vector<fixed> amts;
    return calculateTaxOn(cache, tax, taxable, exempt, ids, amts);
}

// Calculate the taxes removed from a total amount.  Return the total tax
fixed
QuasarDB::calculateTaxOff(ObjectCache& cache, const Tax& tax, fixed total,
			  vector<Id>& tax_ids, vector<fixed>& tax_amts)
{
    tax_ids.clear();
    tax_amts.clear();
    if (tax.id() == INVALID_ID) return 0.0;

    // Get list of actual taxes
    vector<Id> exempt_ids;
    taxList(tax, tax_ids);

    // First calculate the total tax using the total rate
    fixed total_rate = taxPercent(cache, tax.id(), tax_ids, exempt_ids);
    fixed base = total * 100.0 / (total_rate + 100.0);
    base.moneyRound();
    fixed total_tax = total - base;

    // Allocate total tax over groups.  This does rounding at each tax
    // which might mean that the total of the allocations doesn't add up
    // to the original tax total.  This is considered to be alright and
    // we just return back the total of the allocations.  This is done
    // to work around cases where two taxes are the same rate and the
    // total tax is an odd number (like 7%, 7% and a total of $1.01 in
    // taxes).  If this happens, it would give $0.50 for the first 7%
    // tax and $0.51 for the second which is pretty strange!  Better to
    // just have them both as $0.50 and use a total of $1.00 for the tax.
    fixed total_alloc = 0.0;
    for (unsigned int i = 0; i < tax_ids.size(); ++i) {
	Id tax_id = tax_ids[i];
	fixed rate = taxPercent(cache, tax_id, tax_ids, exempt_ids);

	fixed ratio = rate / total_rate;
	fixed tax_amt = ratio * total_tax;
	tax_amt.moneyRound();

	tax_amts.push_back(tax_amt);
	total_alloc += tax_amt;
    }

    return total_alloc;
}

// Simpler version where we don't care about the individual taxes
fixed
QuasarDB::calculateTaxOff(ObjectCache& cache, const Tax& tax, fixed total)
{
    vector<Id> ids;
    vector<fixed> amts;
    return calculateTaxOff(cache, tax, total, ids, amts);
}
