#pragma once
#include <iostream>
#include <vector>

std::vector<char*> debug_logs{};

namespace Debug
{
    void log_err();

    void log(char* log)
    {
        debug_logs.push_back(log);
        std::cout << log << '\n';
    }

    void get_all_logs(std::vector<char*>& external_logs)
    {
        external_logs = debug_logs;
    }
}
