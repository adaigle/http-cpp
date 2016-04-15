#ifndef RUNNABLE_H
#define RUNNABLE_H

#include <atomic>
#include <exception>
#include <thread>

#include "logger.h"

class class_thread
{
public:
    class_thread() : running(), thread() {

    }

    virtual ~class_thread() {
        try {
            stop();
        } catch(std::exception& e) {
            logger::log(logger::type::server)->error() << "Server error, failed to stop the worker properly.";
            logger::log(logger::type::server)->error() << e.what();
        }
    }

    class_thread(class_thread const&) = delete;
    class_thread& operator =(class_thread const&) = delete;

    void stop() {
        running = false;
        thread.join();
    }
    void start() {
        running = true;
        thread = std::thread(&class_thread::run, this);
    }

protected:
    virtual void run() = 0;

    std::atomic<bool> running;

private:
    std::thread thread;
};

#endif // RUNNABLE_H
