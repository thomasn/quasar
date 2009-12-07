// $Id: serial_port_unix.cpp,v 1.7 2005/03/16 15:38:14 bpepers Exp $
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

#include <sys/ioctl.h>
#include <termios.h>
#include <errno.h>

struct SerialPrivate {
    struct termios state;
};

SerialPort::SerialPort()
    : QFile(), _baud(9600), _dataBits(8), _parity(PARITY_NONE),
      _stopBits(1), _handshake(HS_XON_XOFF), _timeout(1)
{
    _p = new SerialPrivate();
}

SerialPort::SerialPort(const QString& name)
    : QFile(name), _baud(9600), _dataBits(8), _parity(PARITY_NONE),
      _stopBits(1), _handshake(HS_XON_XOFF), _timeout(1)
{
    _p = new SerialPrivate();
}

SerialPort::~SerialPort()
{
    if (isOpen()) {
	if (tcsetattr(handle(), TCSANOW, &_p->state) < 0)
	    qWarning("Restoring termios state failed: %d", errno);
    }

    delete _p;
}

bool
SerialPort::open(int mode)
{
    // Always open as raw so no input/output buffering
    if (!QFile::open(mode | IO_Raw))
	return false;

    // Save current state and set raw
    if (tcgetattr(handle(),&_p->state) < 0) {
	qWarning("Saving termios state failed: %d", errno);
	close();
	return false;
    } else {
	struct termios state = _p->state;
	cfmakeraw(&state);
	state.c_cflag |= CLOCAL;
	state.c_cflag &= ~CRTSCTS;
	if (tcsetattr(handle(), TCSANOW, &state) < 0) {
	    qWarning("Setting raw state failed: %d", errno);
	    close();
	    return false;
	}
    }

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
    if (isOpen()) {
	struct termios state;
	if (tcgetattr(handle(),&state) < 0)
	    qWarning("Getting termios state failed: %d", errno);

	int speed = B9600;
	switch (baud) {
	case 0:		speed = B0; break;
	case 50:	speed = B50; break;
	case 75:	speed = B75; break;
	case 110:	speed = B110; break;
	case 134:	speed = B134; break;
	case 150:	speed = B150; break;
	case 200:	speed = B200; break;
	case 300:	speed = B300; break;
	case 600:	speed = B600; break;
	case 1200:	speed = B1200; break;
	case 1800:	speed = B1800; break;
	case 2400:	speed = B2400; break;
	case 4800:	speed = B4800; break;
	case 9600:	speed = B9600; break;
	case 19200:	speed = B19200; break;
	case 38400:	speed = B38400; break;
	case 57600:	speed = B57600; break;
	case 115200:	speed = B115200; break;
	case 230400:	speed = B230400; break;
	default: qWarning("Invalid baud rate: %d", baud);
	}

	cfsetispeed(&state, speed);
	cfsetospeed(&state, speed);

	if (tcsetattr(handle(), TCSANOW, &state) < 0)
	    qWarning("Setting termios state failed: %d", errno);
    }
}

void
SerialPort::setDataBits(int bits)
{
    _dataBits = bits;
    if (isOpen()) {
	struct termios state;
	if (tcgetattr(handle(),&state) < 0)
	    qWarning("Getting termios state failed: %d", errno);

	state.c_cflag &= ~CSIZE;
	switch (bits) {
	case 5:	 state.c_cflag |= CS5; break;
	case 6:	 state.c_cflag |= CS6; break;
	case 7:	 state.c_cflag |= CS7; break;
	case 8:	 state.c_cflag |= CS8; break;
	default: qWarning("Invalid data bits: %d", bits);
	}

	if (tcsetattr(handle(), TCSANOW, &state) < 0)
	    qWarning("Setting termios state failed: %d", errno);
    }
}

void
SerialPort::setParity(int parity)
{
    _parity = parity;
    if (isOpen()) {
	struct termios state;
	if (tcgetattr(handle(),&state) < 0)
	    qWarning("Getting termios state failed: %d", errno);

	state.c_cflag |= PARENB;
	switch (parity) {
	case PARITY_NONE:	state.c_cflag &= ~PARENB; break;
	case PARITY_ODD:	state.c_cflag |= PARODD; break;
	case PARITY_EVEN:	state.c_cflag &= ~PARODD; break;
	default:	qWarning("Invalid parity: %d", parity);
	}

	if (tcsetattr(handle(), TCSANOW, &state) < 0)
	    qWarning("Setting termios state failed: %d", errno);
    }
}

