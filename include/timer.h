/*
* 3D MMORPG Server
* Copyright (C) 2009-2014 RedLight Team
* author: 
*/

#ifndef _TIMER_H_
#define _TIMER_H_

#include "common.h"

#ifndef WIN32
#include <sys/timeb.h>
#endif

// 这个函数只能支持开机运行48天, 48后会清0重新开始
// 所以只能用来计算时间差, 不能记录于DB
#ifdef WIN32
__forceinline uint32 getMSTime()
{
	uint32 now_time = 0;

	timeBeginPeriod(1);
	now_time = timeGetTime();
	timeEndPeriod(1);

	return now_time;
}
#else
inline uint32 getMSTime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (uint32)((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
}
#endif

class IntervalTimer
{
public:
	IntervalTimer(uint32 index, uint32 cur_time, uint32 interval, timer_handler_type handler)
	{
		index_ = index;
		interval_ = interval;
		last_time_ = cur_time;
		handler_ = handler;
		is_delete_ = false;
	}

	void Update(uint32 cur_time)
	{
		if ((int32)(cur_time - last_time_) >= (int32)interval_)
		{
			last_time_ = cur_time;

			handler_(index_); //回调
		}
	}

	uint32 get_index()
	{
		return index_;
	}

	bool IsDelete()
	{
		return is_delete_;
	}

	void Delete()
	{
		is_delete_ = true;
	}

private:
	uint32 index_;
	uint32 interval_;
	uint32 last_time_;
	timer_handler_type handler_;
	bool is_delete_;
};

#endif //_TIMER_H_