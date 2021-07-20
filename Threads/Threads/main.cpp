#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <iostream>
#include <mutex>

#define MAX_THREADS 10


bool msgAvailable = false;
std::condition_variable cond;

long long int counter = 0;
std::mutex mtx;

void increment(int iter)
{
	for (int i = 0; i < iter; ++i)
	{
		std::unique_lock<std::mutex> lock(mtx); // This blocks upon other threads until scope is finished?  
		counter++;
	}
}

void function(int id)
{
	//... code
	std::cout << "Thread "<< id << " is running..." << std::endl;
}

void event()
{
	std::unique_lock<std::mutex> lock(mtx);

	while (msgAvailable == false)
	{
		cond.wait(lock);
	}

	std::cout << "Thread Event Awakened" << std::endl;
}


int main()
{

	std::thread t1(&event);
	std::cout << "Thread Event Sleeping..." << std::endl;
	
	
	{
		std::unique_lock<std::mutex> lock(mtx);
		msgAvailable = true;
		cond.notify_one();
	}

	t1.join();


	/*
	std::thread t1(&increment, 1000000);
	std::thread t2(&increment, 1000000);

	t1.join();
	t2.join();
	*/

	/*std::thread threads[MAX_THREADS];

	for (int i = 0; i < MAX_THREADS; ++i)
	{
		threads[i] = std::thread(&function, i);
	}


	for (int i = 0; i < 10; ++i)
	{
		threads[i].join();
	}
	*/
	
	std::cout << "Counter = "<< counter << std::endl;
	system("pause");
	return 0;
}