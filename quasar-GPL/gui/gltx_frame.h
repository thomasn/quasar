// $Id: gltx_frame.h,v 1.6 2004/01/31 01:50:31 arandell Exp $
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

#ifndef GLTX_FRAME_H
#define GLTX_FRAME_H

#include <qgroupbox.h>
#include "data_object.h"

class MainWindow;
class QuasarClient;
class Gltx;
class QLabel;
class LineEdit;
class DatePopup;
class TimeEdit;
class LookupEdit;
class StoreLookup;
class CardLookup;
class QGridLayout;

class GltxFrame: public QGroupBox
{
    Q_OBJECT
public:
    GltxFrame(MainWindow* main, const QString& numberText, QWidget* parent);
    virtual ~GltxFrame();

    void setTitle(const QString& title);
    void hideMemo();
    QWidget* firstField();

    void getData(Gltx& gltx);
    void setData(const Gltx& gltx);
    void defaultData(Gltx& gltx);
    void cloneData(Gltx& gltx);

    // Widgets
    QLabel* numberLabel;
    LineEdit* number;
    DatePopup* date;
    TimeEdit* time;
    LineEdit* reference;
    StoreLookup* storeLookup;
    LookupEdit* store;
    LookupEdit* station;
    LookupEdit* employee;
    LookupEdit* shift;
    QLabel* memoLabel;
    LineEdit* memo;
    QGridLayout* grid;

protected:
    MainWindow* _main;
    QuasarClient* _quasar;
};

#endif // GLTX_FRAME_H
