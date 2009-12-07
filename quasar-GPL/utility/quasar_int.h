// $Id: quasar_int.h,v 1.2 2005/04/12 19:52:53 bpepers Exp $
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

// Define integer types used by Quasar in a platform neutral mannor

#include "../config.h"

#ifdef HAVE_STDINT_H
#include <stdint.h>
#else
typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#ifdef __GNUC__
typedef long long int int64_t;
typedef unsigned long long int uint64_t;
#else
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif
#endif

// Validate that int32_t and int64_t are really the size they should be
typedef char Invalid_size_for_int32_t [1 / (sizeof(int32_t) / 4) - 1];
typedef char Invalid_size_for_int64_t [1 / (sizeof(int64_t) / 8) - 1];
