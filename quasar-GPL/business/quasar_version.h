// $Id: quasar_version.h,v 1.19 2005/06/22 05:59:25 bpepers Exp $
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

#ifndef QUASAR_VERSION_H
#define QUASAR_VERSION_H

#include <qstring.h>

#define QUASAR_MAJOR 1
#define QUASAR_MINOR 4
#define QUASAR_PATCH 7

#define QUASAR_MAJOR_S QString::number(QUASAR_MAJOR)
#define QUASAR_MINOR_S QString::number(QUASAR_MINOR)
#define QUASAR_PATCH_S QString::number(QUASAR_PATCH)

#define QUASAR_VERSION_S (QUASAR_MAJOR_S + "." + QUASAR_MINOR_S + "." + QUASAR_PATCH_S)

// Current version of database
#define QUASAR_DB_VERSION "2005-03-07"

#endif // QUASAR_VERSION_H
