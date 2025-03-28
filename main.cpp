#include "ThreadPool.hpp"

#include <execution>
#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <vector>

void	tkt(void *parameter)
{
	if (parameter == NULL)
		return ;

	int *value = (int*)parameter;

	usleep(100000);
}


int	main(void)
{
	int					size = 10, timeTook, j;
	std::vector<int>	vec(size);
	std::clock_t		start;
	ThreadPool			pool((size_t)size);

	for (int i = 0; i < size; i++)
		vec[i] = i;

	// Classic loop
	printf("Classic loop start\n");
	start = std::clock();
	for (int i = 0; i < size; i++)
		tkt(&vec[i]);
	timeTook = ((double)(std::clock() - start) / CLOCKS_PER_SEC) * 1000000;
	printf("took %i us\n\n", timeTook);

	// For each loop
	printf("For each loop start\n");
	start = std::clock();
	std::for_each(vec.begin(), vec.end(), [](int &elem)
	{
		tkt(&elem);
	});
	timeTook = ((double)(std::clock() - start) / CLOCKS_PER_SEC) * 1000000;
	printf("took %i us\n\n", timeTook);

	// For each parallel loop
	printf("For each loop parallel start\n");
	start = std::clock();
	std::for_each(std::execution::par, vec.begin(), vec.end(), [](int &elem)
	{
		tkt(&elem);
	});
	timeTook = ((double)(std::clock() - start) / CLOCKS_PER_SEC) * 1000000;
	printf("took %i us\n\n", timeTook);

	// For each parallel loop
	printf("For each loop parallel unseq start\n");
	start = std::clock();
	std::for_each(std::execution::par_unseq, vec.begin(), vec.end(), [](int &elem)
	{
		tkt(&elem);
	});
	timeTook = ((double)(std::clock() - start) / CLOCKS_PER_SEC) * 1000000;
	printf("took %i us\n\n", timeTook);

	// Thread pool loop
	printf("Thread pool start\n");
	start = std::clock();
	for (int i = 0; i < size; i++)
		pool.addTask(tkt, &vec[i]);
	pool.waitAllTasks();
	timeTook = ((double)(std::clock() - start) / CLOCKS_PER_SEC) * 1000000;
	printf("took %i us\n\n", timeTook);
}
