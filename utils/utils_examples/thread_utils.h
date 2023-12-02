#pragma once 
#include <iostream> 
#include <atomic> 
#include <thread> 
#include <unistd.h>
#include <sys/syscall.h>

namespace Common {

    // Sets affinity of calling thread by pinning it to the input core_id 
    inline auto setThreadCore(int core_id) noexcept {
        cpu_set_t cpuset; 
        CPU_ZERO(&cpuset);  // reset the masks 
        CPU_SET(core_id, &cpuset); // sets the bit corresponding to the core_id in the cpu_set
                                // e.g. (0, 0, 1, 0) pins to CPU core nr. 3 
        // The next function sets the CPU affinity of the cpuset to the calling thread; 
        return (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) == 0); 
    }

    // Creates a thread, sets an affinity for it, and passes the function to be run 
    // to that thread, along with its arguments 
    // T: function; A: T's arguments 
    template <typename T, typename... A> 
    inline auto createAndStartThread(int core_id, const std::string& name, T&& func, A&&... args) noexcept {

        std::atomic<bool> running(false), failed(false); 
        auto thread_body = [&] {
            if (core_id >= 0 && !setThreadCore(core_id)) {
                std::cerr << "Failed to set core affinity for " << name << " " << pthread_self() << " to " << core_id << std::endl; 
            failed = true; 
            return; 
            }
            std::cout << "Set core affinity for " << name << " " << pthread_self() << " to " << core_id << std::endl; 
            running = true; 
            std::forward<T>(func)((std::forward<A>(args))...); 
        }; 
        auto t = new std::thread(thread_body); 
        while (!running && !failed) {
            using namespace std::literals::chrono_literals; 
            std::this_thread::sleep_for(1s); 
        }
        if (failed) {
            t->join(); 
            delete t; 
            t = nullptr; 
        }
        return t; 
    }
}