#include "task_base.h"


typedef void* (*tc_thread_func)(void*);

void _Run_( void *pArguments )
{
	TaskBase* p =(TaskBase*)pArguments;
	if(!p)
		return;

	p->Run();
}

TaskBase::TaskBase()
{
	is_running_ = false;
	is_high_priority_ = false;
}

void TaskBase::Activate()
{
	is_running_ = true;

	if (is_high_priority_)
	{
		pthread_attr_t thread_attr;
		struct sched_param thread_param;
		int status, rr_min_priority, rr_max_priority;
		pthread_attr_init(&thread_attr);

		status = pthread_attr_setschedpolicy(&thread_attr, SCHED_RR);
		if(status != 0)
		{
			printf("Unable to set SCHED_RR policy.\n");
		}
		else
		{
			rr_min_priority = sched_get_priority_min(SCHED_RR);

			if(rr_min_priority == -1)
			{
				printf("Get SCHED_RR min priority");
			}

			rr_max_priority = sched_get_priority_max(SCHED_RR);

			if(rr_max_priority == -1)
			{
				printf("Get SCHED_RR max priority");
			}

			thread_param.sched_priority = (rr_min_priority + rr_max_priority) / 2;
			pthread_attr_setschedparam(&thread_attr, &thread_param);
			pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);
		}

		pthread_create(&id,&thread_attr,(tc_thread_func)_Run_,this);
	}
	else
	{
		pthread_create(&id,NULL,(tc_thread_func)_Run_,this);
	}

}

bool TaskBase::Run()
{
	return true;
}

void TaskBase::Shutdown()
{
	//pthread_cancel(new_thread);
	//pthread_join(new_thread, NULL);
}

void TaskBase::set_name( string name )
{
	thread_name_ = name;
}

void TaskBase::set_thread_priority_highest()
{
	is_high_priority_ = true;
}
