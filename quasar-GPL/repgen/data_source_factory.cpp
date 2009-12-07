// $Id: data_source_factory.cpp,v 1.6 2005/06/22 23:17:51 bpepers Exp $
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

#include "data_source_factory.h"
#include "code_data_source.h"
#include "filter_data_source.h"

template
QValueVector<DataSourceFactory::TypePair> DataSourceFactory::_types;

static DataSource*
createCode() { return new CodeDataSource(); }

static DataSource*
createFilter() { return new FilterDataSource(); }

template <>
void
DataSourceFactory::addBuiltinTypes()
{
    _types.push_back(TypePair("code", createCode));
    _types.push_back(TypePair("filter", createFilter));
}
