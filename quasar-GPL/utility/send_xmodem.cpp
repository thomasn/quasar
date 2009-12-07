// $Id: send_xmodem.cpp,v 1.10 2005/03/16 20:33:40 bpepers Exp $
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

#include "send_xmodem.h"
#include "ymodem.h"
#include "quasar_misc.h"

#include <qapplication.h>
#include <qfileinfo.h>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <stdio.h>
#include <string.h>

/*
 *  Crc calculation stuff
 */
/* crctab calculated by Mark G. Mendel, Network Systems Corporation */
static unsigned short crctab[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
};

/*
 * updcrc macro derived from article Copyright (C) 1986 Stephen Satchell. 
 *  NOTE: First srgument must be in range 0 to 255.
 *        Second argument is referenced twice.
 * 
 * Programmers may incorporate any or all code into their programs, 
 * giving proper credit within the source. Publication of the 
 * source routines is permitted so long as proper credit is given 
 * to Stephen Satchell, Satchell Evaluations and Chuck Forsberg, 
 * Omen Technology.
 */

#define updateCRC(cp, crc) ( crctab[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)

//======================================================================
//	Support routines for communications
//======================================================================
static void
sendChar(SerialPort& port, unsigned char c)
{
    //qDebug("send char: %d", int(c));
    port.writeBlock((char*)&c, 1);
}

static void
sendBytes(SerialPort& port, unsigned char* str, int len)
{
    //qDebug("send bytes: %d", len);
    port.writeBlock((char*)str, len);
}

static int
recvChar(SerialPort& port, int timeout)
{
    static char line[BLOCK_LEN];
    static char* buffer;
    static long numChars = 0;

    // Read in to buffer if needed
    if (numChars <= 0) {
	buffer = line;
	port.setTimeout(timeout);
	//qDebug("try to read %d chars (%d)", BLOCK_LEN, timeout);
	numChars = port.readBlock(buffer, BLOCK_LEN);
	//qDebug("read %ld chars", numChars);
    }

    // If still no data then return timeout
    if (numChars <= 0) return TIMEOUT;

    int ch = *buffer++ & 0377;
    --numChars;

    //qDebug("received char: %d", ch);
    return ch;
}

//======================================================================
//	Send a block.  Handles different block lengths.
//======================================================================
static int
sendBlock(SerialPort& port, unsigned char* block, int blockNum, bool useCRC)
{
    //qDebug("send block %d (%d)", blockNum, useCRC);
    for (int errorCnt = 0; errorCnt < RETRY_MAX; errorCnt++) {
	sendChar(port, SOH);
	sendChar(port, blockNum & 0xff);
	sendChar(port, ~blockNum & 0xff);
	sendBytes(port, block, BLOCK_LEN);

	if (useCRC) {
	    int crc = 0;
	    for (int pos = 0; pos < BLOCK_LEN; pos++)
		crc = updateCRC((0377 & block[pos]), crc);
	    crc = updateCRC(0, updateCRC(0, crc));
	    sendChar(port, (crc >> 8) & 0xff);
	    sendChar(port, crc & 0xff);
	} else {
	    int checksum = 0;
	    for (int pos = 0; pos < BLOCK_LEN; pos++)
		checksum += block[pos];
	    sendChar(port, checksum & 0xff);
	}

	while (1) {
	    int ch = recvChar(port, 2);

	    if (ch == ACK)
	        return OK;
	    if (ch == CAN)
	        return ERROR;
	    if (ch == NAK)
	        break;
	}
    }

    return ERROR;
}

//======================================================================
//	Opens the file and sends a header block (0)
//======================================================================
static int
sendHeader(QFile& file, SerialPort& port, bool useCRC)
{
    //qDebug("send header block");
    QString fileName = QFileInfo(file).fileName();
    unsigned char block[BLOCK_LEN];

    memset(block, 0, BLOCK_LEN);
    sprintf((char*)block, "%s%c%ld", fileName.latin1(), 0, file.size());

    return sendBlock(port, block, 0, useCRC);
}

//======================================================================
//	Send the data of a file
//======================================================================
static int
sendData(QFile& file, SerialPort& port, bool useCRC, QProgressDialog* dialog)
{
    unsigned char block[BLOCK_LEN];
    long fileSize= file.size();
    int blockNum = 1;
    //qDebug("send data blocks: %ld", fileSize);

    file.reset();
    do {
	if (dialog != NULL) {
	    dialog->setProgress((blockNum - 1) * BLOCK_LEN);
	    qApp->processEvents();
	    if (dialog->wasCancelled()) {
		QString message = "Sending items cancelled";
		QMessageBox::information(dialog->parentWidget(), "Cancelled",
					 message);
		return ERROR;
	    }
	}

        memset(block, 0, BLOCK_LEN);
        file.readBlock((char*)block, BLOCK_LEN);

	if (sendBlock(port, block, blockNum++, useCRC) != OK)
	    return ERROR;

	fileSize -= BLOCK_LEN;
    } while (fileSize > 0);

    for (int errorCnt = 0; errorCnt < RETRY_MAX; errorCnt++) {
        sendChar(port, EOT);
        int ch = recvChar(port, 2);

	if (ch == ACK)
	    return OK;
	if (ch == CAN)
	    return ERROR;
    }

    return ERROR;
}

//======================================================================
// Wait for the 'C' or NAK to tell us to start sending.  Will wait for
// up to 20 seconds or 10 failed characters.  Returns true if we got a
// proper 'C' or NAK.
//======================================================================
static int
sendWait(SerialPort& port, bool* useCRC=NULL)
{
    //qDebug("wait for start character");
    for (int cnt = 0; cnt < 10; cnt++) {
        int ch = recvChar(port, 2);
	if (ch == NAK || ch == 'C') {
	    if (useCRC != NULL)
		*useCRC = (ch == 'C');
	    return OK;
	}
    }

    return ERROR;
}

//======================================================================
//	Send a blank block at the end of the file transfer
//======================================================================
static int
sendBlank(SerialPort& port, bool useCRC)
{
    //qDebug("send ending blank block");
    if (sendWait(port) != OK)
	return ERROR;

    unsigned char block[BLOCK_LEN];
    memset(block, 0, BLOCK_LEN);
    return sendBlock(port, block, 0, useCRC);
}

//======================================================================
//	Main code
//======================================================================
int
sendXModem(QFile& file, SerialPort& port, QProgressDialog* dialog)
{
    // First wait for starting signal
    bool useCRC = false;
    if (sendWait(port, &useCRC) != OK) {
	qWarning("Error: waiting for transfer start failed");
	return ERROR;
    }

    // First block (number zero) is the filename
    if (sendHeader(file, port, useCRC) != OK) {
	qWarning("Error: sending header block failed");
        return ERROR;
    }

    // Now send the data blocks
    if (sendData(file, port, useCRC, dialog) != OK) {
	qWarning("Error: sending data blocks failed");
        return ERROR;
    }

    // Final block is a blank one
    if (sendBlank(port, useCRC) != OK) {
	qWarning("Error: sending final block failed");
	return ERROR;
    }

    // Flush serial port and wait a bit to make sure all is done
    port.flush();

    return 0;
}
