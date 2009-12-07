// $Id: report_box.h,v 1.6 2004/02/03 00:56:03 arandell Exp $
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

#ifndef REPORT_BOX_H
#define REPORT_BOX_H

#include <qtabwidget.h>
#include "report_label.h"

#define REPORT_BOX_ROWS 4
#define REPORT_BOX_COLS 2
#define REPORT_BOX_CNT (REPORT_BOX_ROWS * REPORT_BOX_COLS)

class ReportBox: public QTabWidget {
    Q_OBJECT
public:
    ReportBox(QWidget* parent=0, const char* name=0);
    ~ReportBox();

    QString text(int row, int col);
    void setText(int row, int col, const QString& text);

    QString type(int row, int col);
    void setType(int row, int col, const QString& type);

    QString args(int row, int col);
    void setArgs(int row, int col, const QString& args);

signals:
    void clicked(int row, int col);
    void clicked(QString type, QString args);

protected slots:
    void labelClicked(ReportLabel* label);

protected:
    ReportLabel* _labels[REPORT_BOX_CNT];
};

#endif // REPORT_BOX_H
