// $Id: shared_state.cpp,v 1.2 2004/12/22 07:10:19 bpepers Exp $
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

#include "shared_state.h"

#include <assert.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>

#define IPC_KEY 0xBAADFEED
#define CLIENT_MAX 128
#define ADDRESS_SIZE 16
#define COMPANY_SIZE 256

struct ClientInfo {
    pid_t pid;
    int flags;
    char address[ADDRESS_SIZE];
    char company[COMPANY_SIZE];
};

struct MemoryLayout {
    pid_t quasar_posd_pid;
    ClientInfo clients[CLIENT_MAX];
};

SharedState::SharedState()
    : _semid(-1), _memory(NULL)
{
}

SharedState::~SharedState()
{
    if (_memory != NULL) {
	shmdt(_memory);
	_memory = NULL;
    }
}

bool
SharedState::initialize()
{
    if (_memory != NULL) return true;

    // Get the semaphore
    bool initSemaphore = false;
    _semid = semget(IPC_KEY, 1, 0777);
    if (_semid == -1 && errno == ENOENT) {
	_semid = semget(IPC_KEY, 1, IPC_CREAT | 0777);
	initSemaphore = true;
    }
    if (_semid == -1) return false;

    // Initialize semaphore to 1 if needed
    if (initSemaphore) {
	struct sembuf sop;
	sop.sem_num = 0;
	sop.sem_op  = 1;
	sop.sem_flg = 0;
	if (semop(_semid, &sop, 1) == -1)
	    return false;
    }

    // Get the shared memory
    bool initMemory = false;
    int size = (sizeof(MemoryLayout) + 4095) & ~4095;
    int id = shmget(IPC_KEY, size, 0666);
    if (id == -1 && errno == ENOENT) {
	id = shmget(IPC_KEY, size, IPC_CREAT | 0666);
	initMemory = true;
    }
    if (id == -1) return false;

    // Attach to shared memory
    _memory = shmat(id, NULL, 0);
    if (_memory == NULL) return false;

    // Initialize memory if needed
    if (initMemory) {
	MemoryLayout* memory = (MemoryLayout*)_memory;
	memory->quasar_posd_pid = 0;
	for (unsigned int i = 0; i < CLIENT_MAX; ++i) {
	    ClientInfo& client = memory->clients[i];
	    client.pid = 0;
	}
    }

    return true;
}

bool
SharedState::lock()
{
    if (!initialize()) return false;

    struct sembuf sop;
    sop.sem_num = 0;
    sop.sem_op  = -1;
    sop.sem_flg = SEM_UNDO;
    if (semop(_semid, &sop, 1) == -1)
	return false;

    return true;
}

bool
SharedState::release()
{
    assert(_memory != NULL);

    struct sembuf sop;
    sop.sem_num = 0;
    sop.sem_op  = 1;
    sop.sem_flg = SEM_UNDO;
    if (semop(_semid, &sop, 1) == -1)
	return false;

    return true;
}

pid_t
SharedState::getPosPid()
{
    assert(_memory != NULL);
    MemoryLayout* memory = (MemoryLayout*)_memory;

    pid_t pid = memory->quasar_posd_pid;
    if (pid != 0 && kill(pid, 0) == -1 && errno == ESRCH)
	memory->quasar_posd_pid = 0;

    return memory->quasar_posd_pid;
}

void
SharedState::setPosPid(pid_t pid)
{
    assert(_memory != NULL);
    MemoryLayout* memory = (MemoryLayout*)_memory;
    memory->quasar_posd_pid = pid;
}

bool
SharedState::getClients(QStringList& clients, QStringList& companies)
{
    assert(_memory != NULL);
    clients.clear();
    companies.clear();

    MemoryLayout* memory = (MemoryLayout*)_memory;
    for (unsigned int i = 0; i < CLIENT_MAX; ++i) {
	ClientInfo& client = memory->clients[i];
	if (client.pid == 0) continue;

	// Check if client has died without cleanup
	if (kill(client.pid, 0) == -1 && errno == ESRCH) {
	    client.pid = 0;
	    continue;
	}

	clients.push_back(client.address);
	companies.push_back(client.company);
    }

    return true;
}

bool
SharedState::addClient(pid_t pid, const QString& addr, const QString& company)
{
    assert(_memory != NULL);
    MemoryLayout* memory = (MemoryLayout*)_memory;
    for (unsigned int i = 0; i < CLIENT_MAX; ++i) {
	ClientInfo& client = memory->clients[i];
	if (client.pid != 0) continue;

	client.pid = pid;
	client.flags = 0;
	strncpy(client.address, addr, ADDRESS_SIZE);
	strncpy(client.company, company, COMPANY_SIZE);
	return true;
    }
    return false;
}

bool
SharedState::removeClient(pid_t pid)
{
    assert(_memory != NULL);
    MemoryLayout* memory = (MemoryLayout*)_memory;
    for (unsigned int i = 0; i < CLIENT_MAX; ++i) {
	ClientInfo& client = memory->clients[i];
	if (client.pid != pid) continue;

	client.pid = 0;
	return true;
    }
    return false;
}
