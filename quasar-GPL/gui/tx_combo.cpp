// $Id: tx_combo.cpp,v 1.13 2004/11/02 11:13:07 bpepers Exp $
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

#include "tx_combo.h"
#include "quasar_client.h"

TxCombo::TxCombo(QuasarClient*, QWidget* parent, const char* name)
    : ComboBox(parent, name)
{
    QStringList types;
    types << DataObject::dataTypeName(DataObject::ITEM_ADJUST);
    types << DataObject::dataTypeName(DataObject::CHEQUE);
    types << DataObject::dataTypeName(DataObject::GENERAL);
    types << DataObject::dataTypeName(DataObject::INVOICE);
    types << DataObject::dataTypeName(DataObject::RECEIPT);
    types << DataObject::dataTypeName(DataObject::RECEIVE);
    types << DataObject::dataTypeName(DataObject::NOSALE);
    types << DataObject::dataTypeName(DataObject::PAYOUT);
    types << DataObject::dataTypeName(DataObject::RETURN);
    types << DataObject::dataTypeName(DataObject::SHIFT);
    types << DataObject::dataTypeName(DataObject::TEND_ADJUST);
    types << DataObject::dataTypeName(DataObject::CLAIM);
    types << DataObject::dataTypeName(DataObject::WITHDRAW);
    types << DataObject::dataTypeName(DataObject::CARD_ADJUST);
    types.sort();

    insertItem(tr("All Types"));
    for (unsigned int i = 0; i < types.count(); ++i)
	insertItem(types[i]);
}

TxCombo::~TxCombo()
{
}

int
TxCombo::dataType() const
{
    if (currentItem() == 0) return -1;
    return DataObject::dataType(currentText());
}

void
TxCombo::setDataType(int type)
{
    if (type == -1)
	setCurrentItem(0);
    else
	setCurrentItem(DataObject::dataTypeName(type));
}
