#include "debugger.h"

std::vector<char*> logs{};

void debug::log(char* log)
{
    logs.push_back(log);
}


void debug::get_all_logs(std::vector<char*>& external_logs)
{
    external_logs = logs;
}