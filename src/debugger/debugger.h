#pragma once
#include <vector>

namespace Debug
{
    void log(char* log);

    void log_err();

    void get_all_logs(std::vector<char*>& logs);
}