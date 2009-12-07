// $Id: serial_port.h,v 1.6 2005/03/16 15:38:14 bpepers Exp $
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

#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <qfile.h>

struct SerialPrivate;

class SerialPort: public QFile
{
public:
    SerialPort();
    SerialPort(const QString& name);
    ~SerialPort();

    // Parity types
    enum { PARITY_NONE, PARITY_ODD, PARITY_EVEN };

    // Handshaking
    enum { HS_NONE, HS_XON_XOFF, HS_RTS_CTS };

    // Status bits
    enum { DCD=1, DSR=2, DTR=4, RTS=8, CTS=16, RNG=32 };

    // Open serial port and setup for raw input/output
    bool open(int mode);

    // Serial line characteristics
    void setBaud(int baud);
    void setDataBits(int bits);
    void setParity(int parity);
    void setStopBits(int bits);
    void setHandshake(int mode);
    int getBaud();
    int getDataBits();
    int getParity();
    int getStopBits();
    int getHandshake();

    // Set most things at once
    void setSerial(int baud, int dataBits, int parity, int stopBits);
    void getSerial(int& baud, int& dataBits, int& parity, int& stopBits);

    // Get the modem line status (can only use DCD,DSR,CTS,RNG)
    bool getStatus(int bit);
    int getStatus();

    // Set the line status (can only use DTR and RTS)
    void setStatus(int bit, bool value);
    void setStatus(int status);

    // Set a timeout on reads
    void setTimeout(int seconds);
    int getTimeout();

    // Wait until all output has been sent
    void drain();

protected:
    int _baud;
    int _dataBits;
    int _parity;
    int _stopBits;
    int _handshake;
    int _timeout;
    SerialPrivate* _p;
};

#endif // SERIAL_PORT_H
