#include <iostream>
#include <vector>

//-----------part 1: for use in multiple thread, accessing same data vector--------------
#include <thread>
#include <cmath>
#include <mutex>
//std::vector<int> primes;
//std::mutex mu;

//------------------------------------------
//part 2: for use in future and promise 
//part 3: also for use in future and async
//------------------------------------------
#include <future>

//------------------------------------------
//part 4: for use in Semaphore
//------------------------------------------
#include "semaphore.h"

//------------------------------------------
//part 5: for use in task queue
//------------------------------------------
#include <queue>



bool isPrime(int num) {
    if(num <= 1) return false;
    if(num == 2) return true;
    if(num % 2 == 0) return false;
    for(int i = 3; i <= sqrt(num); i += 2) {
        if(num % i == 0)
            return false;
    }
    return true;
}

void findPrimes(int start, int end, std::vector<int>& primes, std::mutex& mu) {
    for(int i = start; i <= end; ++i) {
        if(isPrime(i)) {
            mu.lock();
            primes.push_back(i);
            mu.unlock();
        }
    }
}

/*using multiple thread to find primes*/   
void multithreadFindPrimes() {
    int num = 200;  // Limit
    int numThreads = 5;  // Number of threads

    // Each thread checks a range of numbers for primality
    std::vector<std::thread> threads;
    std::vector<int> primes;
    std::mutex mu;

    int range = num / numThreads;
    for(int i = 0; i < numThreads; ++i) {
        threads.push_back(std::thread(findPrimes, i * range + 1, (i + 1) * range, std::ref(primes), std::ref(mu)));
    }

    // Wait for all threads to finish
    for(auto& t : threads) {
        t.join();
    }

    // Print the primes
    std::cout << "The result from multi-threads updating same vector which stores primes numbers are: " ;
    for(int prime : primes) {
        std::cout << prime << ' ';
    }
    std::cout << std::endl << std::endl;

}


//----------------------------------------------
int doWork(int limit) {
    int result = 0;
    for (int i = 0; i < limit; ++i) {
        result += i;
    }
    return result;
}

void setPromiseValue(std::promise<int> val) {
    int res = doWork(1000);
    val.set_value(res); // we use promise.set_value / .set_exception to set value and raise exception of the promise

}

void testFuturePromise() {
    
    std::promise<int> pm;//declare a promise
    std::future<int> fu = pm.get_future();//promise get_futre => fu is defined to get future value from the promise

    std::thread t(setPromiseValue,std::move(pm));//create a thread named "t", and set the value of promise in that thread

    int out = fu.get();//in the consumer thread, fu.get() will wait/block until promise's value has been set 
    std::cout <<"The result from custom-controlled promise.set_value(): " << out << std::endl<< std::endl;
    t.join();//wait till thread ends, if not wait the thread ends, a terminate will be called
}
//---------------------------------------------
void testFutureAsync1() {
    //no need to create promise, and no need to create thread 
    //directly use std::async and will return a future 
    //is like using promise.get_future
    std::future<int> fu = std::async(doWork,1000);

    int out = fu.get();
    std::cout <<"The result from std::async given a function: " << out << std::endl<< std::endl;

    //no need join

}
//-----------------------------------------------
//using Async with lambda

void testFutureAsync2() {
    //no need to create promise, and no need to create thread 
    //directly use std::async and will return a future 
    //is like using promise.get_future
    std::future<int> fu = std::async([](int limit){return limit*limit;},1000);

    int out = fu.get();
    std::cout <<"The result from asynchronous lambda: " << out << std::endl<< std::endl;

    //no need join

}

//-----------------------------------------------
//using std::lock_guard
std::mutex mtx;

void printThread(int id) {
    //using std:lock_guard<> will automatically locks the mutex when it is constructed 
    //and unlocks it when it is destructed (which happens when the function returns)
    //void deadlock
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Thread " << id << ' ';
}

void testLockGuard() {
    std::cout << "The result of 10 threads using lock_guard to protect other threads from entering mutex: ";
    std::thread threads[10];
    for (int i = 0; i < 10; ++i)
        threads[i] = std::thread(printThread, i);

    for (auto& th : threads)
        th.join();

    std::cout << std::endl<< std::endl;
}

//-----------------------------------------------
//test self-defined semaphore
Semaphore sem(3);// Initialize a semaphore with a count of 3

void worker(int id) {
    sem.wait();
    std::cout << "Worker " << id << " is working\n";
    sem.notify();
}

void testSemaphore() {
    std::cout << "The result of 10 threads using Semaphore: "; 
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.push_back(std::thread(worker, i));
    }

    for (auto& th : threads) {
        th.join();
    }    
}

//--------------------------- using event driven model (less threads are used)------------------------

// Instead of creating threads for each task, use an event-driven model with a task queue
std::queue<std::function<void()>> tasks;
std::mutex mtx2;
std::condition_variable cv;
bool taskProcessing = true;

void thread_func() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mtx2);
            cv.wait(lock, []{ return !tasks.empty() || !taskProcessing; });//will wait at beginning since tasks is empty
            if(tasks.empty() && !taskProcessing)
                return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
        cv.notify_one(); // Notify the main cv that a task has been processed
    }
}

//event-driven model
void useTaskQueue() {
    std::cout << "The result of event driven model using task queue: "; 
    std::thread worker(thread_func);
    //...
    {
        std::lock_guard<std::mutex> lock(mtx2);
        tasks.push([]() { std::cout << "Task 1\n"; });
        tasks.push([]() { std::cout << "Task 2\n"; });
        cv.notify_one();    //after setup tasks, it wake up the thread to work
    }
    
    // wait until all tasks are processed
    {
        std::unique_lock<std::mutex> lock(mtx2);
        cv.wait(lock, []{ return tasks.empty(); });//will wait while main loop first arrives, since tasks are still in process
        taskProcessing = false;
        cv.notify_one();// notify the thread cv to wake up and check (since taskProcessing is changed)
    }

    worker.join();
    
}

//--------------------------- main test of different codes -------------------
int main() {
    multithreadFindPrimes();
    testFuturePromise();
    testFutureAsync1();
    testFutureAsync2();
    testLockGuard();
    testSemaphore();
    useTaskQueue();


    return 0;
}
