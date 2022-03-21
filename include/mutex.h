/*
* 3D MMORPG Server
* Copyright (C) 2009-2014 RedLight Team
* author: 
*/

#ifndef _MUTEX_H
#define _MUTEX_H																		

#include <pthread.h>

class Mutex
{
public:
	Mutex();
	~Mutex();

	void Lock();
	void UnLock();

protected:
	pthread_mutex_t mutex;
};

#endif //_MUTEX_H

