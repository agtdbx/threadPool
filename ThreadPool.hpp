#ifndef THREADPOOL_HPP
# define THREADPOOL_HPP

# include <vector>
# include <thread>
# include <mutex>


typedef struct task_s
{
	void	*param;
	void	(*function)(void *);
}	Task;


typedef enum ThreadStatus_e
{
	THREAD_STARTING,
	THREAD_WAITING,
	THREAD_PROCESSING,
	THREAD_ENDING,
}	ThreadStatus;


typedef struct ThreadParams_s
{
	std::mutex		*mutex;
	ThreadStatus	status;
	Task			*task;

}	ThreadParams;


class ThreadPool
{
public:
	ThreadPool(void);
	ThreadPool(size_t nbThreads);
	ThreadPool(const ThreadPool &obj);
	~ThreadPool();

	bool	threadsRunning(void) const;

	ThreadPool	&operator=(const ThreadPool &obj);

	void	startThreads(size_t nbThreads);
	void	stopThreads(void);
	void	addTask(void (*function)(void *), void *param);
	void	waitAllTasks(void);

private:
	size_t						nbThreads;
	std::vector<std::thread*>	threads;
	std::vector<ThreadParams>	params;
	std::vector<Task>			tasks;
};

#endif
