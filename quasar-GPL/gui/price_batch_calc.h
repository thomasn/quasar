// $Id: price_batch_calc.h,v 1.1 2004/03/18 22:41:16 bpepers Exp $
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

#ifndef PRICE_BATCH_CALC_H
#define PRICE_BATCH_CALC_H

#include <qdialog.h>
#include "fixed.h"

class QRadioButton;
class QPushButton;
class NumberEdit;

class PriceBatchCalc: public QDialog {
    Q_OBJECT
public:
    PriceBatchCalc(QWidget* parent);
    virtual ~PriceBatchCalc();

    enum Type { Markup, Margin, Change, Target };

    int getType();
    fixed getPercent();

public slots:
    void slotMarkupChanged();
    void slotMarginChanged();
    void slotChangeChanged();
    void slotModeChanged(int index);

protected slots:
    void accept();

protected:
    QRadioButton* _useMarkup;
    QRadioButton* _useMargin;
    QRadioButton* _useChange;
    QRadioButton* _useTarget;
    NumberEdit* _markup;
    NumberEdit* _margin;
    NumberEdit* _change;
    QPushButton* _ok;
};

#endif // PRICE_BATCH_CALC_H
