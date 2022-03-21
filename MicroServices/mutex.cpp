#include "common.h"
#include "mutex.h"   

Mutex::Mutex()
{
	pthread_mutex_init(&mutex, NULL);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&mutex);
}

void Mutex::Lock()
{
	pthread_mutex_lock(&mutex);
}

void Mutex::UnLock()
{
	pthread_mutex_unlock(&mutex);
}
