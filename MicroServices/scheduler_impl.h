#ifndef _SHEDULER_IMPL_H_
#define _SHEDULER_IMPL_H_

#include "task_base.h"
#include "mutex.h"
#include "rwlock.h"
#include "timer.h"
#include "safe_queue.h"
#include "service.h"

#define MAX_SERVICE_COUNT 10240

#include <pthread.h>

//
// 更新线程
//
class UpdateThread : public TaskBase
{
public:
	UpdateThread();
	virtual ~UpdateThread();

protected:
	virtual bool Run();
};

//
// 工作线程
//
class WorkThread : public TaskBase
{
public:
	WorkThread(uint32 thread_id);
	virtual ~WorkThread();
	
	uint32 get_thread_id();
	void set_weight(int weight);

	void PushService(Service* service);
	Service* PopService();
	
	bool ProcessServiceMsgs(Service* service, int weight, int thread_id);

	void WorkerWait();
	void WakeUp();

protected:
	virtual bool Run();

public:
	// 投递的服务列表
	deque<Service*> post_service_list_;
	Mutex post_mutux_;

	pthread_mutex_t mutex_;
	pthread_cond_t cond_;
	bool is_block_;

	int post_queue_size_;

private:
	uint32 thread_id_;
	int weight_;
};

class Scheduler_impl
{
public:
	Scheduler_impl();
	virtual ~Scheduler_impl();

	bool startup();
	bool shutdown();

	Service* new_service();
	void delete_service(uint32 service_id);
	void send(uint32 service_id, const char* data, uint32 data_len);

private:
	void SpawnWorkerThreads();

private:
	UpdateThread* updateThread_;
};

#endif //_SHEDULER_IMPL_H_