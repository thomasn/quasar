// $Id: quasar_config.cpp,v 1.50 2005/03/13 22:12:27 bpepers Exp $
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

#include "quasar_config.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"
#include "images.h"
#include "table.h"
#include "combo_box.h"
#include "integer_edit.h"
#include "money_edit.h"
#include "lookup_edit.h"
#include "store_lookup.h"
#include "station_lookup.h"
#include "employee_lookup.h"
#include "account_lookup.h"
#include "date_popup.h"
#include "date_valcon.h"
#include "time_valcon.h"
#include "percent_valcon.h"
#include "money_valcon.h"
#include "account.h"
#include "invoice.h"
#include "invoice_select.h"
#include "user.h"
#include "station.h"
#include "icu_util.h"

#include <qapplication.h>
#include <qlistview.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qgroupbox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qheader.h>
#include <qcolordialog.h>
#include <qfontdialog.h>
#include <qmessagebox.h>
#include <qstylefactory.h>
#include <qstyle.h>
#include <typeinfo>

static Locale systemLocale;

QuasarConfig::QuasarConfig(MainWindow* main)
    : QuasarWindow(main, "QuasarConfig"), _styleChanged(false)
{
    _helpSource = "quasar_config.html";

    QFrame* frame = new QFrame(this);

    _areas = new QListView(frame);
    _stack = new QWidgetStack(frame);

    _display = new QFrame(_stack);
    _i18n = new QFrame(_stack);

    _changeStyle = new QCheckBox(tr("Change Style?"), _display);
    _changeStyle->setMinimumSize(_changeStyle->sizeHint());
    connect(_changeStyle, SIGNAL(toggled(bool)), SLOT(slotChangeStyle(bool)));

    _style = new ComboBox(_display);
    _style->insertItem("");
    _style->insertStringList(QStyleFactory::keys());

    _changeColor = new QCheckBox(tr("Change Color?"), _display);
    _changeColor->setMinimumSize(_changeColor->sizeHint());
    connect(_changeColor, SIGNAL(toggled(bool)), SLOT(slotChangeColor(bool)));

    _color = new QPushButton(tr("New Color"), _display);
    connect(_color, SIGNAL(clicked()), SLOT(slotPickColor()));

    _changeFont = new QCheckBox(tr("Change Font?"), _display);
    _changeFont->setMinimumSize(_changeFont->sizeHint());
    connect(_changeFont, SIGNAL(toggled(bool)), SLOT(slotChangeFont(bool)));

    _font = new QPushButton(tr("Push To Choose"), _display);
    connect(_font, SIGNAL(clicked()), SLOT(slotPickFont()));

    QGridLayout* displayGrid = new QGridLayout(_display);
    displayGrid->setSpacing(3);
    displayGrid->setMargin(3);
    displayGrid->setRowStretch(3, 1);
    displayGrid->setColStretch(2, 1);
    displayGrid->addWidget(_changeStyle, 0, 0);
    displayGrid->addWidget(_style, 0, 1);
    displayGrid->addWidget(_changeColor, 1, 0);
    displayGrid->addWidget(_color, 1, 1);
    displayGrid->addWidget(_changeFont, 2, 0);
    displayGrid->addWidget(_font, 2, 1);

    QLabel* localeLabel = new QLabel(tr("Locale:"), _i18n);
    _locale = new ComboBox(_i18n);
    _locale->setMinimumWidth(_locale->fontMetrics().width("x") * 30);
    localeLabel->setBuddy(_locale);
    connect(_locale, SIGNAL(activated(int)), SLOT(slotLocaleChange()));

    QGroupBox* samples = new QGroupBox(tr("Data Formats"), _i18n);

    QLabel* positiveLabel = new QLabel(tr("Positive"), samples);
    QLabel* negativeLabel = new QLabel(tr("Negative"), samples);

    QLabel* numLabel = new QLabel(tr("Number:"), samples);
    _numberPosSample = new QLabel(samples);
    _numberNegSample = new QLabel(samples);

    QLabel* moneyLabel = new QLabel(tr("Currency:"), samples);
    _moneyPosSample = new QLabel(samples);
    _moneyNegSample = new QLabel(samples);

    QLabel* percentLabel = new QLabel(tr("Percent:"), samples);
    _percentPosSample = new QLabel(samples);
    _percentNegSample = new QLabel(samples);

    QLabel* dateLabel = new QLabel(tr("Date:"), samples);
    _dateSample = new QLabel(samples);

    QLabel* timeLabel = new QLabel(tr("Time:"), samples);
    _timeSample = new QLabel(samples);

    QGridLayout* sampleGrid = new QGridLayout(samples);
    sampleGrid->addRowSpacing(0, samples->fontMetrics().height());
    sampleGrid->setSpacing(3);
    sampleGrid->setMargin(10);
    sampleGrid->setColStretch(1, 1);
    sampleGrid->setColStretch(2, 1);
    sampleGrid->setRowStretch(3, 1);
    sampleGrid->addRowSpacing(3, 20);
    sampleGrid->addWidget(dateLabel, 1, 0);
    sampleGrid->addWidget(_dateSample, 1, 1, AlignRight|AlignVCenter);
    sampleGrid->addWidget(timeLabel, 2, 0);
    sampleGrid->addWidget(_timeSample, 2, 1, AlignRight|AlignVCenter);
    sampleGrid->addWidget(positiveLabel, 4, 1, AlignRight|AlignVCenter);
    sampleGrid->addWidget(negativeLabel, 4, 2, AlignRight|AlignVCenter);
    sampleGrid->addWidget(numLabel, 5, 0);
    sampleGrid->addWidget(_numberPosSample, 5, 1, AlignRight|AlignVCenter);
    sampleGrid->addWidget(_numberNegSample, 5, 2, AlignRight|AlignVCenter);
    sampleGrid->addWidget(moneyLabel, 6, 0);
    sampleGrid->addWidget(_moneyPosSample, 6, 1, AlignRight|AlignVCenter);
    sampleGrid->addWidget(_moneyNegSample, 6, 2, AlignRight|AlignVCenter);
    sampleGrid->addWidget(percentLabel, 7, 0);
    sampleGrid->addWidget(_percentPosSample, 7, 1, AlignRight|AlignVCenter);
    sampleGrid->addWidget(_percentNegSample, 7, 2, AlignRight|AlignVCenter);

    QGridLayout* i18nGrid = new QGridLayout(_i18n);
    i18nGrid->setSpacing(3);
    i18nGrid->setMargin(3);
    i18nGrid->addRowSpacing(1, 20);
    i18nGrid->setRowStretch(2, 1);
    i18nGrid->setColStretch(2, 1);
    i18nGrid->addWidget(localeLabel, 0, 0);
    i18nGrid->addWidget(_locale, 0, 1, AlignLeft | AlignVCenter);
    i18nGrid->addMultiCellWidget(samples, 2, 2, 0, 2);

    _stack->addWidget(_display, 0);
    _stack->addWidget(_i18n, 1);

    _areas->addColumn(tr("Area"), -1);
    _areas->setSorting(-1);
    _areas->header()->hide();
    connect(_areas, SIGNAL(selectionChanged()), SLOT(slotAreaChange()));

    QListViewItem* user = new QListViewItem(_areas, tr("User Configuration"));

    user->setOpen(true);
    new QListViewItem(user, tr("Internationalization"));
    QListViewItem* displayArea = new QListViewItem(user, tr("Display"));

    QFrame* buttons = new QFrame(frame);
    QPushButton* ok = new QPushButton(tr("&OK"), buttons);
    QPushButton* apply = new QPushButton(tr("&Apply"), buttons);
    QPushButton* defaults = new QPushButton(tr("&Defaults"), buttons);
    QPushButton* cancel = new QPushButton(tr("&Cancel"), buttons);

    ok->setMinimumSize(defaults->sizeHint());
    apply->setMinimumSize(defaults->sizeHint());
    defaults->setMinimumSize(defaults->sizeHint());
    cancel->setMinimumSize(defaults->sizeHint());

    connect(ok, SIGNAL(clicked()), SLOT(slotOk()));
    connect(apply, SIGNAL(clicked()), SLOT(slotApply()));
    connect(defaults, SIGNAL(clicked()), SLOT(slotDefaults()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));

    QGridLayout* buttonGrid = new QGridLayout(buttons);
    buttonGrid->setSpacing(3);
    buttonGrid->setMargin(3);
    buttonGrid->setColStretch(0, 1);
    buttonGrid->addWidget(ok, 0, 1);
    buttonGrid->addWidget(apply, 0, 2);
    buttonGrid->addWidget(defaults, 0, 3);
    buttonGrid->addWidget(cancel, 0, 4);

    QGridLayout* grid = new QGridLayout(frame);
    grid->setSpacing(3);
    grid->setMargin(3);
    grid->setColStretch(1, 1);
    grid->addWidget(_areas, 0, 0);
    grid->addWidget(_stack, 0, 1);
    grid->addMultiCellWidget(buttons, 1, 1, 0, 1);

    _config.load();
    _locales = Locale::getAvailableLocales(_localesCnt);

    slotDefaults();
    _areas->setCurrentItem(displayArea);
    _stack->raiseWidget(_display);
    _changeStyle->setFocus();

    setCentralWidget(frame);
    setCaption(tr("Quasar Configuration"));
    finalize();
}

