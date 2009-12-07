// $Id: lookup_edit.cpp,v 1.38 2005/03/13 22:30:14 bpepers Exp $
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

#include "lookup_edit.h"
#include <qapplication.h>

LookupEdit::LookupEdit(QWidget* parent, const char* name)
    : LineEdit(parent, name)
{
    _lookup = NULL;
    _id = INVALID_ID;
    addPopup(Key_F9, tr("lookup"));
    addPopup(Key_F10, tr("edit"));
    addPopup(Key_F11, tr("new"));
}

LookupEdit::LookupEdit(LookupWindow* lookup, QWidget* parent, const char* name)
    : LineEdit(parent, name)
{
    _lookup = lookup;
    _id = INVALID_ID;
    addPopup(Key_F9, tr("lookup"));
    if (lookup->allowEdit())
	addPopup(Key_F10, tr("edit"));
    if (lookup->allowNew())
	addPopup(Key_F11, tr("new"));
}

LookupEdit::~LookupEdit()
{
}

Variant
LookupEdit::value()
{
    Id id = getId();
    if (id == INVALID_ID)
	return Variant();
    return id;
}

void
LookupEdit::setValue(Variant value)
{
    Id id;

    switch (value.type()) {
    case Variant::T_NULL:
	QLineEdit::setText("");
	_id = INVALID_ID;
	_changed = false;
	_valid = true;
	break;
    case Variant::ID:
	setId(value.toId());
	break;
    case Variant::PLU:
	QLineEdit::setText(value.toPlu().number());
	_id = value.toPlu().itemId();
	_changed = false;
	_valid = true;
	break;
    default:
	qWarning("Invalid type for LookupEdit: %s", value.typeName());
	setId(INVALID_ID);
    }
}

Id
LookupEdit::getId()
{
    if (!valid()) return INVALID_ID;
    QString txt = text().simplifyWhiteSpace();
    if (txt == "") return INVALID_ID;
    if (_id != INVALID_ID) return _id;

    vector<DataPair> data = _lookup->lookupByText(txt);
    if (data.size() == 0) return INVALID_ID;
    if (data.size() > 1 && data[0].second != txt) {
	for (unsigned int i = 0; i < data.size(); ++i)
	    if (data[i].second == txt) return data[i].first;
	return INVALID_ID;
    }

    return data[0].first;
}

void
LookupEdit::setId(Id id)
{
    QString text = _lookup->lookupById(id);
    QLineEdit::setText(text);
    _id = id;
    _changed = false;
    _valid = true;
}

void
LookupEdit::created(Id id)
{
    setId(id);
    emit validData();
}

bool
LookupEdit::doValidation(Reason reason)
{
    vector<DataPair> data;
    int size = _lookup->matchCount(text());
    _id = INVALID_ID;

    // Check for the single match case first with extra case in case
    // we get a different count of matches from the lookupByText call
    if (size == 1) {
	data = _lookup->lookupByText(text());
	size = data.size();
	if (size == 1) {
	    QLineEdit::setText(data[0].second);
	    _id = data[0].first;
	    return true;
	}
    }

    // If no matches then perhaps do a popup but otherwise return false
    if (size == 0) {
	if (reason == Tab || reason == Return)
	    return popup(Key_F9);
	return false;
    }

    // Try to expand the text typed in up till the first non-unique part
    // but only if there weren't too many matches.  For example if the user
    // types in "B" and the matches are "Brad" and "Brian" then we extend
    // the text to "Br".  But if the user typed "0" into an inventory with
    // 10,000 items all UPC codes starting with "0" then don't bother
    // trying them all.
    if (size < 20) {
	data = _lookup->lookupByText(text());
	size = data.size();
	uint length = text().length();
	while (true) {
	    bool allSame = true;
	    bool foundLonger = false;
	    QChar match = data[0].second.at(length);
	    for (int line = 1; line < size; ++line) {
		if (data[line].second.at(length) != match)
		    allSame = false;
		if (data[line].second.length() > length)
		    foundLonger = true;
	    }

	    if (!allSame) break;
	    if (!foundLonger) break;
	    ++length;
	}

	// If could extend then set the text and return
	if (length > text().length()) {
	    QLineEdit::setText(data[0].second.left(length));
	    return false;
	}

	// Count how many matches there are and the last index
	int foundCount = 0;
	int foundIndex = -1;
	for (int i = 0; i < size; ++i) {
	    if (data[i].second == text()) {
		++foundCount;
		foundIndex = i;
	    }
	}

	// If there is exactly one match, then use it
	if (foundCount == 1) {
	    QLineEdit::setText(data[foundIndex].second);
	    _id = data[foundIndex].first;
	    return true;
	}
    }

    // Finally try a popup if good reason to do so
    if (reason == Tab || reason == Return)
	return popup(Key_F9);

    return false;
}

bool
LookupEdit::popup(QKeySequence key)
{
    bool result = true;

    if (key == QKeySequence(Key_F9)) {
        _lookup->clearLines();
	_lookup->setText(text());
	if (!text().isEmpty())
	    _lookup->refresh();

	result = _lookup->exec();
	if (result) {
	    setId(_lookup->getId());
	    emit validData();
	}
    } else if (key == QKeySequence(Key_F10)) {
        if (_id != INVALID_ID && _lookup->allowEdit())
	    _lookup->slotEdit(_id);
    } else if (key == QKeySequence(Key_F11)) {
	if (_lookup->allowNew()) {
	    QWidget* screen = _lookup->slotNew();
	    if (screen != NULL)
		connect(screen, SIGNAL(created(Id)), SLOT(created(Id)));
	}
    } else {
        qDebug("Unknown popup key: " + QString(key));
    }

    return result;
}
