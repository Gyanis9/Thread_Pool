cmake_minimum_required(VERSION 3.28)
project(Thread_Pool)

set(CMAKE_CXX_STANDARD 17)

add_executable(Thread_Pool main.cpp
        src/ThreadPool.cpp
        lib/ThreadPool.h
        lib/Task.h
        src/Thread.cpp
        lib/Thread.h
        lib/Any.hpp
        src/Semaphore.cpp
        lib/Semaphore.h
        src/Task.cpp
)