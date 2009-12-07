// $Id: ymodem.h,v 1.3 2004/12/08 05:12:06 bpepers Exp $
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

#ifndef HH_YMODEM_H
#define HH_YMODEM_H

//======================================================================
//	Special Characters
//======================================================================
const char SOH = 0x01;
const char STX = 0x02;
const char EOT = 0x04;
const char ACK = 0x06;
const char NAK = 0x15;
const char CAN = 0x18;

//======================================================================
//	Return Values
//======================================================================
const int OK = 0;
const int ERROR = -1;
const int TIMEOUT = -2;
const int GOT_EOT = -3;

const int MAX_BLOCK_LEN = 1024;
const int BLOCK_LEN = 128;
const int RETRY_MAX = 10;

#endif
