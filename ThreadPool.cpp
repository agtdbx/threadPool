#include "ThreadPool.hpp"

#include <iostream>
#include <unistd.h>

//**** STATIC FUNCTION DECLARATIONS ********************************************

static void	threadRoutine(ThreadParams *params);

//**** INITIALISION ************************************************************
//---- Constructors ------------------------------------------------------------

ThreadPool::ThreadPool(void)
{
	this->nbThreads = 0;
}


ThreadPool::ThreadPool(size_t nbThreads)
{
	this->nbThreads = nbThreads;
	if (this->nbThreads < 1)
		this->nbThreads = 1;
	this->startThreads(nbThreads);
}


ThreadPool::ThreadPool(const ThreadPool &obj)
{
	this->nbThreads = obj.nbThreads;
	this->startThreads(this->nbThreads);
}

//---- Destructor --------------------------------------------------------------

ThreadPool::~ThreadPool()
{
	this->stopThreads();
}

//**** ACCESSORS ***************************************************************
//---- Getters -----------------------------------------------------------------

bool	ThreadPool::threadsRunning(void) const
{
	return (this->nbThreads == 0);
}

//---- Setters -----------------------------------------------------------------

//---- Operators ---------------------------------------------------------------

ThreadPool	&ThreadPool::operator=(const ThreadPool &obj)
{
	if (this == &obj)
		return (*this);

	this->threads.clear();
	this->nbThreads = obj.nbThreads;

	return (*this);
}

//**** PUBLIC METHODS **********************************************************

void	ThreadPool::startThreads(size_t nbThreads)
{
	if (nbThreads < 1)
		return ;

	this->nbThreads = nbThreads;
	this->params.resize(this->nbThreads);
	this->tasks.resize(this->nbThreads);

	for (size_t i = 0; i < this->nbThreads; i++)
	{
		this->tasks[i].function = NULL;
		this->tasks[i].param = NULL;

		this->params[i].mutex = new std::mutex();
		this->params[i].status = THREAD_STARTING;
		this->params[i].task = &this->tasks[i];

		this->threads.push_back(new std::thread(threadRoutine, &this->params[i]));

		ThreadStatus	status = THREAD_STARTING;

		while (status == THREAD_STARTING)
		{
			usleep(10000);
			this->params[i].mutex->lock();
			status = this->params[i].status;
			this->params[i].mutex->unlock();
		}
	}
}


void	ThreadPool::stopThreads(void)
{
	if (this->nbThreads == 0)
		return ;

	this->waitAllTasks();

	for (size_t i = 0; i < this->threads.size(); i++)
	{
		this->params[i].mutex->lock();
		this->params[i].status = THREAD_ENDING;
		this->params[i].mutex->unlock();
		if (this->threads[i] != NULL)
		{
			this->threads[i]->join();
			delete this->threads[i];
		}
	}

	for (size_t i = 0; i < this->params.size(); i++)
		if (this->params[i].mutex != NULL)
			delete this->params[i].mutex;

	this->threads.clear();
	this->params.clear();
	this->tasks.clear();
	this->nbThreads = 0;
}


void	ThreadPool::addTask(void (*function)(void *), void *param)
{
	if (this->nbThreads == 0)
		return ;

	while (1)
	{
		for (size_t i = 0; i < this->nbThreads; i++)
		{
			this->params[i].mutex->lock();

			if (this->params[i].status == THREAD_PROCESSING)
			{
				this->params[i].mutex->unlock();
				continue;
			}

			this->tasks[i].function = function;
			this->tasks[i].param = param;
			this->params[i].status = THREAD_PROCESSING;

			this->params[i].mutex->unlock();

			return ;
		}
		usleep(10000);
	}
}


void	ThreadPool::waitAllTasks(void)
{
	if (this->nbThreads == 0)
		return ;

	bool	wait = true;

	while (wait)
	{
		wait = false;
		for (size_t i = 0; i < this->nbThreads; i++)
		{
			this->params[i].mutex->lock();

			if (this->params[i].status == THREAD_PROCESSING)
				wait = true;

			this->params[i].mutex->unlock();

			if (wait)
				break;
		}
		usleep(10000);
	}
}

//**** PRIVATE METHODS *********************************************************

//**** STATIC FUNCTION *********************************************************

static void	threadRoutine(ThreadParams *params)
{
	if (params == NULL)
	{
		std::cout << "Thread params error" << std::endl;
		return ;
	}

	if (params->mutex == NULL)
	{
		std::cout << "Thread mutex error" << std::endl;
		return ;
	}

	std::mutex		*mutex = params->mutex;
	ThreadStatus	status;

	mutex->lock();
	params->status = THREAD_WAITING;
	mutex->unlock();
	status = THREAD_WAITING;

	while (true)
	{
		mutex->lock();
		status = params->status;
		mutex->unlock();

		if (status == THREAD_ENDING)
			break;

		if (status == THREAD_PROCESSING)
		{
			if (params->task != NULL &&
				params->task->function != NULL &&
				params->task->param != NULL)
			{
				(*params->task->function)(params->task->param);
			}

			mutex->lock();
			params->status = THREAD_WAITING;
			mutex->unlock();
		}
		else
			usleep(10000);
	}
}
