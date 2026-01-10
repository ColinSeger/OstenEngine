#pragma once
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include "../platform.h"

static std::vector<std::string> debug_logs{};

namespace Debug
{
    constexpr uint8_t time_capacity = 15;
    struct TimeLog
    {
        double timers[time_capacity];
        uint8_t index = 0;
    };

    static TimeLog time_logger{};

    void log_err();

    static inline void log(char* log)
    {
        debug_logs.push_back(log);
        std::cout << log << '\n';
    }
    static inline void log(std::string log)
    {
        debug_logs.push_back(log.c_str());
        std::cout << log << '\n';
    }

    static inline uint32_t logs_size()
    {
        return debug_logs.size();
    }

    static inline std::vector<std::string>& get_all_logs()
    {
        return debug_logs;
    }
    static inline void profile_time_start()
    {
        time_logger.timers[time_logger.index] = get_time_since_start();
        if(time_logger.index >= time_capacity){
            return;
        }
        time_logger.index++;
    }

    static inline void profile_time_end()
    {
        double end_time = get_time_since_start();
        time_logger.index--;
        double time = end_time - time_logger.timers[time_logger.index];
        time*=10;//Converts to seconds

        std::string time_string = std::to_string(time);

        log(time_string.c_str());
    }
}
