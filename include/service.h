/*
* 3D MMORPG Server
* Copyright (C) 2009-2014 RedLight Team
* author: 
*/

#ifndef _SERVICE_HEAD_H
#define _SERVICE_HEAD_H

#include <queue>
#include "event.h"

typedef Function<void (char*, uint32, uint32)> service_handler_type;


//
// ��Ϣ
//
struct Message 
{
	uint32	data_len;
	char*	data;

	Message()
	{
		data_len = 0;
		data = NULL;
	}

	~Message()
	{
		if (data)
		{
			delete[] data;
			data = NULL;
		}
	}
};

//
// ����
//
struct Service
{
	uint32 id; // ����ID
	//ModuleBase* mod;

	deque<Message*> msg_lists; // ��Ϣ����
	Mutex msg_lock;

	Mutex post_lock;	// Ͷ����
	bool is_post;		// �Ƿ��Ѿ�Ͷ��

#ifdef WIN32
	volatile long ref_count; //���ô���
#else
	volatile unsigned int ref_count;
#endif

	service_handler_type handler;
	uint32 thread_id;

	uint32 recv_msg_count;
	uint32 send_msg_count;

	Service()
	{
		id = 0;
		is_post = false;
		ref_count = 0;
		recv_msg_count = 0;
		send_msg_count = 0;

		thread_id = INVALID_INDEX;
	}

	inline void AddRef()
	{
#ifdef WIN32
		InterlockedIncrement(&ref_count);
#else
		__sync_add_and_fetch(&ref_count, 1);
#endif
	}

	inline bool Release()
	{
#ifdef WIN32
		InterlockedDecrement(&ref_count);
#else
		__sync_sub_and_fetch(&ref_count, 1);
#endif

		if (ref_count == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	inline void set_callback_handler( service_handler_type service_handle )
	{
		handler = service_handle;
	}
};


#endif //_SERVICE_HEAD_H

