/*
* 3D MMORPG Server
* Copyright (C) 2009-2014 RedLight Team
* author: 
*/

#ifndef _GUARD_H_
#define _GUARD_H_

#include "mutex.h"

/************************************************************************/
/* Guard class, unlocks mutex on destroy								*/
/************************************************************************/

class Guard
{
public:
	Guard(Mutex& mutex) : target(mutex)
	{
		target.Lock();
	}

	~Guard()
	{
		target.UnLock();
	}

	Guard& operator=(Guard& src)
	{
		this->target = src.target;
		return *this;
	}

protected:
	Mutex& target;
};

#endif //_GUARD_H_

