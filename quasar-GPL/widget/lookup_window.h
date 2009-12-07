// $Id: lookup_window.h,v 1.16 2004/03/20 01:33:45 bpepers Exp $
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

#ifndef LOOKUP_WINDOW_H
#define LOOKUP_WINDOW_H

#include <qdialog.h>
#include "data_lookup.h"

class ListView;
class LineEdit;
class QFrame;
class QLabel;
class QPushButton;
class QGridLayout;
class QListViewItem;

class LookupWindow: public QDialog, public DataLookup {
    Q_OBJECT
public:
    LookupWindow(QWidget* parent, const char* name=0);
    ~LookupWindow();

    bool autoRefresh() const;
    bool allowNew() const;
    bool allowEdit() const;
    void setAutoRefresh(bool flag);
    void setAllowNew(bool flag);
    void setAllowEdit(bool flag);

    virtual QString lookupById(Id id) = 0;
    virtual vector<DataPair> lookupByText(const QString& text) = 0;
    virtual int matchCount(const QString& text);

    virtual Id getId() const;
    virtual QString text() const;

    void clearLines();

public slots:
    virtual void setText(const QString& text);
    virtual void refresh() = 0;
    virtual QWidget* slotNew() = 0;
    virtual QWidget* slotEdit(Id id) = 0;

protected slots:
    virtual void slotEdit();
    virtual void currentChanged(QListViewItem*);
    void resizeEvent(QResizeEvent* e);
    void showEvent(QShowEvent* e);
    bool eventFilter(QObject* o, QEvent* e);
    void accept();
    void ensureItemVisible(QListViewItem* item);

signals:
    void idSelected(Id id); 

protected:
    void findClosest();

    bool _autoRefresh;
    bool _ignoreChanges;

    QFrame* _search;
    QLabel* _label;
    LineEdit* _text;
    QPushButton* _refresh;
    ListView* _list;
    QFrame* _buttons;
    QPushButton* _new;
    QPushButton* _edit;
    QPushButton* _ok;
    QPushButton* _cancel;
    QGridLayout* _searchGrid;
    QGridLayout* _buttonGrid;
};

#endif // LOOKUP_WINDOW_H
