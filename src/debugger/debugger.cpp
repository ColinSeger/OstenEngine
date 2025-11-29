#include "debugger.h"

std::vector<char*> logs{};

void Debug::log(char* log)
{
    logs.push_back(log);
}


void Debug::get_all_logs(std::vector<char*>& external_logs)
{
    external_logs = logs;
}