// $Id: recv_xmodem.cpp,v 1.5 2004/12/09 21:42:54 bpepers Exp $
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

#include "recv_xmodem.h"
#include "ymodem.h"
#include "quasar_misc.h"

#include <stdio.h>
#include <string.h>

//======================================================================
//	Global Variables
//======================================================================
static FILE* outputFile;
static char blockBuf[MAX_BLOCK_LEN];
static int blockLen = 128;
static long bytesLeft;
static int exitCode = 0;

//======================================================================
//	Support routines for communications
//======================================================================
static void
sendChar(SerialPort& port, unsigned char c)
{
    port.writeBlock((char*)&c, 1);
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
	numChars = port.readBlock(buffer, BLOCK_LEN);
    }

    // If still no data then return timeout
    if (numChars <= 0) return TIMEOUT;

    int ch = *buffer++ & 0377;
    --numChars;

    return ch;
}

static void
flushInput(SerialPort& port)
{
    int ch;
    do {
	ch = recvChar(port, 2);
    } while (ch != TIMEOUT);
}

//======================================================================
//	Get a whole block.  Handles checksums, ...
//======================================================================
static int
getBlock(SerialPort& port, char* block)
{
    int checksum;
    int ch;
    int blockNum;

    for (int errorCnt = 0; errorCnt < RETRY_MAX; errorCnt++) {
	ch = recvChar(port, 2);
	if (ch == EOT) {
	    sendChar(port, ACK);
	    return GOT_EOT;
	}

	if (ch == CAN) {
	    exitCode = 7;
	    return ERROR;
	}

	if (ch == SOH || ch == STX) {
	    if (ch == SOH)
	        blockLen = 128;
	    else
	        blockLen = 1024;

	    blockNum = recvChar(port, 2);
	    ch = recvChar(port, 2);
	    if (blockNum + ch == 0377) {
		checksum = 0;
		char* p = block;
		for (int cnt = 0; cnt < blockLen; cnt++) {
		    ch = recvChar(port, 2);
		    checksum += (*p++ = ch);
		}

		ch = recvChar(port, 2);
		if (((checksum - ch) & 0377) == 0)
		    return blockNum;
		exitCode = 8;
	    }
	}

	if (ch == TIMEOUT)
	    exitCode = 9;

	flushInput(port);
	sendChar(port, NAK);
    }

    return ERROR;
}

//======================================================================
//	Get a header record.  If its not an empty one, open the output
//	file.
//======================================================================
static int
header(SerialPort& port)
{
    // Get a header sector (sector 0)
    sendChar(port, NAK);
    int block = getBlock(port, blockBuf);
    if (block == ERROR)
        return ERROR;
    if (block != 0) {
        exitCode = 3;
	return ERROR;
    }
    sendChar(port, ACK);

    // If its a empty header, just return
    if (blockBuf[0] == 0)
	return OK;

    // Get the number of bytes in the file
    bytesLeft = 2000000000L;
    char* ptr = blockBuf + 1 + strlen(blockBuf);
    if (*ptr)
	sscanf(ptr, "%ld", &bytesLeft);

    // Try to open the filename
    outputFile = fopen(blockBuf, "w");
    if (outputFile == NULL) {
        exitCode = 4;
	return ERROR;
    }

    printf("Receiving %s (%ld)\n", blockBuf, bytesLeft);

    return OK;
}

//======================================================================
//	Receive the data sectors and write them out
//======================================================================
static int
data(SerialPort& port)
{
    int blockCurr;
    int blockNum = 0;
    int sendchar = NAK;

    for (;;) {
	sendChar(port, sendchar);
	blockCurr = getBlock(port, blockBuf);

	if (blockCurr == (blockNum + 1 & 0377)) {
	    blockNum++;
	    int len = (bytesLeft > blockLen) ? blockLen : bytesLeft;
	    fwrite(blockBuf, 1, len, outputFile);
	    bytesLeft -= len;
	    if (bytesLeft < 0)
		bytesLeft = 0;
	    sendchar = ACK;
	} else if (blockCurr == (blockNum & 0377)) {
	    sendchar = ACK;
	} else if (blockCurr == GOT_EOT) {
	    if (fclose(outputFile) != 0) {
	        exitCode = 5;
		return ERROR;
	    }
	    if (bytesLeft != 0) {
	        exitCode = 6;
		return ERROR;
	    }
	    return OK;
	} else if (blockCurr == ERROR)
	    return ERROR;
	else
	    return ERROR;
    }
}

//======================================================================
//	Receive a file
//======================================================================
static void
receive(SerialPort& port)
{
    flushInput(port);

    while (true) {
	if (header(port) != OK)
	    break;

	// Empty header so must be done!
	if (blockBuf[0] == 0) {
	    exitCode = 0;
	    break;
	}

	// Get the data sectors
	if (data(port) != OK) {
	    break;
	}
    }
}

//======================================================================
//	Main code
//======================================================================
int
recvXModem(SerialPort& port)
{
    receive(port);
    qsleep(2);
    return exitCode;
}