QuasarConfig::~QuasarConfig()
{
}

void
QuasarConfig::slotOk()
{
    slotApply();

    QString localeName;
    if (_locale->currentItem() != 0)
	localeName = _locales[_locale->currentItem() - 1].getName();

    if (_config.locale != localeName)
	QuasarClient::setLocale(localeName);

    // User configuration
    _config.changeStyle = _changeStyle->isChecked();
    _config.changeColor = _changeColor->isChecked();
    _config.changeFont = _changeFont->isChecked();
    _config.style = _style->currentText();
    _config.color = _color->palette().active().background().name();
    _config.font = _font->font().toString();
    _config.locale = localeName;
    _config.save(true);

    delete this;
}

void
QuasarConfig::slotApply()
{
    // Try to restore the style but because of how Qt works if the
    // default system style was used and at some point the style was
    // changed, there is no way to go back to the original default
    // style.  It would be nice if setStyle(NULL) restored the default
    // or else there was a way to get the name of the default style.
    bool styleMessage = false;
    if (_changeStyle->isChecked()) {
	qApp->setStyle(_style->currentText());
	_styleChanged = true;
    } else if (_styleChanged || _config.changeStyle) {
	styleMessage = true;
    }

    if (_changeColor->isChecked()) {
	qApp->setPalette(_color->palette(), true);
    } else {
	qApp->setPalette(UserConfig::defaultPalette, true);
    }

    if (_changeFont->isChecked()) {
	qApp->setFont(_font->font(), true);
    } else {
	qApp->setFont(UserConfig::defaultFont, true);
    }

    if (styleMessage) {
	QString message = tr("Your style will be restored to the\n"
			     "default next time you start Quasar");
	qApp->beep();
	QMessageBox::warning(this, tr("Warning"), message);
    }
}

