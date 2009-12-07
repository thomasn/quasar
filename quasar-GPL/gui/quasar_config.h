// $Id: quasar_config.h,v 1.29 2005/03/13 22:12:27 bpepers Exp $
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

#ifndef QUASAR_CONFIG_H
#define QUASAR_CONFIG_H

#include "quasar_window.h"
#include "user_config.h"
#include <unicode/locid.h>

class LineEdit;
class IntegerEdit;
class LookupEdit;
class DatePopup;
class ComboBox;
class QListView;
class QWidgetStack;
class QCheckBox;
class QPushButton;
class QLabel;

class QuasarConfig: public QuasarWindow {
    Q_OBJECT
public:
    QuasarConfig(MainWindow* main);
    virtual ~QuasarConfig();

public slots:
    void slotOk();
    void slotApply();
    void slotDefaults();
    void slotCancel();
    void slotAreaChange();
    void slotChangeStyle(bool flag);
    void slotChangeColor(bool flag);
    void slotChangeFont(bool flag);
    void slotPickColor();
    void slotPickFont();
    void slotLocaleChange();

protected:
    UserConfig _config;
    bool _styleChanged;

    // Widgets
    QListView* _areas;
    QWidgetStack* _stack;

    // Display
    QFrame* _display;
    QCheckBox* _changeStyle;
    QCheckBox* _changeColor;
    QCheckBox* _changeFont;
    ComboBox* _style;
    QPushButton* _color;
    QPushButton* _font;

    // I18N
    const Locale* _locales;
    int32_t _localesCnt;
    QFrame* _i18n;
    ComboBox* _locale;
    QLabel* _dateSample;
    QLabel* _timeSample;
    QLabel* _numberPosSample;
    QLabel* _numberNegSample;
    QLabel* _moneyPosSample;
    QLabel* _moneyNegSample;
    QLabel* _percentPosSample;
    QLabel* _percentNegSample;
    void loadLocales();
    void setSamples();
};

#endif // QUASAR_CONFIG_H
