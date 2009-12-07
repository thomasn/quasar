// $Id: report_param_type.h,v 1.4 2004/10/12 05:46:26 bpepers Exp $
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

#ifndef REPORT_PARAM_TYPE_H
#define REPORT_PARAM_TYPE_H

#include "report_param.h"
#include <qvariant.h>

class QWidget;

class ReportParamType {
public:
    ReportParamType();
    virtual ~ReportParamType();

    // Type name
    virtual QString type() const=0;

    // Create a clone
    virtual ReportParamType* clone() const=0;

    // Set param
    void setParam(const ReportParam& param) { _param = param; }

    // Get the widget for data entry
    virtual QWidget* getWidget(QWidget* parent)=0;

    // Get/set the value from the widget (internal values)
    virtual QVariant getValue(QWidget* widget)=0;
    virtual void setValue(QWidget* widget, const QVariant& value)=0;

    // Set the value as text
    virtual void setText(QWidget* widget, const QString& text);

    // Validate the current widget contents
    virtual bool validate(QWidget* widget)=0;

    // Convert from external text format to internal QVariant
    virtual bool convert(const QString& text, QVariant& value);

    // Set internal variables that convert will use
    static void setInternal(const QString& key, const QVariant& value);

protected:
    // Link to report parameter definition
    ReportParam _param;

    // Internal variables
    static QMap<QString,QVariant> _internal;

private:
    // No copy constructor allowed
    ReportParamType(const ReportParamType& e);
};

#endif // REPORT_PARAM_TYPE_H
