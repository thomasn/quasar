// $Id: quasar_misc.h,v 1.4 2005/04/12 07:37:19 bpepers Exp $
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

#ifndef QUASAR_MISC_H
#define QUASAR_MISC_H

// Misc routines that don't fit in a particular class

#include <qstring.h>

// Directory where program executable is found
extern QString programDir();

// Encrypt a password using SHA-1
extern QString sha1Crypt(const QString& password);

// Sleep for a number of seconds
extern void qsleep(int seconds);

// Set permissions on a file (user and/or group can be NULL to not set)
extern void setPermissions(const char* file, int mode, const char* user,
			   const char* group);

#endif // QUASAR_MISC_H