void
SerialPort::setStopBits(int bits)
{
    _stopBits = bits;
    if (isOpen()) {
	struct termios state;
	if (tcgetattr(handle(),&state) < 0)
	    qWarning("Getting termios state failed: %d", errno);

	switch (bits) {
	case 1:	 state.c_cflag &= ~CSTOPB; break;
	case 2:	 state.c_cflag |= CSTOPB; break;
	default: qWarning("Invalid stop bits: %d", bits);
	}

	if (tcsetattr(handle(), TCSANOW, &state) < 0)
	    qWarning("Setting termios state failed: %d", errno);
    }
}

void
SerialPort::setHandshake(int mode)
{
    _handshake = mode;
    if (isOpen()) {
	struct termios state;
	if (tcgetattr(handle(),&state) < 0)
	    qWarning("Getting termios state failed: %d", errno);

	switch (mode) {
	case HS_NONE:
	    state.c_cflag &= ~CRTSCTS;
	    state.c_iflag &= ~IXON;
	    state.c_iflag &= ~IXOFF;
	    break;
	case HS_XON_XOFF:
	    state.c_cflag &= ~CRTSCTS;
	    state.c_iflag |= IXON;
	    state.c_iflag |= IXOFF;
	    break;
	case HS_RTS_CTS:
	    state.c_cflag |= CRTSCTS;
	    state.c_iflag &= ~IXON;
	    state.c_iflag &= ~IXOFF;
	    break;
	default:
	    qWarning("Invalid handshake: %d", mode);
	}

	if (tcsetattr(handle(), TCSANOW, &state) < 0)
	    qWarning("Setting termios state failed: %d", errno);
    }
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
    setBaud(baud);
    setDataBits(dataBits);
    setParity(parity);
    setStopBits(stopBits);
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
    _timeout = timeout;
    if (isOpen()) {
	struct termios state;
	if (tcgetattr(handle(),&state) < 0)
	    qWarning("Getting termios state failed: %d", errno);

	state.c_cc[VMIN] = 0;
	state.c_cc[VTIME] = timeout * 10;

	if (tcsetattr(handle(), TCSANOW, &state) < 0)
	    qWarning("Setting termios state failed: %d", errno);
    }
}

int
SerialPort::getTimeout()
{
    return _timeout;
}

int
convertBits(int bits)
{
    int termBits = 0;
    if (bits & SerialPort::DTR) termBits |= TIOCM_DTR;
    if (bits & SerialPort::RTS) termBits |= TIOCM_RTS;
    return termBits;
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
	int termBit = convertBits(bit);
	if (value) {
	    if (ioctl(handle(), TIOCMBIS, &termBit) < 0)
		qWarning("Setting status bit failed: %d", errno);
	} else {
	    if (ioctl(handle(), TIOCMBIC, &termBit) < 0)
		qWarning("Clearing status bit failed: %d", errno);
	}
    }
}

int
SerialPort::getStatus()
{
    if (!isOpen()) return 0;

    int termStatus = 0;
    if (ioctl(handle(), TIOCMGET, &termStatus) < 0)
	qWarning("Getting status bits failed: %d", errno);

    int status = 0;
    if (termStatus & TIOCM_CD)  status |= DCD;
    if (termStatus & TIOCM_DSR) status |= DSR;
    if (termStatus & TIOCM_CTS) status |= CTS;
    if (termStatus & TIOCM_RNG) status |= RNG;

    return status;
}

void
SerialPort::setStatus(int status)
{
    if (isOpen()) {
	int termBits = convertBits(status);
	if (ioctl(handle(), TIOCMSET, &termBits) < 0)
	    qWarning("Setting status bits failed: %d", errno);
    }
}

void
SerialPort::drain()
{
    if (isOpen()) {
	tcdrain(handle());
    }
}
