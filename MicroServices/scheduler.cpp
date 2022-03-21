#include "scheduler.h"
#include "scheduler_impl.h"

initialiseSingleton(Scheduler);

Scheduler::Scheduler()
{
	imp_ = new Scheduler_impl();
}

Scheduler::~Scheduler()
{
	if (imp_)
	{
		delete imp_;
		imp_ = NULL;
	}
}

void Scheduler::set_log(CLog* log)
{
	log_ = log;
}

bool Scheduler::startup()
{
	return imp_->startup();
}

bool Scheduler::shutdown()
{
	return imp_->shutdown();
}

CLog* Scheduler::get_logger()
{
	return log_;
}

Service* Scheduler::new_service()
{
	return imp_->new_service();
}

void Scheduler::delete_service( uint32 service_id )
{
	imp_->delete_service(service_id);
}

void Scheduler::send( uint32 service_id, const char* data, uint32 data_len )
{
	imp_->send(service_id, data, data_len);
}