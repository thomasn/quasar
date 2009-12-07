// $Id: factory.h,v 1.4 2005/01/17 22:27:46 bpepers Exp $
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

#ifndef FACTORY_H
#define FACTORY_H

#include <qpair.h>
#include <qstringlist.h>
#include <qvaluevector.h>

template<class T> class Factory {
public:
    // Create type function
    typedef T* (*CreateFunc)();

    // Return list of types
    static QStringList types() {
	if (_types.size() == 0) addBuiltinTypes();

	QStringList types;
	for (unsigned int i = 0; i < _types.size(); ++i)
	    types.push_back(_types[i].first);

	return types;
    }

    // Create an item based on a type
    static T* create(const QString& type) {
	if (_types.size() == 0) addBuiltinTypes();

	for (unsigned int i = 0; i < _types.size(); ++i) {
	    if (_types[i].first.lower() == type.lower())
		return _types[i].second();
	}

	qWarning("Can't create type: " + type);
	return NULL;
    }

    // Register a new type
    static bool registerType(const QString& type, CreateFunc func) {
	if (_types.size() == 0) addBuiltinTypes();

	// Replace existing type if it exists
	for (unsigned int i = 0; i < _types.size(); ++i) {
	    if (_types[i].first.lower() == type.lower()) {
		_types[i].second = func;
		return true;
	    }
	}

	TypePair entry(type, func);
	_types.push_back(entry);
	return true;
    }

    // Remove registration of a type
    static bool unregisterType(const QString& type) {
	if (_types.size() == 0) addBuiltinTypes();

	// Find type and remove
	for (unsigned int i = 0; i < _types.size(); ++i) {
	    if (_types[i].first.lower() == type.lower()) {
		_types.erase(_types.begin() + i);
		return true;
	    }
	}
	return false;
    }

    // Check if type exists
    static bool typeExists(const QString& type) {
	if (_types.size() == 0) addBuiltinTypes();

	// Find type
	for (unsigned int i = 0; i < _types.size(); ++i) {
	    if (_types[i].first.lower() == type.lower()) {
		return true;
	    }
	}
	return false;
    }

private:
    // List of type names and create functions
    typedef QPair<QString,CreateFunc> TypePair;
    static QValueVector<TypePair> _types;

    // Fill in types with builtins
    static void addBuiltinTypes();
};

template<class T>
QValueVector<typename Factory<T>::TypePair> Factory<T>::_types;

#endif // FACTORY_H
