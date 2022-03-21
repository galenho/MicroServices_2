/*
* 3D MMORPG Server
* Copyright (C) 2009-2014 RedLight Team
* author: 
*/
#ifndef _SHEDULER_H_
#define _SHEDULER_H_

#include "common.h"
#include "singleton.h"
#include "clog.h"
#include "service.h"

//--------------------------------------------------------------------------
// 线程调度器(用于同一时刻只能由一条活动线程来处理消息)
// 单例, 负责卡住IO线程, Accept线程, 它自己就是Update线程, 负责时钟函数触发
//--------------------------------------------------------------------------
class Scheduler_impl;

typedef Function<void (uint32)> timer_handler_type;
typedef Function<void (void)> post_handler_type;

class Scheduler : public Singleton<Scheduler>
{
public:
	Scheduler();
	~Scheduler();

	void set_log(CLog* log);

	bool startup();
	bool shutdown();

	CLog* get_logger();

	Service* new_service();
	void delete_service(uint32 service_id);
	void send(uint32 service_id, const char* data, uint32 data_len);

private:
	Scheduler_impl *imp_;
	CLog *log_;
};

#define sScheduler Scheduler::get_instance()

#define PRINTF_DEBUG(ftm, ...)	sScheduler->get_logger()->WriteLogFile(CLog::LOG_LEVEL_DEBUG, ftm, ##__VA_ARGS__);
#define PRINTF_INFO(ftm, ...)	sScheduler->get_logger()->WriteLogFile(CLog::LOG_LEVEL_INFO, ftm, ##__VA_ARGS__);
#define PRINTF_WARN(ftm, ...)	sScheduler->get_logger()->WriteLogFile(CLog::LOG_LEVEL_WARN, ftm, ##__VA_ARGS__);
#define PRINTF_ERROR(ftm, ...)	sScheduler->get_logger()->WriteLogFile(CLog::LOG_LEVEL_ERROR, ftm, ##__VA_ARGS__);

#endif //_SHEDULER_H_