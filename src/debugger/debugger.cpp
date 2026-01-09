#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include <chrono>

std::vector<std::string> debug_logs{};

struct TimeProfile
{

};

std::unordered_map<std::string, TimeProfile> timers;

auto start_time = std::chrono::high_resolution_clock::now();

namespace Debug
{
    void log_err();

    void log(char* log)
    {
        debug_logs.push_back(log);
        std::cout << log << '\n';
    }
    void log(std::string log)
    {
        debug_logs.push_back(log.c_str());
        std::cout << log << '\n';
    }

    uint32_t logs_size()
    {
        return debug_logs.size();
    }

    std::vector<std::string>& get_all_logs()
    {
        return debug_logs;
    }
    void profile_time_start()
    {
        start_time = std::chrono::high_resolution_clock::now();
    }

    void profile_time_end()
    {
        auto end_time = std::chrono::high_resolution_clock::now();
        double time = std::chrono::duration<double, std::chrono::seconds::period>(end_time - start_time).count();
        time*=10;//Converts to seconds

        std::string time_string = std::to_string(time);

        log(time_string.c_str());
    }
}
