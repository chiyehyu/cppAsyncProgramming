#include "semaphore.h"

Semaphore::Semaphore(int count) : m_count(count) {
}

void Semaphore::notify() {
    std::unique_lock<std::mutex> lock(m_mtx);
    m_count++;
    m_cv.notify_one();
}

void Semaphore::wait() {
    std::unique_lock<std::mutex> lock(m_mtx);
    while (m_count == 0) {
        m_cv.wait(lock);
    }
    m_count--;
}

/*
To compile and run these files together using g++, you can use the following command:

sh
Copy code
g++ -std=c++14 -pthread semaphore.cpp main.cpp -o main
./main
This will compile both semaphore.cpp and main.cpp, link them together, and produce an executable named main. The -pthread flag is necessary for multithreading support.
*/





