// $Id: data_source.h,v 1.13 2004/08/09 23:44:53 bpepers Exp $
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

#ifndef DATA_SOURCE_H
#define DATA_SOURCE_H

#include <qvariant.h>
#include <qvaluevector.h>
#include <qdom.h>

class ReportInterp;

class DataSource {
public:
    DataSource();
    virtual ~DataSource();

    // Type name
    virtual QString type() const=0;

    // Create a clone
    virtual DataSource* clone() const=0;

    // Columns are unique names for each index into a row.
    QStringList columns;

    // Initialize the data source.  When done next should be called to
    // get the first row (or it will return false if there were no rows)
    virtual bool initialize(ReportInterp* interp)=0;

    // Move to next row
    virtual bool next(ReportInterp* interp)=0;

    // Get the value of a column in the current row.  A default
    // implementation is provided for those data sources that can
    // set the _currentRow variable.  The result of get before the
    // data source has been initialized, before the first working
    // next call, and after the last failing next call is undefined.
    virtual QVariant get(ReportInterp* interp, int column);

    // Convert from/to XML element
    virtual bool fromXML(QDomElement e)=0;
    virtual bool toXML(QDomElement& e) const=0;

protected:
    // Assignment only in subclasses
    DataSource& operator=(const DataSource& rhs);

    // Storage for current row of data
    QValueVector<QVariant> _currentRow;

private:
    // No copy constructor allowed
    DataSource(const DataSource& e);
};

#endif // DATA_SOURCE_H
