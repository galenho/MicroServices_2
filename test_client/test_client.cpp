#include "scheduler.h"
#include "clog.h"
#include "service.h"

CLog* g_logger;
bool is_running_ = true;

// 5000 x 10000条消息6条线程, i5 9400(cpu)
#define SERVICE_COUNT 5000
#define MESSAGE_COUNT 10000

vector<Service*> service_list;
Mutex finish_count_mutex;
int finish_count = 0;
uint32 begin_time = 0;
uint32 end_time = 0;
int32 wakeup_count = 0;
int32 block_count = 0;
//-------------------------------------------------------------------------------------------------
void OnServiceCallBack1(char* data, uint32 len, uint32 thread_id, uint32 service_id)
{
	service_list[service_id]->recv_msg_count++;
	int recv_msg_count = service_list[service_id]->recv_msg_count;
	if (recv_msg_count >= MESSAGE_COUNT)
	{
		finish_count_mutex.Lock();
		finish_count++;
		if (finish_count == SERVICE_COUNT)
		{
			end_time = GetTickCount();
			PRINTF_INFO("service is all finish, use time = %d ms", end_time - begin_time);

			PRINTF_INFO("wakeup_count = %d, one thread block_count = %d", wakeup_count, block_count);
		}
		finish_count_mutex.UnLock();
		
		//PRINTF_INFO("service_id = %d is finish num = %d", service_id, recv_msg_count);
		//g_logger->Save();
	}
}

// 由主线程一次性给5000个服务，每个发10000个包 (43秒左右)
void Test1()
{
	PRINTF_INFO("Test1...");

	char data[32];
	memset(data, 0, 32);
	int* p = (int*)data;
	*p = 1;
	int data_len = 32;

	PRINTF_INFO("message send begin");

	// 创建服务
	for (uint32 n=0; n < SERVICE_COUNT; n++)
	{
		Service* service = sScheduler->new_service();
		//service->thread_id = 0;
		service_list.push_back(service);
		service->set_callback_handler(Bind(&OnServiceCallBack1, _1, _2, _3, service->id));
	}

	// 发送消息
	for (uint32 i=0; i < MESSAGE_COUNT; i++)
	{
		for (uint32 n=0; n < service_list.size(); n++)
		{
			sScheduler->send(service_list[n]->id, data, 32);
		}
	}

	PRINTF_INFO("message send end");
}

//-------------------------------------------------------------------------------------------------

void OnServiceCallBack2(char* data, uint32 len, uint32 thread_id, uint32 service_id)
{
	service_list[service_id]->recv_msg_count++;

	int recv_msg_count = service_list[service_id]->recv_msg_count;
	int send_msg_count = service_list[service_id]->send_msg_count;

	if (recv_msg_count >= MESSAGE_COUNT && send_msg_count >= MESSAGE_COUNT)
	{
		finish_count_mutex.Lock();
		finish_count++;
		if (finish_count == SERVICE_COUNT)
		{
			end_time = GetTickCount();
			PRINTF_INFO("service is all finish, use time = %d ms", end_time - begin_time);

			PRINTF_INFO("wakeup_count = %d, one thread block_count = %d", wakeup_count, block_count);
		}
		finish_count_mutex.UnLock();

		//PRINTF_INFO("service_id = %d is finish num = %d", service_id, send_msg_count);
		//g_logger->Save();
	}
	else
	{
		int next_service_id = service_id + 1;
		if (next_service_id >= SERVICE_COUNT)
		{
			next_service_id = 0;
		}

		sScheduler->send(next_service_id, data, len);

		service_list[service_id]->send_msg_count++;
	}
}

// 测试5000个不同服务之间的发包, 每个发10000个包 (12.4秒左右)
void Test2()
{
	PRINTF_INFO("Test2...");

	char data[32];
	memset(data, 0, 32);
	int* p = (int*)data;
	*p = 1;
	int data_len = 32;

	PRINTF_INFO("message send begin");
	
	// 创建服务
	for (uint32 n=0; n < SERVICE_COUNT; n++)
	{
		Service* service = sScheduler->new_service();
		service_list.push_back(service);
		service->set_callback_handler(Bind(&OnServiceCallBack2, _1, _2, _3, service->id));
	}

	// 发送消息
	for (uint32 i=0; i < 1; i++)
	{
		for (uint32 n=0; n < service_list.size(); n++)
		{
			sScheduler->send(service_list[n]->id, data, 32);
		}
	}

	PRINTF_INFO("message send end");
}

//-------------------------------------------------------------------------------------------------
int main()
{
	srand((unsigned int)time(NULL));

	g_logger = new CLog(CLog::LOG_LEVEL_DEBUG);
	g_logger->set_log_name("client_log.txt");
	g_logger->Start(); //启动日志线程

	sScheduler->set_log(g_logger);
	sScheduler->startup();

	begin_time = GetTickCount();

	Test1();
	//Test2();

	while(is_running_)
	{
		SleepMs(200);
	}

	sScheduler->shutdown();

	LOG_INFO(g_logger, "test_client exit success!");
	g_logger->Save();
	delete g_logger;

	return 0;
}
