#include "common.h"
#include "scheduler_impl.h"
#include "scheduler.h"
#include "guard.h"

// 工作线程T
vector<WorkThread*> work_threads_;

// 服务S
RwLock service_lock_; // 读写锁
Service** service_list_; 
int g_service_size_ = 1024;

Mutex cursor_lock_; // 游标锁
uint32 service_cursor_ = 0;
uint32 service_thread_cursor_ = 0;

void PushServiceMsg(Service* service, Message* message)
{
	service->msg_lock.Lock();
	service->msg_lists.push_back(message);
	service->msg_lock.UnLock();
}

Message* PopServiceMsg(Service* service)
{
	Message* message = NULL;
	service->msg_lock.Lock();
	if (service->msg_lists.size() > 0)
	{
		message = service->msg_lists.at(0);
		service->msg_lists.pop_front();
	}
	service->msg_lock.UnLock();

	return message;
}


Scheduler_impl::Scheduler_impl()
{
	service_cursor_ = 0;
	service_list_ = new Service*[g_service_size_];
	memset(service_list_, 0, g_service_size_ * sizeof(Service*));
}

Scheduler_impl::~Scheduler_impl()
{
	delete[] service_list_;
}


bool Scheduler_impl::startup()
{
	//创建WorkThread
	SpawnWorkerThreads();

	//创建UpdateThread
	updateThread_ = new UpdateThread();
	updateThread_->Activate();

	return true;
}

void Scheduler_impl::SpawnWorkerThreads()
{
	uint32 threadcount = 0;

#ifdef WIN32
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	threadcount = si.dwNumberOfProcessors;
#else
	threadcount = get_nprocs();
#endif

	threadcount = 1;

	for (uint32 n = 0; n < threadcount; ++n)
	{
		WorkThread *work_thread = new WorkThread(n);
		string name = "work_thread" + (n + 1);
		work_thread->set_name(name);
		work_threads_.push_back(work_thread);
		work_thread->Activate();
	}
}

bool Scheduler_impl::shutdown()
{
	return true;
}

Service* Scheduler_impl::new_service()
{
	Service* service = new Service();

	service_lock_.WriteLock(); //用来互斥service_cursor_, service_list_

	if (service->thread_id == INVALID_INDEX)
	{
		service->thread_id = service_thread_cursor_;
		service_thread_cursor_++;
		if (service_thread_cursor_ >= work_threads_.size())
		{
			service_thread_cursor_ = 0;
		}
	}

	for(int i=0; i<g_service_size_; i++)
	{
		int index = (service_cursor_ + i) % g_service_size_;
		if (!service_list_[index])
		{
			service->id = index;
			service_list_[index] = service;
			service->AddRef();

			service_cursor_ = (index + 1) % g_service_size_;

			service_lock_.WriteUnLock();

			return service;
		}
	}
	
	int new_service_size_ = g_service_size_ * 2; //扩展到2倍的空间
	Service** new_service_list = new Service*[new_service_size_];
	memset(new_service_list, 0, new_service_size_ * sizeof(Service*));
	memcpy(new_service_list, service_list_, g_service_size_ * sizeof(Service*));
	delete[] service_list_;
	
	// 当前g_service_size_就是要寻找的下标
	service_cursor_ = g_service_size_;
	
	service_list_ = new_service_list;
	g_service_size_ = new_service_size_;

	service->id = service_cursor_;
	service_list_[service_cursor_] = service;
	service->AddRef();

	service_cursor_ = (service_cursor_ + 1) % g_service_size_;

	service_lock_.WriteUnLock();

	return service;
}

void Scheduler_impl::delete_service( uint32 service_id )
{
	service_lock_.WriteLock();
	Service* service = service_list_[service_id];
	if (!service)
	{
		return;
	}

	bool ret = service->Release();
	if (ret)
	{
		service_list_[service_id] = NULL;
		service_lock_.WriteUnLock();

		delete service;
		service = NULL;
	}
	else
	{
		service_lock_.WriteUnLock();
	}
}

void Scheduler_impl::send(uint32 service_id, const char* data, uint32 data_len)
{
	service_lock_.ReadLock();
	Service* service = service_list_[service_id];
	if (!service)
	{
		service_lock_.ReadUnLock();
		return;
	}

	service->AddRef(); // 引用消息
	service_lock_.ReadUnLock();
	//-----------------------------------------------------

	Message* message = new Message();
	message->data = new char[data_len];
	memcpy(message->data, data, data_len);

	PushServiceMsg(service, message);

	WorkThread* workThread = work_threads_[service->thread_id];

	bool isWakeUp = false;
	service->post_lock.Lock();
	if (!service->is_post) 
	{
		service->is_post = true;	
		// 压进线程队列
		workThread->PushService(service);

		isWakeUp = true;
	}
	service->post_lock.UnLock();

	if (isWakeUp)
	{
		// 尝试激活线程
		workThread->WakeUp();
	}
}

