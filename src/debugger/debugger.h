#pragma once
#include <stdint.h>
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
        Timer timers[time_capacity];
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
        time_logger.timers[time_logger.index] = platform_get_time_handle();
        if(time_logger.index >= time_capacity){
            return;
        }
        time_logger.index++;
    }

    static inline void profile_time_end(){
        time_logger.index--;

        double time = platform_calc_elapsed_time_seconds(time_logger.timers[time_logger.index]);

        std::string time_string = std::to_string(time);

        log(time_string.c_str());
    }
}
