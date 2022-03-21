#include "common.h"
#include "rwlock.h"   

RwLock::RwLock()
{
	pthread_rwlock_init(&lock, NULL);
}
RwLock::~RwLock()
{
	pthread_rwlock_destroy(&lock);
}

void RwLock::ReadLock()
{
	pthread_rwlock_rdlock(&lock);
}

void RwLock::ReadUnLock()
{
	pthread_rwlock_unlock(&lock);
}

void RwLock::WriteLock()
{
	pthread_rwlock_wrlock(&lock);
}

void RwLock::WriteUnLock()
{
	pthread_rwlock_unlock(&lock);
}