void
QuasarConfig::slotDefaults()
{
    _changeStyle->setChecked(_config.changeStyle);
    _style->setEnabled(_config.changeStyle);
    _style->setCurrentItem(_config.style);
    _changeColor->setChecked(_config.changeColor);
    _color->setEnabled(_config.changeColor);
    _color->setPalette(QColor(_config.color));
    _changeFont->setChecked(_config.changeFont);
    _font->setEnabled(_config.changeFont);
    _font->font().fromString(_config.font);

    loadLocales();
    setSamples();
}

void
QuasarConfig::loadLocales()
{
    bool firstTime = (_locale->count() == 0);
    int index = _locale->currentItem();

    _locale->clear();
    _locale->insertItem(tr("System Default"));
    for (int32_t i = 0; i < _localesCnt; ++i) {
	UnicodeString uname;
	_locales[i].getDisplayName(uname);

	QString name = convertToQt(uname);
	_locale->insertItem(name);

	if (firstTime && _config.locale == _locales[i].getName()) {
	    firstTime = false;
	    index = _locale->count() - 1;
	}
    }

    _locale->setCurrentItem(index);
}

void
QuasarConfig::slotCancel()
{
    slotDefaults();
    slotApply();
    delete this;
}

void
QuasarConfig::slotAreaChange()
{
    QListViewItem* item = _areas->currentItem();
    if (item == NULL) return;

    QString text = item->text(0);
    if (text == tr("Display")) _stack->raiseWidget(_display);
    if (text == tr("Internationalization")) _stack->raiseWidget(_i18n);
}

