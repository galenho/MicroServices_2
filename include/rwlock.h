/*
* 3D MMORPG Server
* Copyright (C) 2009-2014 RedLight Team
* author: 
*/

#ifndef _RWLOCK_H
#define _RWLOCK_H

#include <pthread.h>

class RwLock
{
public:
	RwLock();
	~RwLock();

	void ReadLock();
	void ReadUnLock();

	void WriteLock();
	void WriteUnLock();

private:
	pthread_rwlock_t lock;
};

#endif //_RWLOCK_H
