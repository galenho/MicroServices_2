#ifndef _TASK_BASE_H_
#define _TASK_BASE_H_

#include "common.h"
#include <pthread.h>

class TaskBase
{
public:
	TaskBase();
	virtual ~TaskBase() {}

	void Activate();
	virtual bool Run();
	virtual void Shutdown();

	void set_name(string name);
	void set_thread_priority_highest();

protected:
	bool is_running_;

	bool is_high_priority_;

	pthread_t id;

	string thread_name_;
};

#endif //_TASK_BASE_H_

