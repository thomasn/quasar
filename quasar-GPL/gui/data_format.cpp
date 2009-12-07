// $Id: data_format.cpp,v 1.8 2005/02/24 02:48:27 bpepers Exp $
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

#include "data_format.h"
#include "images.h"
#include "integer_edit.h"
#include "combo_box.h"

#include <qapplication.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>

DataFormat::DataFormat(QWidget* parent, const char* name, bool modal)
    : QDialog(parent, name, modal)
{
#ifndef _WIN32
    setIcon(QPixmap(window_icon_xpm));
#endif

    QTabWidget* tabs = new QTabWidget(this);
    QFrame* date = new QFrame(tabs);
    QFrame* time = new QFrame(tabs);
    QFrame* number = new QFrame(tabs);
    QFrame* money = new QFrame(tabs);
    QFrame* percent = new QFrame(tabs);
    tabs->addTab(date, tr("Date"));
    tabs->addTab(time, tr("Time"));
    tabs->addTab(number, tr("Number"));
    tabs->addTab(money, tr("Money"));
    tabs->addTab(percent, tr("Percent"));

    QLabel* dateOrderLabel = new QLabel(tr("Field Order:"), date);
    _dateOrder = new ComboBox(false, date);
    _dateOrder->insertItem("dmy");
    _dateOrder->insertItem("mdy");
    _dateOrder->insertItem("ymd");
    dateOrderLabel->setBuddy(_dateOrder);

    QLabel* dateSepLabel = new QLabel(tr("Separator:"), date);
    _dateSep = new LineEdit(3, date);
    dateSepLabel->setBuddy(_dateSep);

    _dateTextMonth = new QCheckBox(tr("Text month?"), date);
    _datePadDay = new QCheckBox(tr("Zero pad day?"), date);
    _datePadMonth = new QCheckBox(tr("Zero pad month?"), date);
    _dateShortMonth = new QCheckBox(tr("Short month?"), date);
    _dateShortYear = new QCheckBox(tr("Short year?"), date);

    QLabel* dateExampleLabel = new QLabel(tr("Example:"), date);
    _dateExample = new LineEdit(20, date);
    _dateExample->setFocusPolicy(NoFocus);

    connect(_dateOrder, SIGNAL(activated(int)), SLOT(slotDateChanged()));
    connect(_dateSep, SIGNAL(textChanged(const QString&)),
	    SLOT(slotDateChanged()));
    connect(_datePadDay, SIGNAL(clicked()), SLOT(slotDateChanged()));
    connect(_datePadMonth, SIGNAL(clicked()), SLOT(slotDateChanged()));
    connect(_dateTextMonth, SIGNAL(clicked()), SLOT(slotDateChanged()));
    connect(_dateShortMonth, SIGNAL(clicked()), SLOT(slotDateChanged()));
    connect(_dateShortYear, SIGNAL(clicked()), SLOT(slotDateChanged()));

    QGridLayout* dateGrid = new QGridLayout(date);
    dateGrid->setSpacing(3);
    dateGrid->setMargin(3);
    dateGrid->setRowStretch(4, 1);
    dateGrid->addRowSpacing(4, 10);
    dateGrid->setColStretch(2, 1);
    dateGrid->addColSpacing(2, 20);
    dateGrid->addWidget(dateOrderLabel, 0, 0);
    dateGrid->addWidget(_dateOrder, 0, 1, AlignLeft | AlignVCenter);
    dateGrid->addWidget(dateSepLabel, 1, 0);
    dateGrid->addWidget(_dateSep, 1, 1, AlignLeft | AlignVCenter);
    dateGrid->addMultiCellWidget(_dateTextMonth, 3, 3, 0, 1);
    dateGrid->addWidget(_datePadDay, 0, 3);
    dateGrid->addWidget(_datePadMonth, 1, 3);
    dateGrid->addWidget(_dateShortMonth, 2, 3);
    dateGrid->addWidget(_dateShortYear, 3, 3);
    dateGrid->addWidget(dateExampleLabel, 5, 0);
    dateGrid->addMultiCellWidget(_dateExample, 5, 5, 1, 3,
				 AlignLeft | AlignVCenter);

    QLabel* timeSepLabel = new QLabel(tr("Separator:"), time);
    _timeSep = new LineEdit(3, time);
    timeSepLabel->setBuddy(_timeSep);

    _timePadHour = new QCheckBox(tr("Zero pad hour?"), time);
    _timeShowSeconds = new QCheckBox(tr("Show seconds?"), time);
    _timePadMinute = new QCheckBox(tr("Zero pad minute?"), time);
    _timeUseAmPm = new QCheckBox(tr("Use AM/PM?"), time);

    QLabel* timeAmTextLabel = new QLabel(tr("AM Text:"), time);
    _timeAmText = new LineEdit(10, time);
    timeAmTextLabel->setBuddy(_timeAmText);

    QLabel* timePmTextLabel = new QLabel(tr("PM Text:"), time);
    _timePmText = new LineEdit(10, time);
    timePmTextLabel->setBuddy(_timePmText);

    QLabel* timeExampleLabel = new QLabel(tr("Example:"), time);
    _timeExample = new LineEdit(20, time);
    _timeExample->setFocusPolicy(NoFocus);

    connect(_timeSep, SIGNAL(textChanged(const QString&)),
	    SLOT(slotTimeChanged()));
    connect(_timePadHour, SIGNAL(clicked()), SLOT(slotTimeChanged()));
    connect(_timePadMinute, SIGNAL(clicked()), SLOT(slotTimeChanged()));
    connect(_timeShowSeconds, SIGNAL(clicked()), SLOT(slotTimeChanged()));
    connect(_timeUseAmPm, SIGNAL(clicked()), SLOT(slotTimeChanged()));
    connect(_timeAmText, SIGNAL(textChanged(const QString&)),
	    SLOT(slotTimeChanged()));
    connect(_timePmText, SIGNAL(textChanged(const QString&)),
	    SLOT(slotTimeChanged()));

    QGridLayout* timeGrid = new QGridLayout(time);
    timeGrid->setSpacing(3);
    timeGrid->setMargin(3);
    timeGrid->setRowStretch(4, 1);
    timeGrid->addRowSpacing(4, 10);
    timeGrid->setColStretch(2, 1);
    timeGrid->addColSpacing(2, 20);
    timeGrid->addWidget(timeSepLabel, 0, 0);
    timeGrid->addWidget(_timeSep, 0, 1, AlignLeft | AlignVCenter);
    timeGrid->addMultiCellWidget(_timePadHour, 0, 0, 3, 4);
    timeGrid->addMultiCellWidget(_timeShowSeconds, 1, 1, 0, 1);
    timeGrid->addMultiCellWidget(_timePadMinute, 1, 1, 3, 4);
    timeGrid->addMultiCellWidget(_timeUseAmPm, 2, 2, 0, 1);
    timeGrid->addWidget(timeAmTextLabel, 2, 3);
    timeGrid->addWidget(_timeAmText, 2, 4, AlignLeft | AlignVCenter);
    timeGrid->addWidget(timePmTextLabel, 3, 3);
    timeGrid->addWidget(_timePmText, 3, 4, AlignLeft | AlignVCenter);
    timeGrid->addWidget(timeExampleLabel, 5, 0);
    timeGrid->addMultiCellWidget(_timeExample, 5, 5, 1, 4,
				 AlignLeft | AlignVCenter);

    QLabel* numPosPrefixLabel = new QLabel(tr("Positive Prefix:"), number);
    _numPosPrefix = new LineEdit(6, number);
    connect(_numPosPrefix, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numPosSuffixLabel = new QLabel(tr("Positive Suffix:"), number);
    _numPosSuffix = new LineEdit(6, number);
    connect(_numPosSuffix, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numNegPrefixLabel = new QLabel(tr("Negative Prefix:"), number);
    _numNegPrefix = new LineEdit(6, number);
    connect(_numNegPrefix, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numNegSuffixLabel = new QLabel(tr("Negative Suffix:"), number);
    _numNegSuffix = new LineEdit(6, number);
    connect(_numNegSuffix, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numMaxIntLabel = new QLabel(tr("Max Int. Digits:"), number);
    _numMaxInt = new IntegerEdit(6, number);
    connect(_numMaxInt, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numMinIntLabel = new QLabel(tr("Min Int. Digits:"), number);
    _numMinInt = new IntegerEdit(6, number);
    connect(_numMinInt, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numMaxDecLabel = new QLabel(tr("Max Dec. Digits:"), number);
    _numMaxDec = new IntegerEdit(6, number);
    connect(_numMaxDec, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numMinDecLabel = new QLabel(tr("Min Dec. Digits:"), number);
    _numMinDec = new IntegerEdit(6, number);
    connect(_numMinDec, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numDecSepLabel = new QLabel(tr("Decimal Separator:"), number);
    _numDecSep = new LineEdit(6, number);
    connect(_numDecSep, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numGroupByLabel = new QLabel(tr("Group By:"), number);
    _numGroupBy = new IntegerEdit(6, number);
    connect(_numGroupBy, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numGroupSepLabel = new QLabel(tr("Grouping Separator:"), number);
    _numGroupSep = new LineEdit(6, number);
    connect(_numGroupSep, SIGNAL(validData()), SLOT(slotNumberChanged()));

    QLabel* numExampleLabel = new QLabel(tr("Example:"), number);
    _numExample = new LineEdit(20, number);
    _numExample->setFocusPolicy(NoFocus);

    QGridLayout* numGrid = new QGridLayout(number);
    numGrid->setSpacing(3);
    numGrid->setMargin(3);
    numGrid->setRowStretch(6, 1);
    numGrid->setColStretch(2, 1);
    numGrid->setColStretch(5, 3);
    numGrid->addWidget(numPosPrefixLabel, 0, 0);
    numGrid->addWidget(_numPosPrefix, 0, 1, AlignLeft | AlignVCenter);
    numGrid->addWidget(numPosSuffixLabel, 0, 3);
    numGrid->addWidget(_numPosSuffix, 0, 4, AlignLeft | AlignVCenter);
    numGrid->addWidget(numNegPrefixLabel, 1, 0);
    numGrid->addWidget(_numNegPrefix, 1, 1, AlignLeft | AlignVCenter);
    numGrid->addWidget(numNegSuffixLabel, 1, 3);
    numGrid->addWidget(_numNegSuffix, 1, 4, AlignLeft | AlignVCenter);
    numGrid->addWidget(numMaxIntLabel, 2, 0);
    numGrid->addWidget(_numMaxInt, 2, 1, AlignLeft | AlignVCenter);
    numGrid->addWidget(numMinIntLabel, 2, 3);
    numGrid->addWidget(_numMinInt, 2, 4, AlignLeft | AlignVCenter);
    numGrid->addWidget(numMaxDecLabel, 3, 0);
    numGrid->addWidget(_numMaxDec, 3, 1, AlignLeft | AlignVCenter);
    numGrid->addWidget(numMinDecLabel, 3, 3);
    numGrid->addWidget(_numMinDec, 3, 4, AlignLeft | AlignVCenter);
    numGrid->addWidget(numDecSepLabel, 4, 0);
    numGrid->addWidget(_numDecSep, 4, 1, AlignLeft | AlignVCenter);
    numGrid->addWidget(numGroupByLabel, 5, 0);
    numGrid->addWidget(_numGroupBy, 5, 1, AlignLeft | AlignVCenter);
    numGrid->addWidget(numGroupSepLabel, 5, 3);
    numGrid->addWidget(_numGroupSep, 5, 4, AlignLeft | AlignVCenter);
    numGrid->addWidget(numExampleLabel, 7, 0);
    numGrid->addMultiCellWidget(_numExample,7,7,1,4,AlignLeft|AlignVCenter);

    QLabel* monPosPrefixLabel = new QLabel(tr("Positive Prefix:"), money);
    _monPosPrefix = new LineEdit(6, money);
    connect(_monPosPrefix, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monPosSuffixLabel = new QLabel(tr("Positive Suffix:"), money);
    _monPosSuffix = new LineEdit(6, money);
    connect(_monPosSuffix, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monNegPrefixLabel = new QLabel(tr("Negative Prefix:"), money);
    _monNegPrefix = new LineEdit(6, money);
    connect(_monNegPrefix, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monNegSuffixLabel = new QLabel(tr("Negative Suffix:"), money);
    _monNegSuffix = new LineEdit(6, money);
    connect(_monNegSuffix, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monMaxIntLabel = new QLabel(tr("Max Int. Digits:"), money);
    _monMaxInt = new IntegerEdit(6, money);
    connect(_monMaxInt, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monMinIntLabel = new QLabel(tr("Min Int. Digits:"), money);
    _monMinInt = new IntegerEdit(6, money);
    connect(_monMinInt, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monMaxDecLabel = new QLabel(tr("Max Dec. Digits:"), money);
    _monMaxDec = new IntegerEdit(6, money);
    connect(_monMaxDec, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monMinDecLabel = new QLabel(tr("Min Dec. Digits:"), money);
    _monMinDec = new IntegerEdit(6, money);
    connect(_monMinDec, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monDecSepLabel = new QLabel(tr("Decimal Separator:"), money);
    _monDecSep = new LineEdit(6, money);
    connect(_monDecSep, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monGroupByLabel = new QLabel(tr("Group By:"), money);
    _monGroupBy = new IntegerEdit(6, money);
    connect(_monGroupBy, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monGroupSepLabel = new QLabel(tr("Grouping Separator:"), money);
    _monGroupSep = new LineEdit(6, money);
    connect(_monGroupSep, SIGNAL(validData()), SLOT(slotMoneyChanged()));

    QLabel* monExampleLabel = new QLabel(tr("Example:"), money);
    _monExample = new LineEdit(20, money);
    _monExample->setFocusPolicy(NoFocus);

    QGridLayout* monGrid = new QGridLayout(money);
    monGrid->setSpacing(3);
    monGrid->setMargin(3);
    monGrid->setRowStretch(6, 1);
    monGrid->setColStretch(2, 1);
    monGrid->setColStretch(5, 3);
    monGrid->addWidget(monPosPrefixLabel, 0, 0);
    monGrid->addWidget(_monPosPrefix, 0, 1, AlignLeft | AlignVCenter);
    monGrid->addWidget(monPosSuffixLabel, 0, 3);
    monGrid->addWidget(_monPosSuffix, 0, 4, AlignLeft | AlignVCenter);
    monGrid->addWidget(monNegPrefixLabel, 1, 0);
    monGrid->addWidget(_monNegPrefix, 1, 1, AlignLeft | AlignVCenter);
    monGrid->addWidget(monNegSuffixLabel, 1, 3);
    monGrid->addWidget(_monNegSuffix, 1, 4, AlignLeft | AlignVCenter);
    monGrid->addWidget(monMaxIntLabel, 2, 0);
    monGrid->addWidget(_monMaxInt, 2, 1, AlignLeft | AlignVCenter);
    monGrid->addWidget(monMinIntLabel, 2, 3);
    monGrid->addWidget(_monMinInt, 2, 4, AlignLeft | AlignVCenter);
    monGrid->addWidget(monMaxDecLabel, 3, 0);
    monGrid->addWidget(_monMaxDec, 3, 1, AlignLeft | AlignVCenter);
    monGrid->addWidget(monMinDecLabel, 3, 3);
    monGrid->addWidget(_monMinDec, 3, 4, AlignLeft | AlignVCenter);
    monGrid->addWidget(monDecSepLabel, 4, 0);
    monGrid->addWidget(_monDecSep, 4, 1, AlignLeft | AlignVCenter);
    monGrid->addWidget(monGroupByLabel, 5, 0);
    monGrid->addWidget(_monGroupBy, 5, 1, AlignLeft | AlignVCenter);
    monGrid->addWidget(monGroupSepLabel, 5, 3);
    monGrid->addWidget(_monGroupSep, 5, 4, AlignLeft | AlignVCenter);
    monGrid->addWidget(monExampleLabel, 7, 0);
    monGrid->addMultiCellWidget(_monExample,7,7,1,4,AlignLeft|AlignVCenter);

    QLabel* perPosPrefixLabel = new QLabel(tr("Positive Prefix:"), percent);
    _perPosPrefix = new LineEdit(6, percent);
    connect(_perPosPrefix, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perPosSuffixLabel = new QLabel(tr("Positive Suffix:"), percent);
    _perPosSuffix = new LineEdit(6, percent);
    connect(_perPosSuffix, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perNegPrefixLabel = new QLabel(tr("Negative Prefix:"), percent);
    _perNegPrefix = new LineEdit(6, percent);
    connect(_perNegPrefix, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perNegSuffixLabel = new QLabel(tr("Negative Suffix:"), percent);
    _perNegSuffix = new LineEdit(6, percent);
    connect(_perNegSuffix, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perMaxIntLabel = new QLabel(tr("Max Int. Digits:"), percent);
    _perMaxInt = new IntegerEdit(6, percent);
    connect(_perMaxInt, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perMinIntLabel = new QLabel(tr("Min Int. Digits:"), percent);
    _perMinInt = new IntegerEdit(6, percent);
    connect(_perMinInt, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perMaxDecLabel = new QLabel(tr("Max Dec. Digits:"), percent);
    _perMaxDec = new IntegerEdit(6, percent);
    connect(_perMaxDec, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perMinDecLabel = new QLabel(tr("Min Dec. Digits:"), percent);
    _perMinDec = new IntegerEdit(6, percent);
    connect(_perMinDec, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perDecSepLabel = new QLabel(tr("Decimal Separator:"), percent);
    _perDecSep = new LineEdit(6, percent);
    connect(_perDecSep, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perGroupByLabel = new QLabel(tr("Group By:"), percent);
    _perGroupBy = new IntegerEdit(6, percent);
    connect(_perGroupBy, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perGroupSepLabel = new QLabel(tr("Grouping Separator:"), percent);
    _perGroupSep = new LineEdit(6, percent);
    connect(_perGroupSep, SIGNAL(validData()), SLOT(slotPercentChanged()));

    QLabel* perExampleLabel = new QLabel(tr("Example:"), percent);
    _perExample = new LineEdit(20, percent);
    _perExample->setFocusPolicy(NoFocus);

    QGridLayout* perGrid = new QGridLayout(percent);
    perGrid->setSpacing(3);
    perGrid->setMargin(3);
    perGrid->setRowStretch(6, 1);
    perGrid->setColStretch(2, 1);
    perGrid->setColStretch(5, 3);
    perGrid->addWidget(perPosPrefixLabel, 0, 0);
    perGrid->addWidget(_perPosPrefix, 0, 1, AlignLeft | AlignVCenter);
    perGrid->addWidget(perPosSuffixLabel, 0, 3);
    perGrid->addWidget(_perPosSuffix, 0, 4, AlignLeft | AlignVCenter);
    perGrid->addWidget(perNegPrefixLabel, 1, 0);
    perGrid->addWidget(_perNegPrefix, 1, 1, AlignLeft | AlignVCenter);
    perGrid->addWidget(perNegSuffixLabel, 1, 3);
    perGrid->addWidget(_perNegSuffix, 1, 4, AlignLeft | AlignVCenter);
    perGrid->addWidget(perMaxIntLabel, 2, 0);
    perGrid->addWidget(_perMaxInt, 2, 1, AlignLeft | AlignVCenter);
    perGrid->addWidget(perMinIntLabel, 2, 3);
    perGrid->addWidget(_perMinInt, 2, 4, AlignLeft | AlignVCenter);
    perGrid->addWidget(perMaxDecLabel, 3, 0);
    perGrid->addWidget(_perMaxDec, 3, 1, AlignLeft | AlignVCenter);
    perGrid->addWidget(perMinDecLabel, 3, 3);
    perGrid->addWidget(_perMinDec, 3, 4, AlignLeft | AlignVCenter);
    perGrid->addWidget(perDecSepLabel, 4, 0);
    perGrid->addWidget(_perDecSep, 4, 1, AlignLeft | AlignVCenter);
    perGrid->addWidget(perGroupByLabel, 5, 0);
    perGrid->addWidget(_perGroupBy, 5, 1, AlignLeft | AlignVCenter);
    perGrid->addWidget(perGroupSepLabel, 5, 3);
    perGrid->addWidget(_perGroupSep, 5, 4, AlignLeft | AlignVCenter);
    perGrid->addWidget(perExampleLabel, 7, 0);
    perGrid->addMultiCellWidget(_perExample,7,7,1,4,AlignLeft|AlignVCenter);

    QFrame* buttons = new QFrame(this);
    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);

    connect(ok, SIGNAL(clicked()), SLOT(accept()));
    connect(cancel, SIGNAL(clicked()), SLOT(reject()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(cancel, 0, 2);

    QGridLayout* grid = new QGridLayout(this);
    grid->setSpacing(6);
    grid->setMargin(6);
    grid->addWidget(tabs, 0, 0);
    grid->addWidget(buttons, 1, 0);

    _dateOrder->setFocus();
    setCaption(tr("Customize Data Formats"));
}

DataFormat::~DataFormat()
{
}

void
DataFormat::setDateFormat(const QString& text)
{
#if 0
    ::DateFormat format;
    if (text != "date" && !text.isEmpty())
	format.fromString(text);

    _dateOrder->setCurrentItem(format.order);
    _dateSep->setText(format.separator);
    _datePadDay->setChecked(format.padDay);
    _datePadMonth->setChecked(format.padMonth);
    _dateTextMonth->setChecked(format.textMonth);
    _dateShortMonth->setChecked(format.shortMonth);
    _dateShortYear->setChecked(format.shortYear);

    slotDateChanged();
#endif
}

void
DataFormat::setTimeFormat(const QString& text)
{
#if 0
    TimeFormat format;
    if (text != "time" && !text.isEmpty())
	format.fromString(text);

    _timeSep->setText(format.separator);
    _timePadHour->setChecked(format.padHour);
    _timePadMinute->setChecked(format.padMinute);
    _timeShowSeconds->setChecked(format.showSeconds);
    _timeUseAmPm->setChecked(format.useAmPm);
    _timeAmText->setText(format.amText);
    _timePmText->setText(format.pmText);

    slotTimeChanged();
#endif
}

void
DataFormat::setNumberFormat(const QString& text)
{
#if 0
    NumberFormat format;
    if (text != "number" && !text.isEmpty())
	format.fromString(text);

    _numPosPrefix->setText(format.posPrefix);
    _numPosSuffix->setText(format.posSuffix);
    _numNegPrefix->setText(format.negPrefix);
    _numNegSuffix->setText(format.negSuffix);
    _numMaxInt->setInt(format.maxIntDigits);
    _numMinInt->setInt(format.minIntDigits);
    _numMaxDec->setInt(format.maxDecDigits);
    _numMinDec->setInt(format.minDecDigits);
    _numDecSep->setText(format.decimalSep);
    _numGroupBy->setInt(format.groupDigits);
    _numGroupSep->setText(format.groupSep);

    slotNumberChanged();
#endif
}

void
DataFormat::setMoneyFormat(const QString& text)
{
#if 0
    MoneyFormat format;
    if (text != "money" && !text.isEmpty())
	format.fromString(text);

    _monPosPrefix->setText(format.posPrefix);
    _monPosSuffix->setText(format.posSuffix);
    _monNegPrefix->setText(format.negPrefix);
    _monNegSuffix->setText(format.negSuffix);
    _monMaxInt->setInt(format.maxIntDigits);
    _monMinInt->setInt(format.minIntDigits);
    _monMaxDec->setInt(format.maxDecDigits);
    _monMinDec->setInt(format.minDecDigits);
    _monDecSep->setText(format.decimalSep);
    _monGroupBy->setInt(format.groupDigits);
    _monGroupSep->setText(format.groupSep);

    slotMoneyChanged();
#endif
}

void
DataFormat::setPercentFormat(const QString& text)
{
#if 0
    PercentFormat format;
    if (text != "percent" && !text.isEmpty())
	format.fromString(text);

    _perPosPrefix->setText(format.posPrefix);
    _perPosSuffix->setText(format.posSuffix);
    _perNegPrefix->setText(format.negPrefix);
    _perNegSuffix->setText(format.negSuffix);
    _perMaxInt->setInt(format.maxIntDigits);
    _perMinInt->setInt(format.minIntDigits);
    _perMaxDec->setInt(format.maxDecDigits);
    _perMinDec->setInt(format.minDecDigits);
    _perDecSep->setText(format.decimalSep);
    _perGroupBy->setInt(format.groupDigits);
    _perGroupSep->setText(format.groupSep);

    slotPercentChanged();
#endif
}

QString
DataFormat::getDateFormat() const
{
#if 0
    ::DateFormat format;
    format.order = _dateOrder->currentText();
    format.separator = _dateSep->text();
    format.padDay = _datePadDay->isChecked();
    format.padMonth = _datePadMonth->isChecked();
    format.textMonth = _dateTextMonth->isChecked();
    format.shortMonth = _dateShortMonth->isChecked();
    format.shortYear = _dateShortYear->isChecked();
    return format.toString();
#endif
    return "";
}

QString
DataFormat::getTimeFormat() const
{
#if 0
    TimeFormat format;
    format.separator = _timeSep->text();
    format.padHour = _timePadHour->isChecked();
    format.padMinute = _timePadMinute->isChecked();
    format.showSeconds = _timeShowSeconds->isChecked();
    format.useAmPm = _timeUseAmPm->isChecked();
    format.amText = _timeAmText->text();
    format.pmText = _timePmText->text();
    return format.toString();
#endif
    return "";
}

QString
DataFormat::getNumberFormat() const
{
#if 0
    NumberFormat format;
    format.posPrefix = _numPosPrefix->text();
    format.posSuffix = _numPosSuffix->text();
    format.negPrefix = _numNegPrefix->text();
    format.negSuffix = _numNegSuffix->text();
    format.maxIntDigits = _numMaxInt->getInt();
    format.minIntDigits = _numMinInt->getInt();
    format.maxDecDigits = _numMaxDec->getInt();
    format.minDecDigits = _numMinDec->getInt();
    format.decimalSep = _numDecSep->text();
    format.groupDigits = _numGroupBy->getInt();
    format.groupSep = _numGroupSep->text();
    return format.toString();
#endif
    return "";
}

QString
DataFormat::getMoneyFormat() const
{
#if 0
    MoneyFormat format;
    format.posPrefix = _monPosPrefix->text();
    format.posSuffix = _monPosSuffix->text();
    format.negPrefix = _monNegPrefix->text();
    format.negSuffix = _monNegSuffix->text();
    format.maxIntDigits = _monMaxInt->getInt();
    format.minIntDigits = _monMinInt->getInt();
    format.maxDecDigits = _monMaxDec->getInt();
    format.minDecDigits = _monMinDec->getInt();
    format.decimalSep = _monDecSep->text();
    format.groupDigits = _monGroupBy->getInt();
    format.groupSep = _monGroupSep->text();
    return format.toString();
#endif
    return "";
}

QString
DataFormat::getPercentFormat() const
{
#if 0
    PercentFormat format;
    format.posPrefix = _perPosPrefix->text();
    format.posSuffix = _perPosSuffix->text();
    format.negPrefix = _perNegPrefix->text();
    format.negSuffix = _perNegSuffix->text();
    format.maxIntDigits = _perMaxInt->getInt();
    format.minIntDigits = _perMinInt->getInt();
    format.maxDecDigits = _perMaxDec->getInt();
    format.minDecDigits = _perMinDec->getInt();
    format.decimalSep = _perDecSep->text();
    format.groupDigits = _perGroupBy->getInt();
    format.groupSep = _perGroupSep->text();
    return format.toString();
#endif
    return "";
}

void
DataFormat::slotDateChanged()
{
#if 0
    DateValcon valcon;
    valcon.setDisplayFmt(getDateFormat());
    _dateExample->setText(valcon.format(QDate::currentDate()));
#endif
}

void
DataFormat::slotTimeChanged()
{
#if 0
    TimeValcon valcon;
    valcon.setDisplayFmt(getTimeFormat());
    _timeExample->setText(valcon.format(QTime::currentTime()));
#endif
}

void
DataFormat::slotNumberChanged()
{
#if 0
    NumberValcon valcon;
    valcon.setDisplayFmt(getNumberFormat());
    _numExample->setText(valcon.format(fixed(12345.6789)));
#endif
}

void
DataFormat::slotMoneyChanged()
{
#if 0
    MoneyValcon valcon;
    valcon.setDisplayFmt(getMoneyFormat());
    _monExample->setText(valcon.format(fixed(12345.6789)));
#endif
}

void
DataFormat::slotPercentChanged()
{
#if 0
    PercentValcon valcon;
    valcon.setDisplayFmt(getPercentFormat());
    _perExample->setText(valcon.format(fixed(12.3456)));
#endif
}