void
QuasarConfig::slotChangeStyle(bool flag)
{
    _style->setEnabled(flag);
}

void
QuasarConfig::slotChangeColor(bool flag)
{
    _color->setEnabled(flag);
}

void
QuasarConfig::slotChangeFont(bool flag)
{
    _font->setEnabled(flag);
}

void
QuasarConfig::slotPickColor()
{
    QColor color = QColorDialog::getColor(_color->backgroundColor());
    if (color.isValid())
	_color->setPalette(color);
}

void
QuasarConfig::slotPickFont()
{
    bool ok;
    QFont font = QFontDialog::getFont(&ok, _font->font());
    if (ok)
	_font->setFont(font);
}

void
QuasarConfig::slotLocaleChange()
{
    Locale locale;
    if (_locale->currentItem() == 0)
	locale = systemLocale;
    else
	locale = _locales[_locale->currentItem() - 1];

    QString localesDir = QuasarClient::localesDir();
    QString language(locale.getLanguage());
    QString country(locale.getCountry());

    bool found = false;
    QTranslator translator(0);
    if (!country.isEmpty()) {
	QString dir = localesDir + "/" + language + "_" + country;
	if (translator.load("messages.qm", dir))
	    found = true;
    }
    if (!found) {
	QString dir = localesDir + "/" + language;
	if (translator.load("messages.qm", dir))
	    found = true;
    }
    if (!found) {
	QString message = tr("Quasar has not been localized for this\n"
			     "locale so the text of the program will\n"
			     "not change but the date, time, number,\n"
			     "currency, and percent should be properly\n"
			     "localized.");
	QMessageBox::warning(this, tr("Warning"), message);
    }

    // Set default locale for ICU
    UErrorCode status = U_ZERO_ERROR;
    Locale::setDefault(locale, status);
    if (U_FAILURE(status)) {
	QString msg = tr("Failed setting locale to %1").arg(locale.getName());
	QMessageBox::critical(this, tr("Error"), msg);
    }

    loadLocales();
    setSamples();
}

void
QuasarConfig::setSamples()
{
    QDate date(2001, 12, 31);
    _dateSample->setText(DateValcon().format(date));

    QTime time(12, 29, 30);
    _timeSample->setText(TimeValcon().format(time));

    fixed value = 12345.6789;
    _numberPosSample->setText(NumberValcon().format(value));
    _numberNegSample->setText(NumberValcon().format(-value));

    _moneyPosSample->setText(MoneyValcon().format(value));
    _moneyNegSample->setText(MoneyValcon().format(-value));

    value = .123456;
    _percentPosSample->setText(PercentValcon().format(value));
    _percentNegSample->setText(PercentValcon().format(-value));
}
