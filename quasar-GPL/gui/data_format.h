// $Id: data_format.h,v 1.3 2004/01/31 01:50:31 arandell Exp $
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

#ifndef DATA_FORMAT_H
#define DATA_FORMAT_H

#include <qdialog.h>
#include "date_valcon.h"
#include "time_valcon.h"
#include "money_valcon.h"
#include "percent_valcon.h"

class ComboBox;
class LineEdit;
class IntegerEdit;
class QCheckBox;

class DataFormat: public QDialog {
    Q_OBJECT
public:
    DataFormat(QWidget* parent=0, const char* name=0, bool modal=true);
    virtual ~DataFormat();

    // Set existing formats
    void setDateFormat(const QString& format);
    void setTimeFormat(const QString& format);
    void setNumberFormat(const QString& format);
    void setMoneyFormat(const QString& format);
    void setPercentFormat(const QString& format);

    // Get new formats
    QString getDateFormat() const;
    QString getTimeFormat() const;
    QString getNumberFormat() const;
    QString getMoneyFormat() const;
    QString getPercentFormat() const;

protected slots:
    void slotDateChanged();
    void slotTimeChanged();
    void slotNumberChanged();
    void slotMoneyChanged();
    void slotPercentChanged();

private:
    // Date format
    ComboBox* _dateOrder;
    LineEdit* _dateSep;
    QCheckBox* _datePadDay;
    QCheckBox* _datePadMonth;
    QCheckBox* _dateTextMonth;
    QCheckBox* _dateShortMonth;
    QCheckBox* _dateShortYear;
    LineEdit* _dateExample;

    // Time format
    LineEdit* _timeSep;
    QCheckBox* _timePadHour;
    QCheckBox* _timePadMinute;
    QCheckBox* _timeShowSeconds;
    QCheckBox* _timeUseAmPm;
    LineEdit* _timeAmText;
    LineEdit* _timePmText;
    LineEdit* _timeExample;

    // Number format
    LineEdit* _numPosPrefix;
    LineEdit* _numPosSuffix;
    LineEdit* _numNegPrefix;
    LineEdit* _numNegSuffix;
    IntegerEdit* _numMaxInt;
    IntegerEdit* _numMinInt;
    IntegerEdit* _numMaxDec;
    IntegerEdit* _numMinDec;
    LineEdit* _numDecSep;
    IntegerEdit* _numGroupBy;
    LineEdit* _numGroupSep;
    LineEdit* _numExample;

    // Money format
    LineEdit* _monPosPrefix;
    LineEdit* _monPosSuffix;
    LineEdit* _monNegPrefix;
    LineEdit* _monNegSuffix;
    IntegerEdit* _monMaxInt;
    IntegerEdit* _monMinInt;
    IntegerEdit* _monMaxDec;
    IntegerEdit* _monMinDec;
    LineEdit* _monDecSep;
    IntegerEdit* _monGroupBy;
    LineEdit* _monGroupSep;
    LineEdit* _monExample;

    // Percent format
    LineEdit* _perPosPrefix;
    LineEdit* _perPosSuffix;
    LineEdit* _perNegPrefix;
    LineEdit* _perNegSuffix;
    IntegerEdit* _perMaxInt;
    IntegerEdit* _perMinInt;
    IntegerEdit* _perMaxDec;
    IntegerEdit* _perMinDec;
    LineEdit* _perDecSep;
    IntegerEdit* _perGroupBy;
    LineEdit* _perGroupSep;
    LineEdit* _perExample;
};

#endif // DATA_FORMAT_H
