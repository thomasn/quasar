// $Id: serial_port_win32.cpp,v 1.4 2005/03/16 15:38:14 bpepers Exp $
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

#include "serial_port.h"

#include <windows.h>
#include <io.h>

#define WIN_HANDLE ((HANDLE)_get_osfhandle(handle()))

SerialPort::SerialPort()
    : QFile(), _baud(9600), _dataBits(8), _parity(PARITY_NONE),
      _stopBits(), _handshake(HS_XON_XOFF), _timeout(1)
{
}

SerialPort::SerialPort(const QString& name)
    : QFile(name), _baud(9600), _dataBits(8), _parity(PARITY_NONE),
      _stopBits(), _handshake(HS_XON_XOFF), _timeout(1)
{
}

SerialPort::~SerialPort()
{
}

bool
SerialPort::open(int mode)
{
    // Always open as raw so no input/output buffering
    if (!QFile::open(mode | IO_Raw))
	return false;

    // Set serial port to current characteristics
    setSerial(_baud, _dataBits, _parity, _stopBits);
    setHandshake(_handshake);
    setTimeout(_timeout);

    return true;
}

void
SerialPort::setBaud(int baud)
{
    _baud = baud;
    setSerial(_baud, _dataBits, _parity, _stopBits);
}

void
SerialPort::setDataBits(int bits)
{
    _dataBits = bits;
    setSerial(_baud, _dataBits, _parity, _stopBits);
}

void
SerialPort::setParity(int parity)
{
    _parity = parity;
    setSerial(_baud, _dataBits, _parity, _stopBits);
}

void
SerialPort::setStopBits(int bits)
{
    _stopBits = bits;
    setSerial(_baud, _dataBits, _parity, _stopBits);
}

void
SerialPort::setHandshake(int mode)
{
    _handshake = mode;
    setSerial(_baud, _dataBits, _parity, _stopBits);
}

int
SerialPort::getBaud()
{
    return _baud;
}

int
SerialPort::getDataBits()
{
    return _dataBits;
}

int
SerialPort::getParity()
{
    return _parity;
}

int
SerialPort::getStopBits()
{
    return _stopBits;
}

int
SerialPort::getHandshake()
{
    return _handshake;
}

void
SerialPort::setSerial(int baud, int dataBits, int parity, int stopBits)
{
    DCB dcb;
    if (!GetCommState(WIN_HANDLE, &dcb)) {
	qWarning("Error: GetCommState failed: %d", GetLastError());
	return;
    }

    _baud = baud;
    _dataBits = dataBits;
    _parity = parity;
    _stopBits = stopBits;

    BYTE winParity = NOPARITY;
    switch (parity) {
    case PARITY_NONE:		winParity = NOPARITY; break;
    case PARITY_ODD:		winParity = ODDPARITY; break;
    case PARITY_EVEN:		winParity = EVENPARITY; break;
    }

    BYTE winStopBits = ONESTOPBIT;
    switch (stopBits) {
    case 1:		winStopBits = ONESTOPBIT; break;
    case 2:		winStopBits = TWOSTOPBITS; break;
    }

    // TODO: set the bits below properly for _handshake mode

    dcb.BaudRate = baud;
    dcb.ByteSize = dataBits;
    dcb.Parity = winParity;
    dcb.StopBits  = winStopBits;
    dcb.fBinary  = TRUE;
    dcb.fDsrSensitivity = FALSE;
    dcb.fParity  = TRUE;
    dcb.fOutX  = FALSE;
    dcb.fInX   = FALSE;
    dcb.fNull   = FALSE;
    dcb.fAbortOnError = TRUE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxDsrFlow = FALSE;
    dcb.fDtrControl = DTR_CONTROL_DISABLE;
    dcb.fDsrSensitivity= FALSE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fOutxCtsFlow = FALSE;
    dcb.fOutxCtsFlow = FALSE;

    if (!SetCommState(WIN_HANDLE, &dcb)) {
	qWarning("Error: SetCommState failed: %d", GetLastError());
	return;
    }
}

void
SerialPort::getSerial(int& baud, int& dataBits, int& parity, int& stopBits)
{
    baud = getBaud();
    dataBits = getDataBits();
    parity = getParity();
    stopBits = getStopBits();
}

void
SerialPort::setTimeout(int timeout)
{
    COMMTIMEOUTS cts;
    if (!GetCommTimeouts(WIN_HANDLE, &cts)) {
	qWarning("Error: GetCommTimeouts failed: %d", GetLastError());
	return;
    }

    _timeout = timeout;

    cts.ReadIntervalTimeout = 0;
    cts.ReadTotalTimeoutConstant = timeout * 1000;
    cts.ReadTotalTimeoutMultiplier = 1;
    cts.WriteTotalTimeoutConstant = timeout * 1000;
    cts.WriteTotalTimeoutMultiplier = 1;

    if (!SetCommTimeouts(WIN_HANDLE, &cts)) {
	qWarning("Error: SetCommTimeouts failed: %d", GetLastError());
	return;
    }
}

int
SerialPort::getTimeout()
{
    return _timeout;
}

bool
SerialPort::getStatus(int bit)
{
    return getStatus() & bit;
}

void
SerialPort::setStatus(int bit, bool value)
{
    if (isOpen()) {
	DWORD function = 0;

	if (bit == DTR && value)  function = SETDTR;
	if (bit == DTR && !value) function = CLRDTR;
	if (bit == RTS && value)  function = SETRTS;
	if (bit == RTS && !value) function = CLRRTS;
	if (function == 0) return;

	if (!EscapeCommFunction(WIN_HANDLE, function)) {
	    qWarning("Error: EscapeCommFunction failed: %d", GetLastError());
	    return;
	}
    }
}

int
SerialPort::getStatus()
{
    if (!isOpen()) return 0;

    DWORD winStatus = 0;
    if (!GetCommModemStatus(WIN_HANDLE, &winStatus)) {
	qWarning("Error: GetCommModemStatus failed: %d", GetLastError());
	return 0;
    }

    int status = 0;
    if (winStatus & MS_RLSD_ON) status |= DCD;
    if (winStatus & MS_DSR_ON) status |= DSR;
    if (winStatus & MS_CTS_ON) status |= CTS;
    if (winStatus & MS_RING_ON) status |= RNG;

    return status;
}

void
SerialPort::setStatus(int status)
{
    if (isOpen()) {
        setStatus(DTR, status & DTR);
        setStatus(RTS, status & RTS);
    }
}

void
SerialPort::drain()
{
    if (isOpen())
	FlushFileBuffers(WIN_HANDLE);
}
