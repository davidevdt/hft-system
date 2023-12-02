#pragma once 

#include <cstring> 
#include <iostream> 

// tells the compiler that a condition is expected to be true or false 
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)

inline auto ASSERT(bool cond, const std::string& msg) noexcept {
    if (UNLIKELY(!cond)) {
        std::cerr << "ASSERT: " << msg << std::endl; 
        exit(EXIT_FAILURE); 
    }
}

inline auto FATAL(const std::string& msg) noexcept {
    std::cerr << "FATAL: " << msg << std::endl; 
    exit(EXIT_FAILURE); 
}