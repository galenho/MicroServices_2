/*
* 3D MMORPG Server
* Copyright (C) 2009-2014 RedLight Team
* author: galen
*/
#ifndef _REFERABLE_H_
#define _REFERABLE_H_

class Referable
{
public:
	Referable();
	virtual ~Referable();

	void AddRef();
	bool Release();

private:

#ifdef WIN32
	volatile long count_;
#else
	volatile unsigned int count_;
#endif
}; 

#endif // _REFERABLE_H_