bool StealService(WorkThread* work_thread)
{
	bool is_success = false;
	uint32 max_service_size = 0;
	int sel_thread_id = 0;

	// 找出最多服务的线程(不包括自己)
	for (uint32 n=0; n < work_threads_.size(); n++)
	{
		WorkThread* p = work_threads_[n];
		if (p->is_block_)
		{
			continue;
		}

		if (p == work_thread)
		{
			continue;
		}

		uint32 service_size = 0;
		p->post_mutux_.Lock();
		service_size = p->post_service_list_.size();
		p->post_mutux_.UnLock();

		if (service_size > max_service_size)
		{
			max_service_size = service_size;
			sel_thread_id = n;
		}
	}

	if (max_service_size > 1)
	{
		assert(work_thread->get_thread_id() != sel_thread_id);

		Service* service = NULL;

		for (uint32 i=0; i < max_service_size/2; i++) //偷一半的Service
		{
			service = work_threads_[sel_thread_id]->PopService();
			if (service) //成功
			{
				service->post_lock.Lock();
				service->thread_id = work_thread->get_thread_id();
				work_thread->PushService(service);

				service->post_lock.UnLock();

				is_success = true;
			}
			else
			{
				break;
			}
		}
	}

	return is_success;
}

UpdateThread::UpdateThread()
{

}

UpdateThread::~UpdateThread()
{

}

bool UpdateThread::Run()
{
	while(is_running_)
	{
		for (int i=0; i < (int)work_threads_.size(); i++)
		{
			WorkThread *work_thread = work_threads_[i];
			if (work_thread->is_block_)
			{
				bool ret = StealService(work_thread);
				if (ret)
				{
					work_thread->WakeUp();
				}
			}
		}

		SleepMs(10);
	}

	return true;
}

WorkThread::WorkThread(uint32 thread_id)
{
	thread_id_ = thread_id;
	is_block_ = true;
	weight_ = 1;
	post_queue_size_ = 0;

	pthread_mutex_init(&mutex_, NULL);
	pthread_cond_init(&cond_, NULL);
}

WorkThread::~WorkThread()
{
	
}

uint32 WorkThread::get_thread_id()
{
	return thread_id_;
}

void WorkThread::set_weight(int weight)
{
	weight_ = weight;
}

void WorkThread::PushService(Service* service)
{
	post_mutux_.Lock();
	post_service_list_.push_back(service);
	post_queue_size_ = post_service_list_.size();
	post_mutux_.UnLock();
}

Service* WorkThread::PopService()
{
	Service* service = NULL;

	post_mutux_.Lock();
	//assert(post_queue_size_ == post_service_list_.size());
	if (post_service_list_.size() > 0)
	{
		service = post_service_list_.at(0);
		post_service_list_.pop_front();
		post_queue_size_ = post_service_list_.size();
	}
	post_mutux_.UnLock();

	return service;
}

bool WorkThread::ProcessServiceMsgs(Service* service, int weight, int thread_id)
{
	bool is_destroy = false;
	int haveMsgCount = 0;

	Message* message = NULL;
	while(message = PopServiceMsg(service))
	{
		// 回调服务的回调函数
		service->handler(message->data, message->data_len, thread_id);

		delete message;
		message = NULL;

		is_destroy = service->Release();
		if (is_destroy)
		{
			break;
		}
	}

	return is_destroy;
}

extern int32 block_count;

void WorkThread::WorkerWait()
{
	pthread_mutex_lock(&mutex_);
	is_block_ = true;
	block_count++;
	pthread_cond_wait(&cond_, &mutex_);
	is_block_ = false;
	pthread_mutex_unlock(&mutex_);
}

extern int32 wakeup_count;

void WorkThread::WakeUp()
{
	pthread_mutex_lock(&mutex_);
	if (is_block_)
	{
		//printf("pthread_cond_signal\n");

		wakeup_count++;
		pthread_cond_signal(&cond_);
		is_block_ = false;
	}
	pthread_mutex_unlock(&mutex_);
}

bool WorkThread::Run()
{
	Service *service = NULL;

	while(is_running_)
	{
		Service *service = PopService();
		if (!service)
		{
			// galen: 去偷其他线程的Service
			//printf("StealService \n");
 			bool is_success = StealService(this);
 			if (!is_success)
 			{
 				WorkerWait();
 			}
		}
		else
		{
			bool is_destory = ProcessServiceMsgs(service, weight_, thread_id_); //处理消息
			if (is_destory)
			{
				service_lock_.WriteLock();
				service_list_[service->id] = NULL;
				service_lock_.WriteUnLock();

				delete service;
			}
			else
			{
				// 看service的队列中是否还有消息, 如果有的话继续投递
				service->msg_lock.Lock();
				if (service->msg_lists.size() > 0)
				{
					PushService(service); // 把它插入到队尾
				}
				else
				{
					service->msg_lock.UnLock();
					service->is_post = false;
					service->post_lock.UnLock();
				}
				service->msg_lock.UnLock();
			}
		}
	}

	return true;
}
