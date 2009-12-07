// $Id: recurring.h,v 1.7 2005/01/30 00:51:13 bpepers Exp $
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

#ifndef RECURRING_H
#define RECURRING_H

#include "data_object.h"

class Recurring: public DataObject {
public:
    // Constructors and Destructor
    Recurring();
    ~Recurring();

    // Frequencies
    enum { Daily, Weekly, BiWeekly, SemiMonthly, Monthly, Quarterly,
	   SemiAnually, Anually, Other };

    // Get methods
    Id gltxId()				const { return _gltx_id; }
    QString description()		const { return _desc; }
    int frequency()			const { return _frequency; }
    int day1()				const { return _day1; }
    int day2()				const { return _day2; }
    int maxPostings()			const { return _max_post; }
    QDate lastPosted()			const { return _last_posted; }
    int postingCount()			const { return _post_count; }
    Id cardGroup()			const { return _card_group; }

    // Set methods
    void setGltxId(Id gltx_id)			{ _gltx_id = gltx_id; }
    void setDescription(const QString& desc)	{ _desc = desc; }
    void setFrequency(int type)			{ _frequency = type; }
    void setDay1(int day)			{ _day1 = day; }
    void setDay2(int day)			{ _day2 = day; }
    void setDays(int d1, int d2)		{ _day1 = d1; _day2 = d2; }
    void setMaxPostings(int count)		{ _max_post = count; }
    void setLastPosted(QDate date)		{ _last_posted = date; }
    void setPostingCount(int count)		{ _post_count = count; }
    void setCardGroup(Id card_group)		{ _card_group = card_group; }

    // Calculated
    int overdueDays() const;
    QDate nextDue() const;

    // Operations
    bool operator==(const Recurring& rhs) const;
    bool operator!=(const Recurring& rhs) const;

protected:
    Id _gltx_id;		// Transaction to recurr
    QString _desc;		// Description
    int _frequency;		// How often to recurr
    int _day1;			// First day for bi-monthly
    int _day2;			// Second day for bi-monthly
    int _max_post;		// Maximum times to post
    QDate _last_posted;		// When was last posted (used for calculations)
    int _post_count;		// How many times posted?
    Id _card_group;		// Group to apply to
};

#endif // RECURRING_H
